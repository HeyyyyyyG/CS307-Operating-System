#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 4
#define MAX_COMMAND 10

/* the available amount of each resource */
int available [NUMBER_OF_RESOURCES];

/* the maximum demand of each resource */
int maximum [NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

/* the amount currently allocated to each customer */
int allocation [NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

/* the remaining need of each customer */
int need [NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int request_resources (int customer_num, int * request);
void release_resources (int customer_num, int * release);

bool safety (void);
bool allLessThanEqual (int * arr1, int * arr2, int n);
bool anyLessThan (int * arr1, int * arr2, int n);
void outputData (void);
void showMatrix (int (* mat)[NUMBER_OF_RESOURCES], int m, int n);

int main (int argc, char * argv[]) {
	int i, j;
	FILE * fp = NULL;
	char command [MAX_COMMAND];
	int input [NUMBER_OF_RESOURCES+5];

	/* 1. initialize data structures */
	if (argc <= NUMBER_OF_RESOURCES) {
		printf ("[error] not enough arguments, expected %d, get %d\n", 
				NUMBER_OF_RESOURCES + 1, argc);
		return 1;
	}

	for (i = 0; i < NUMBER_OF_RESOURCES; ++ i) {
		available[i] = atoi (argv[i+1]);
	}

	fp = fopen ("test.txt", "r");
	if (fp == NULL) {
		printf ("[error] open test.txt failed\n");
		return 1;
	}
	for (i = 0; i < NUMBER_OF_CUSTOMERS; ++ i) {
		fscanf (fp, "%d", &maximum[i][0]);
		for (j = 1; j < NUMBER_OF_RESOURCES; ++ j) {
			fgetc(fp);
			fscanf (fp, "%d", &maximum[i][j]);
		}
	}
	fclose (fp);

	/* show maximum read
	for (i = 0; i < NUMBER_OF_CUSTOMERS; ++ i) {
		for (j = 0; j < NUMBER_OF_RESOURCES; ++ j) {
			printf ("%d ", maximum[i][j]);
		}
		printf ("\n");
	}
	*/

	for (i = 0; i < NUMBER_OF_CUSTOMERS; ++ i) {
		for (j = 0; j < NUMBER_OF_RESOURCES; ++ j) {
			allocation[i][j] = 0;
			need[i][j] = maximum[i][j];
		}
	}

	/* 2. deal with user input */
	while (true) {
		printf ("cmd> ");
		scanf ("%s", command);
		if (strcmp (command, "*") == 0) {
			outputData();
		} else if (strcmp (command, "exit") == 0) {
			break;
		} else if (strcmp (command, "RQ") == 0) {
			scanf ("%d", &j); // the index of customer
			for (i = 0; i < NUMBER_OF_RESOURCES; ++ i)
				scanf ("%d", &input[i]);
			if (!request_resources (j, input)) {
				printf ("[info] request granted\n");
			}
		} else if (strcmp (command, "RL") == 0) {
			scanf ("%d", &j);
			for (i = 0; i < NUMBER_OF_RESOURCES; ++ i)
				scanf ("%d", &input[i]);
			release_resources (j, input);
		} else {
			printf ("[warning] unknown command.\n");
			continue;
		}
	}

	return 0;
}

int request_resources (int customer_num, int * request) {
	/* return 0 if successful, -1 if unsuccessful */ 
	int i;
	
	if (customer_num < 0 || customer_num >= NUMBER_OF_CUSTOMERS) {
		printf ("[warning] invalid customer index, not granted.\n");
		return -1;
	}

	if (anyLessThan (need[customer_num], request, NUMBER_OF_RESOURCES)) {
		printf ("[warning] request greater than max claimed, not granted.\n");
		return -1;
	}

	if (anyLessThan (available, request, NUMBER_OF_RESOURCES)) {
		printf ("[warning] request exceed available, not granted.\n");
		return -1;
	}

	/* assume granted */
	for (i = 0; i < NUMBER_OF_RESOURCES; ++ i) {
		available[i] -= request[i];
		need[customer_num][i] -= request[i];
		allocation[customer_num][i] += request[i];
	}
	if (safety()) {
	/* if system is in safe state, granted */
		return 0;
	} else {
		/* restore data structures from assumption */
		for (i = 0; i < NUMBER_OF_RESOURCES; ++ i) {
			available[i] += request[i];
			need[customer_num][i] += request[i];
			allocation[customer_num][i] -= request[i];
		}
		printf ("[warning] request causes unsafe state, not granted.\n");
		return -1;
	}
}

void release_resources (int customer_num, int * release) {
	int i;

	if (anyLessThan (allocation[customer_num], release, NUMBER_OF_RESOURCES)) {
		printf ("[warning] resources released exceed allocation.\n");
		for (i = 0; i < NUMBER_OF_RESOURCES; ++ i) {
			if (allocation[customer_num][i] < release[i]) {
				release[i] = allocation[customer_num][i];
			}
		}
		printf ("[warning] release is changed to:");
		for (i = 0; i < NUMBER_OF_RESOURCES; ++ i)
			printf ("%d ", release[i]);
		printf ("\n");
	}

	for (i = 0; i < NUMBER_OF_RESOURCES; ++ i) {
		allocation[customer_num][i] -= release[i];
		need[customer_num][i] += release[i];
		available[i] += release[i];
	}
}

bool safety (void) {
	/* test if system state is safe */
	int work[NUMBER_OF_RESOURCES];
	bool finish[NUMBER_OF_CUSTOMERS];
	int i, j;
	bool quit_flag;
	

	/* init work <- available, finish <- false */
	for (i = 0; i < NUMBER_OF_RESOURCES; ++ i)
		work[i] = available[i];
	for (i = 0; i < NUMBER_OF_CUSTOMERS; ++ i)
		finish[i] = false;

	

	while (true) {
		quit_flag = true;
		for (i = 0; i < NUMBER_OF_CUSTOMERS; ++ i) {
			if ((!finish[i]) && allLessThanEqual (need[i], work, NUMBER_OF_RESOURCES)) {  
				
				
				quit_flag = false;
				for (j = 0; j < NUMBER_OF_RESOURCES; ++ j)
					work[j] += allocation[i][j];
				finish[i] = true;	
				
				
			}
		}
		if (quit_flag)
			break;
	}

	for (i = 0; i < NUMBER_OF_CUSTOMERS; ++ i) {
		if (!finish[i]) {
			return false; /* unsafe */
		}
	}
	return true; /* safe */
}

bool allLessThanEqual (int * arr1, int * arr2, int n) {
	int i;
	for (i = 0; i < n; ++ i) {
		if (arr2[i] < arr1[i])
			return false;
	}
	return true;
}

bool anyLessThan (int * arr1, int * arr2, int n) {
	int i;
	for (i = 0; i < n; ++ i) {
		if (arr1[i] < arr2[i])
			return true;
	}
	return false;
}

void outputData (void) {
	int i;
	
	printf ("Available\n");
	for (i = 0; i < NUMBER_OF_RESOURCES; ++ i)
		printf ("%d ", available[i]);
	printf ("\n");

	printf ("Maximum\n");
	showMatrix (maximum, NUMBER_OF_CUSTOMERS, NUMBER_OF_RESOURCES);

	printf ("Allocation\n");
	showMatrix (allocation, NUMBER_OF_CUSTOMERS, NUMBER_OF_RESOURCES);

	printf ("Need\n");
	showMatrix (need, NUMBER_OF_CUSTOMERS, NUMBER_OF_RESOURCES);
}

void showMatrix (int (* mat)[NUMBER_OF_RESOURCES], int m, int n) {
	int i, j;
	for (i = 0; i < m; ++ i) {
		for (j = 0; j < n; ++ j) 
			printf ("%d ", mat[i][j]);
		printf ("\n");
	}
}
