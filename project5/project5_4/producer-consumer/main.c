#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "buffer.h"

#define MAX_SLEEP 4
#define MAX_ITEM 99

void * producer (void * param);
void * consumer (void * param);

int main (int argc, char * argv[]) {
	int wait_time, num_producer, num_consumer;
	pthread_t * producers, * consumers;
	int i;

	/* 1. Get arguments */
	printf("How long to sleep before terminating?");
	scanf("%d", &wait_time);
	printf("The number of producer threads?");
	scanf("%d",&num_producer);
	printf("The number of consumer threads?");
	scanf("%d",&num_consumer);
/*
	if (argc < 4) {
		printf ("[error] not enough argument. 3 expected, only get %d\n", argc-1);
		return 1;
	}
	
	wait_time = atoi (argv[1]);
	num_producer = atoi (argv[2]);
	num_consumer = atoi (argv[3]);
*/
	/* 2. Initialize buffer */
	initialize_buffer ();

	/* 3. Create producer threads */
	producers = (pthread_t *) malloc (num_producer * sizeof (pthread_t));
	for (i = 0; i < num_producer; ++ i) {
		pthread_create (&producers[i], NULL, producer, NULL);
	}

	/* 4. Create consumer threads */
	consumers = (pthread_t *) malloc (num_consumer * sizeof (pthread_t));
	for (i = 0; i < num_consumer; ++ i) {
		pthread_create (&consumers[i], NULL, consumer, NULL);
	}

	/* 5. Sleep */
	sleep (wait_time);

	/* 6. Exit */
	for (i = 0; i < num_producer; ++ i) {
		pthread_cancel (producers[i]);
	}
	for (i = 0; i < num_consumer; ++ i) {
		pthread_cancel (consumers[i]);
	}
	for (i = 0; i < num_producer; ++ i) {
		pthread_join (producers[i], NULL);
	}
	for (i = 0; i < num_consumer; ++ i) {
		pthread_join (consumers[i], NULL);
	}
	free (producers);
	free (consumers);
	
	return 0;
}

void * producer (void * param) {
	buffer_item item;

	while (true) {
		sleep (rand() % MAX_SLEEP);
		item = rand () % MAX_ITEM;
		if (insert_item (item)) {
			;
		} 
		else {
			printf ("producer produced %d\n", item);
		}
	}
}

void * consumer (void * param) {
	buffer_item item;

	while (true) {
		sleep (rand() % MAX_SLEEP);
		if (remove_item (&item)) {
			;
		} else {
			printf ("consumer consumed %d\n", item);
		}
	}
}
