#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_SIZE 10
#define true 1
#define false 0

struct process
{
	int base;
	int limit;
	char pid[MAX_CMD_SIZE];
	struct process* next;
};

/* use list to store memory*/
struct process* head,*tail;

// allocate types
struct process * new_process(int base, int limit, char* pid, struct process * next);
struct process * first_fit(int memory);
struct process * best_fit(int memory);
struct process * worst_fit(int memory);

//operations
int request_memory(char * pid, int memory, char mode);
int release_memory(char * pid);
void show_status(void);
void compaction(void);

//clear memory of program
void clear_memory(void);

int main(int argc, char * argv[])
{
	int MAX;
	char cmd[MAX_CMD_SIZE];
	char pid[MAX_CMD_SIZE];
	int memory;
	char mode;

	//initialize

	if(argc < 2)
	{
		printf("[error] expecting 1 argument, get %d\n",argc-1);
		return 1;
	}

	MAX = atoi(argv[1]);
	printf("initialization option: MAX=%d\n",MAX);
	tail = new_process(MAX,0,"TAIL",NULL);
	head = new_process(0,0,"HEAD",tail);

	//waiting for input
	
	while(true)
	{
		printf("allocator>");

		scanf("%s",cmd);

		if(strcmp(cmd,"X")==0)	break;
		else if (strcmp(cmd,"RQ")==0)	//request memory
		{
			scanf("%s",pid);
			scanf("%d",&memory);

			while((mode = getchar()) ==' ');

			if(request_memory(pid,memory,mode))
				printf("allocation granted.\n");

			else
				printf("allocation rejected.\n");
		}
		
		else if(strcmp(cmd,"RL")==0)	//memory release
		{
			scanf("%s",pid);
			if(release_memory(pid))	
				printf("release successfully.\n");
			else
				printf("release failed.\n");

		}

		else if(strcmp(cmd,"STAT")==0)	//show status
		{
			show_status();
		}
		else if(strcmp(cmd,"C")==0)	//compaction
		{
			compaction();
			printf("compaction done.\n");
		}
		else if (strcmp(cmd,"exit")==0) break;
		else
		{
			printf("unknown commamd:%s\n",cmd);
		}
		while(getchar()!='\n');
	}

	printf("exiting allocator\n");
	clear_memory();

	return 0;
}

struct process * new_process( int base , int limit , char * pid , struct process * next)
{
	struct process *ptr = (struct process*) malloc(sizeof (struct process));
	ptr->base=base;
	ptr->limit=limit;
	strcpy(ptr->pid,pid);
	ptr->next=next;
	return ptr;
}

struct process * first_fit(int memory)
{
	struct process * ptr = head;
	int vol; //size of hole
	while ( ptr!=tail)
	{
		vol = (ptr->next->base-(ptr->base+ptr->limit));
		if(vol<memory)
			ptr=ptr->next;
		else
			return ptr;
	}
	return NULL;
}

struct process * best_fit(int memory)
{
	struct process * ptr=head;
	struct process * best_ptr=NULL;

	int vol,best_vol;
	best_vol=tail->base+1;

	while(ptr!=tail)
	{
		vol=(ptr->next->base-(ptr->base+ptr->limit));

		if(memory<=vol && vol<best_vol)
		{
			best_vol=vol;
			best_ptr=ptr;
		}
		ptr=ptr->next;

	}

	return best_ptr;
}

struct process * worst_fit(int memory)
{
	struct process * ptr=head;
	struct process * worst_ptr=NULL;

	int vol,worst_vol;
	worst_vol=-1;

	while(ptr!=tail)
	{
		vol=(ptr->next->base-(ptr->base+ptr->limit));

		if(memory<=vol && vol>worst_vol)
		{
			worst_vol=vol;
			worst_ptr=ptr;
		}
		ptr=ptr->next;

	}

	return worst_ptr;
}

int request_memory ( char * pid, int memory, char mode)
{
	struct process * ptr;

	switch(mode)
	{
		case 'F':
			ptr=first_fit(memory);
			break;
		case 'B':
			ptr=best_fit(memory);
			break;
		case 'W':
			ptr=worst_fit(memory);
			break;
		default:
			printf("invaid allocation mode %c \n",mode);
			return false;
	}

	if(ptr==NULL)
	{
		printf("No available memory hole.\n");
		return false;
	}

	ptr->next=new_process ( ptr->base+ptr->limit, memory, pid, ptr->next);

	return true;
}

int release_memory (char * pid)
{
	struct process * ptr = head->next;
	struct process * prev=head;

	int success=false;

	while(ptr!=tail)
	{
		if(strcmp(pid,ptr->pid)==0)
		{
			prev->next=ptr->next;
			free(ptr);
			ptr=prev->next;
			success=true;
		}
		else
		{
			prev = ptr;
			ptr = ptr->next;
		}
	}

	return success;
}

void show_status(void)
{
	struct process * ptr = head->next;
	int start=0;
	while( ptr!=NULL)
	{
		if( ptr->base!=start)	//unused memory?
		{
			printf("Address [%d:%d] Unused\n", start, ptr->base -1);
		}
		
		if( ptr->limit !=0) //this is not tail process
		{
			start = ptr->base + ptr->limit -1;
			printf("Address [%d:%d] Process %s\n", ptr->base, ptr->base+ptr->limit-1, ptr->pid);
			++start;

		}

		ptr=ptr->next;
	}
		
}

void compaction(void)
{
	struct process * ptr = head;

	while(ptr->next!=tail)
	{
		ptr->next->base=ptr->base+ptr->limit;
		ptr=ptr->next;
	}
}

void clear_memory(void)
{
	struct process * ptr = head;
	struct process * tmp;
	
	while(ptr!=tail)
	{
		tmp=ptr;
		ptr=ptr->next;
		free(tmp);
	}

}

		
	
		
