#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include "buffer.h"

// buffer implemented as a cyclic queue
buffer_item buffer [BUFFER_SIZE];
int head, rear;

// mutex and semaphores
sem_t * full, * empty;
pthread_mutex_t mutex;

int initialize_buffer (void) {
	head = rear = 0;
	pthread_mutex_init (&mutex, NULL);
	full = sem_open ("SEM_full", O_CREAT, 0666, 0);
	empty = sem_open ("SEM_empty", O_CREAT, 0666, BUFFER_SIZE - 1);
	return 0;
}

int insert_item (buffer_item item) {
	sem_wait (empty);
	pthread_mutex_lock (&mutex);

	if ((rear + 1) % BUFFER_SIZE == head) {
		printf ("[error] buffer full but try to insert item\n");
		pthread_mutex_unlock (&mutex);
		return 1;
	} else {
		buffer[rear] = item;
		rear = (rear + 1) % BUFFER_SIZE;

		pthread_mutex_unlock (&mutex);
		sem_post (full);

		return 0;
	}
}

int remove_item (buffer_item * item) {
	sem_wait (full);
	pthread_mutex_lock (&mutex);

	if (head == rear) {
		printf ("[error] buffer empty but try to remove item\n");
		pthread_mutex_unlock (&mutex);
		return 1;
	} else {
		*item = buffer [head];
		head = (head + 1) % BUFFER_SIZE;

		pthread_mutex_unlock (&mutex);
		sem_post (empty);

		return 0;
	}
}
