#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "cpu.h"
#include "schedulers.h"

struct node * head = NULL;
struct node * rear;
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

void clearMemory () {
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
	if (head == NULL) {
		head = (struct node *)malloc (sizeof (struct node));
		head->next = NULL;
		head->task = NULL;
		rear = head;
	}
	rear->next = (struct node *) malloc (sizeof (struct node));
	rear = rear->next;
	rear->task = createTask (name, priority, burst);
	rear->next = NULL;
}

// invoke the scheduler
void schedule() {
	struct node * ptr = head->next;
	while (ptr) {
		run (ptr->task, ptr->task->burst);
		ptr = ptr->next;
	}
	clearMemory ();
}
