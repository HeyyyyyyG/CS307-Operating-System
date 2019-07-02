#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PAGE_TABLE_ENTRIES_NUM 256
#define PAGE_SIZE 256
#define TLB_ENTRIES_NUM 16
#define FRAME_SIZE 256
#define NUM_FRAMES 256
#define MEMORY_SIZE FRAME_SIZE*NUM_FRAMES
//#define LEN_FILENAME 32

/* memory data structures for paging */
struct ffl_entry {
        int frame_number;
        struct ffl_entry * next;
};

typedef struct ffl_entry free_frame_list_entry;

char * memory;
free_frame_list_entry * free_frame_list;

/* page data structures */
struct page_table_stack_node {
	int page_number;
	struct page_table_stack_node * prev;
	struct page_table_stack_node * next;
};

typedef struct page_table_stack_node pgs_node;

pgs_node * pgs_top;
pgs_node * pgs_bottom;

typedef struct {
	int frame_number;
	bool valid;
	pgs_node * node;
} page_table_entry;

typedef struct {
	int page_number;
	int frame_number;
} TLB_entry;

page_table_entry * page_table;
TLB_entry * TLB;
int free_TLB_index; // indicates current length of TLB, also a free entry in TLB
int * TLB_cnts; // counter implementation of LRU algorithm for replacement

/* statistics */
int num_references;
int num_page_faults;
int num_TLB_hits;
int num_correct_reads;

/* extract page number and offset from an address */
void extract_address (int address, int * page_number, int * offset);

/* translate from logical address to physical address */
int translate (int logical_address, FILE * f_backing_store, int time_cnt);

/* read a byte from memory using physical address */
char read_from_memory (int physical_address);

/* initialize page table and TLB */
void page_table_initialize (void);

/* clear page table and TLB */
void page_table_clear (void);

/* initialize memory and free frame list */
void memory_initialize (void);

/* clear physical memory and free frame list */
void memory_clear (void);

/* get a free frame in physical memory*/
int get_free_frame (void);

