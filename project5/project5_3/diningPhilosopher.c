// Dining Philosophers problem
	#include<stdio.h>
	#include <stdlib.h>
	#include<semaphore.h>
	#include<pthread.h>
	#include <time.h>
	
	#define N 5
	#define THINKING 0
	#define HUNGRY 1
	#define EATING 2
	#define LEFT (ph_num+4)%N
	#define RIGHT (ph_num+1)%N
	
	

	
	sem_t mutex;
	sem_t S[N];
	
	void * philospher(void *num);
	void take_fork(int);
	void put_fork(int);
	void test(int);
	
	int state[N];
	int phil_num[N]={0,1,2,3,4};
	
	int main()
	{
		int i;
		pthread_t thread_id[N];
		sem_init(&mutex,0,1);
		for(i=0;i<N;i++)
			sem_init(&S[i],0,0);
		for(i=0;i<N;i++)
		{
			pthread_create(&thread_id[i],NULL,philospher,&phil_num[i]);
			printf("Philosopher %d is thinking\n",i+1);
		}
		for(i=0;i<N;i++)
			pthread_join(thread_id[i],NULL);
	}
	
	void *philospher(void *num)
	{
		srand(time(NULL));	int time1=rand()%3+1;	
		while(1)
		{
			int *i = num;
			sleep(time1);
			take_fork(*i);
			sleep(time1);
			put_fork(*i);
			//if(state[*i]==EATING) break;
		}
	}
	
	void take_fork(int ph_num)
	{
		srand(time(NULL));	int time1=rand()%3+1;	
		sem_wait(&mutex);
		state[ph_num] = HUNGRY;
		printf("Philosopher %d is Hungry\n",ph_num+1);
		test(ph_num);
		sem_post(&mutex);
		sem_wait(&S[ph_num]);
		sleep(time1);
	}
	
	void test(int ph_num)
	{
		srand(time(NULL));	int time1=rand()%3+1;	
		if (state[ph_num] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
		{
		state[ph_num] = EATING;
		sleep(time1);
		printf("Philosopher %d takes fork %d and %d\n",ph_num+1,LEFT+1,ph_num+1);
		printf("Philosopher %d is Eating\n",ph_num+1);
		sem_post(&S[ph_num]);
		//pthread_exit(NULL);
		}
	}

	
	void put_fork(int ph_num)
	{
	sem_wait(&mutex);
	state[ph_num] = THINKING;
	printf("Philosopher %d putting fork %d and %d down\n",ph_num+1,LEFT+1,ph_num+1);
	printf("Philosopher %d GONE\n",ph_num+1);
	test(LEFT);
	test(RIGHT);
	sem_post(&mutex);
	pthread_exit(NULL);
	}
