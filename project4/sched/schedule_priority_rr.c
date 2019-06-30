#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "cpu.h"
#include "schedulers.h"

#define TIME 10

struct node ** head_l = NULL;
struct node ** rear_l;
int tid = 0;

Task * createTask (char * name, int priority, int burst) {
	// create a new task and assign a tid
	Task * new_task = (Task*) malloc (sizeof (Task));
	new_task->name = (char*) malloc (sizeof(char)*(strlen(name)+1));
	strcpy (new_task->name, name);
	new_task->priority = priority;
	new_task->burst = burst;
	new_task->tid = 0; // do not assign right now
	//new_task->tid = __sync_fetch_and_add (&tid, 1);
	return new_task;
}

void freeNode (struct node * n) {
	free (n->task->name);
	free (n->task);
	free (n);
}

void clearMemory (int pri) {
	struct node * head = head_l[pri];
	struct node * rear = rear_l[pri];
	rear = head;
	head = head->next;
	free (rear);
	while (head) {
		rear = head;
		head = head->next;
		free (rear->task->name);
		free (rear->task);
		free (rear);
	}
}

// add a task to the list 
void add(char *name, int priority, int burst) {
	int i;
	if (head_l == NULL) {
		head_l = (struct node **) malloc (sizeof (struct node *)*(MAX_PRIORITY+1));
		rear_l = (struct node **) malloc (sizeof (struct node *)*(MAX_PRIORITY+1));
		for (i = 0; i <= MAX_PRIORITY; ++ i) {
			head_l[i] = rear_l[i] = NULL;
		}
	}
	if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
		printf ("[error] Invalid task priority\n");
		return;
	}
	if (head_l[priority] == NULL) {
		head_l[priority] = (struct node *) malloc (sizeof (struct node));
		head_l[priority]->task = NULL;
		head_l[priority]->next = NULL;
		rear_l[priority] = head_l[priority];
	}
	rear_l[priority]->next = (struct node *) malloc (sizeof (struct node));
	rear_l[priority] = rear_l[priority]->next;
	rear_l[priority]->task = createTask (name, priority, burst);
	rear_l[priority]->next = NULL;
}

// invoke the scheduler
void schedulePriority(int priority) {
	int slice;
	struct node * tmp_ptr;
	struct node * head = head_l[priority];
	struct node * rear = rear_l[priority];
	while (head->next) {
		slice = (TIME < head->next->task->burst)?TIME:head->next->task->burst;
		run (head->next->task, slice);
		if (head->next->task->burst == slice) {
		// task complete, remove from list
			tmp_ptr = head->next;
			head->next = head->next->next;
			freeNode (tmp_ptr);
		} else {
			head->next->task->burst -= slice;
			rear->next = head->next;
			rear = rear->next;
			head->next = head->next->next;
			rear->next = NULL;
		}
	}
	clearMemory (priority);
}

void schedule () {
	int i;
	for (i = MAX_PRIORITY; i >= MIN_PRIORITY; -- i) {
		if (head_l[i]) 
			schedulePriority (i);
	}
	free (head_l);
	free (rear_l);
}

