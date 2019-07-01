/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3
#define REAL_QUEUE_SIZE (QUEUE_SIZE+1)

#define TRUE 1

pthread_mutex_t mutex;
//pthread_mutex_init(&mutex,NULL);


// this represents work that has to be 
// completed by a thread in the pool
typedef struct 
{
    void (*function)(void *p);
    void *data;
}
task;

//the stop token
int token=1;

// the work queue
task workstodo[REAL_QUEUE_SIZE];
int head=0;
int tail=0;

// the worker bee
pthread_t bees[NUMBER_OF_THREADS];

// insert a task into the queue
// returns 0 if successful or 1 otherwise, 
int enqueue(task t) 
{
	
	if((tail+1)%(REAL_QUEUE_SIZE)==head)
	{
		printf("the worktodo queue is full. \n");
		return 1;
	}
	else
	{
		workstodo[tail]=t;
		tail=(tail+1)%(REAL_QUEUE_SIZE);
		return 0;
	}    

	
	
}

// remove a task from the queue
task dequeue() 
{
	
	task work_next;
	work_next=workstodo[head];
	head=(head+1)%(REAL_QUEUE_SIZE);
	
	
	return work_next;
	
}

// the worker thread in the thread pool
void *worker(void *param)
{
    // execute the task

	task work_next;
	while(token)
	{
		if(head==tail) usleep(3);
		else
		{
			pthread_mutex_lock(&mutex);
			work_next=dequeue();
			pthread_mutex_unlock(&mutex);
    			execute(work_next.function, work_next.data);
		}
	}

    pthread_exit(0);
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
    	task worktodo;
	worktodo.function = somefunction;
	worktodo.data = p;
	pthread_mutex_lock(&mutex);
	enqueue(worktodo);
	pthread_mutex_unlock(&mutex);
    return 0;
}

// initialize the thread pool
void pool_init(void)
{
    
	for(int i=0;i<NUMBER_OF_THREADS;i++)
	{
	
		pthread_create(&bees[i],NULL,worker,NULL);
	}

}

// shutdown the thread pool
void pool_shutdown(void)
{
    
	while(TRUE)
	{
		if(head==tail)
		{
			token=0;
			break;
		}
		else
		{
			usleep(1);
		}
	}
	for(int i=0;i<NUMBER_OF_THREADS;i++)
	{
	
		pthread_join(bees[i],NULL);
	}
}

