int main (int argc, char * argv[]) {
	int logical_address, physical_address;
	FILE * f_addr, * f_backing_store;
	FILE * f_correct;
	int i;
	int c_logical_address, c_physical_address, c_value;
	char content;

	/* check number of arguments */
	if (argc < 2) {
		printf ("[error] expect 2 arguments, get %d\n", argc);
		return 1;
	}

	/* 1. virtual memory manager init */
	page_table_initialize ();
	memory_initialize ();

	f_backing_store = fopen ("BACKING_STORE.bin", "rb");
	f_correct = fopen ("correct.txt", "r"); // open correct outputs to check answers

	if (f_backing_store == NULL) {
		printf ("[error] fail to open backing store\n");
		return 1;
	}
	if (f_correct == NULL) {
		printf ("[error] fail to open correct outputs\n");
		return 1;
	}

	// statistics init
	num_references = 0;
	num_page_faults = 0;
	num_TLB_hits = 0;
	num_correct_reads = 0;

	/* 2. read input file */
	f_addr = fopen (argv[1], "r");
	if (f_addr == NULL) {
		printf ("[error] fail to open file %s\n", argv[1]);
		return 1;
	}

	i = 0;
	while (~fscanf (f_addr, "%d", &logical_address)) {
		physical_address = translate (logical_address, f_backing_store, i);
		content = read_from_memory (physical_address);
		printf ("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, content);
		++ i;

		// check read result
		fscanf (f_correct, "Virtual address: %d Physical address: %d Value: %d\n", &c_logical_address, &c_physical_address, &c_value);
		if (c_logical_address != logical_address) {
			printf ("[warning] inconsistent correct answer, i=%d, logical_address=%d, correct_logical_address=%d\n", i, logical_address, c_logical_address);
		}
		if (c_value == (int) content)
			++ num_correct_reads;
	}


	/* 3. report statistics */
	printf ("Statistics are shown as following:\n");
	printf ("Page fault rate: %f\n", (double) num_page_faults / num_references);
	printf ("TLB hit rate: %f\n", (double) num_TLB_hits / num_references);
	//printf ("Correct rate: %f\n", (double) num_correct_reads / num_references);

	/* 4. shutdown manager */
	fclose (f_addr);
	fclose (f_backing_store);
	fclose (f_correct);
	page_table_clear ();
	memory_clear ();

	return 0;
}

void extract_address (int address, int * page_number, int * offset) {
	int mask = 0xff;
	* offset = address & mask;
	* page_number = (address >> 8) & mask;
}

int translate (int logical_address, FILE * f_backing_store, int time_cnt) {
	int page_num, offset;
	int frame_num;
	int physical_address;
	int i;
	int TLB_index; // indicates the TLB entry corresponding to page number
	int tmp1, tmp2;
	pgs_node * pgs_ptr;

	++ num_references;
	/* 1. extract page number from logical address */
	extract_address (logical_address, &page_num, &offset);

	/* 2. look up frame number in TLB */
	for (i = 0; i < free_TLB_index; ++ i) {
		if (page_num == TLB[i].page_number) {
			/* TLB hit */
			++ num_TLB_hits;
			TLB_index = i;
			//frame_num = TLB[i].frame_number;
			break;
		}
	}
	if (i == free_TLB_index) {
		/* TLB miss */

	/* 3. if TLB miss, look up frame number in page table */
		if (! page_table[page_num].valid) {
			/* page not in memory, handle page fault */
			++ num_page_faults;

			frame_num = get_free_frame();

			fseek (f_backing_store, page_num * PAGE_SIZE, SEEK_SET);
			fread (memory + frame_num * FRAME_SIZE, sizeof(char), FRAME_SIZE, f_backing_store);
			page_table[page_num].valid = true;
			page_table[page_num].frame_number = frame_num;

			// pg stack update after page fault
			page_table[page_num].node->next = pgs_top->next;
			pgs_top->next->prev = page_table[page_num].node;
			pgs_top->next = page_table[page_num].node;
			page_table[page_num].node->prev = pgs_top;
		}

		// look up frame number from page table
		frame_num = page_table [page_num].frame_number;

		/* replace TLB entry */
		if (free_TLB_index < TLB_ENTRIES_NUM) {
			/* TLB not full */
			TLB_index = free_TLB_index;
			++ free_TLB_index;
		} else {
			/* TLB full, use LRU to replace one entry */
			tmp1 = TLB_cnts[0]; // stores the minimum cnt
			tmp2 = 0; // indicates the entry with the minimum cnt
			for (i = 1; i < TLB_ENTRIES_NUM; ++ i) {
				if (TLB_cnts[i] < tmp1) {
					tmp1 = TLB_cnts[i];
					tmp2 = i;
				}
			}
			TLB_index = tmp2;
		}

		// new entry should be stored in TLB[TLB_index]
		TLB[TLB_index].page_number = page_num;
		TLB[TLB_index].frame_number = frame_num;
	}

	// update page table stack, change six pointers
        pgs_ptr = page_table[page_num].node->next;
        page_table[page_num].node->next = pgs_top->next;
        pgs_top->next->prev = page_table[page_num].node;
        pgs_top->next = page_table[page_num].node;
        pgs_ptr->prev = page_table[page_num].node->prev;
        page_table[page_num].node->prev->next = pgs_ptr;
        page_table[page_num].node->prev = pgs_top;

	frame_num = TLB[TLB_index].frame_number;
	TLB_cnts[TLB_index] = time_cnt; // set the counter

	physical_address = (frame_num << 8) | offset;

	/* reset file pointer, not necessary in this context */
	fseek (f_backing_store, 0, SEEK_SET);

	return physical_address;
}

/* read a byte from memory using physical address */
char read_from_memory (int physical_address) {
	return memory[physical_address];
}

/* initialize page table and TLB */
void page_table_initialize (void) {
	int i;

	page_table = (page_table_entry *) malloc (sizeof (page_table_entry) * PAGE_TABLE_ENTRIES_NUM);

	TLB = (TLB_entry *) malloc (sizeof (TLB_entry) * TLB_ENTRIES_NUM);
	free_TLB_index = 0; // indicates a free TLB entry
        TLB_cnts = (int *) malloc (sizeof (int) * TLB_ENTRIES_NUM);

	// page table stack init; stack implementation for LRU
	pgs_top = (pgs_node *) malloc (sizeof(pgs_node));
	pgs_bottom = (pgs_node *) malloc (sizeof (pgs_node));
	pgs_top->prev=NULL;
	pgs_bottom->next=NULL;
	pgs_top->next = pgs_bottom;
	pgs_bottom->prev = pgs_top;

	for (i = 0; i < PAGE_TABLE_ENTRIES_NUM; ++ i) {
                page_table[i].valid = false;
		// stack init
		page_table[i].node = (pgs_node *)malloc (sizeof (pgs_node));
		page_table[i].node->page_number = i;
	}
}

/* clear page table and TLB */
void page_table_clear (void) {
	int i;
	for (i = 0; i < PAGE_TABLE_ENTRIES_NUM; ++ i) {
		free (page_table[i].node);
	}
	free (pgs_top);
	free (pgs_bottom);
	free (page_table);
        free (TLB);
        free (TLB_cnts);
}

/* initialize memory and free frame list */
void memory_initialize (void) {
	int i, num_frame;
	free_frame_list_entry * tmp;

	memory = (char *) malloc (MEMORY_SIZE * sizeof (char));

	free_frame_list = NULL;
	num_frame = MEMORY_SIZE / FRAME_SIZE;
	for (i = 0; i < num_frame; ++ i) {
		tmp = free_frame_list;
		free_frame_list = (free_frame_list_entry *) malloc (sizeof (free_frame_list_entry));
		free_frame_list->frame_number = num_frame - i - 1;
		free_frame_list->next = tmp;
	}
}

/*clear physical memory and free frame list */
void memory_clear (void) {
	free_frame_list_entry * tmp;

	free (memory);
	while (free_frame_list) {
		tmp = free_frame_list;
		free_frame_list = tmp->next;
		free (tmp);
	}
}

/* get a free frame in physical memory*/
int get_free_frame (void) {
	free_frame_list_entry * tmp;
	int frame_num;
	pgs_node * pgs_ptr;

	if (free_frame_list == NULL) {
		/* no free frame, need page replacement */
		// replace the bottom node of pg stack
		pgs_ptr = pgs_bottom->prev;
		if (pgs_ptr == pgs_top) {
			printf ("[error] page table empty when trying to replace a page.\n");
			return 0;
		}

		// now delete this page from pg stack
		page_table[pgs_ptr->page_number].valid = false;
		pgs_bottom->prev = pgs_ptr->prev;
		pgs_ptr->prev->next = pgs_bottom;

		free_frame_list = (free_frame_list_entry *) malloc (sizeof (free_frame_list_entry));
		free_frame_list->frame_number = page_table[pgs_ptr->page_number].frame_number;
		free_frame_list->next = NULL;
		
	}

	tmp = free_frame_list;
	free_frame_list = tmp->next;
	frame_num = tmp->frame_number;

	free (tmp);

	return frame_num;
}
