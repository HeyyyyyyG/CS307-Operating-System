#include <stdio.h>
	#include <stdlib.h>
	#include <string.h> // memset
	#include <pthread.h> // pthread_t, pthread_create, pthread_join
	#include <semaphore.h> // sem_init, sem_wait, sem_post
	#include <time.h>
	
	#define NUM_SEAT 3
	
	#define SLEEP_MAX 5
	
	//sematphores for students and Teaching Assistant
	sem_t sem_stu;
	sem_t sem_ta;
	
	//mutex to insure the teaching process concurrency
	pthread_mutex_t mutex;
	
	int chair[3]; //chairs 3 and one student should be learning from TA
	int count = 0; //number of waiting students
	int next_seat = 0;
	int next_teach = 0;
	
	void rand_sleep(void);
	void* stu_programming();
	void* ta_teaching();
	int* student_ids;
	int student_num;
	int *studentStatus; //status of students - waiting/learning/finished learning 
	pthread_t *students;
	pthread_t ta;
	int j;
	int main(int argc, char **argv){
	
	
	//index
	int i;
	
	//get number of students from user
	
	printf("How many students coming to TA's office? ");
	scanf("%d", &student_num);
	
	//initialize all parameters
	students = (pthread_t*)malloc(sizeof(pthread_t) * student_num);
	student_ids = (int*)malloc(sizeof(int) * student_num);
	studentStatus=(int*)malloc(sizeof(int)*student_num);
	memset(student_ids, 0, student_num);
	
	sem_init(&sem_stu,0,0);
	sem_init(&sem_ta,0,1);
	
	//set random
	srand(time(NULL));
	
	//sem_init(&mutex,0,1);
	pthread_mutex_init(&mutex,NULL);
	
	//create TA thread
	pthread_create(&ta,NULL,ta_teaching,NULL);
	
	//create Students threads
	for(i=0; i<student_num; i++)
	{
	student_ids[i] = i+1;
	pthread_create(&students[i], NULL, stu_programming, (void*) &student_ids[i]);
	studentStatus[i]=0;
	}
	
	//joining main thread
	pthread_join(ta, NULL);
	
	for(i=0; i<student_num;i++)
	{
	pthread_join(students[i],NULL);
	}
	
	return 0;
	}
	
	void* stu_programming(void* stu_id)
	{
	int id = *(int*)stu_id;
	
	//printf("[stu] student %d is programming\n",id);
	
	while(1)
	{
		//printf("\nStudent Came is %d with status %d ",id,studentStatus[id-1]);
		rand_sleep();
		
		//sem_wait(&mutex);
		pthread_mutex_lock(&mutex);
		
		if(count < NUM_SEAT && studentStatus[id-1]==0)
		{
			chair[next_seat] = id;
			count++;
			// printf("\nstatus of student %d is %d",id,studentStatus[id-1]);
			
			//printf(" [stu] student %d is waiting\n",id);
			// printf("waiting students : [1] %d [2] %d [3] %d\n",chair[0],chair[1],chair[2]);
			next_seat = (next_seat+1) % NUM_SEAT;
			
			//sem_post(&mutex);
			pthread_mutex_unlock(&mutex);
			
			//wakeup ta
			
			sem_post(&sem_stu);
			sem_wait(&sem_ta);
			
		}
		else //no more chairs
		{
			//sem_post(&mutex);
			if(studentStatus[id-1]==1)
			{
			
				pthread_mutex_unlock(&mutex);
				rand_sleep();
				//killing threads if teaching is done for student by TA
				pthread_exit(NULL);
				
			}
			else
			{
			
				pthread_mutex_unlock(&mutex);
				printf("\n[stu] no more chairs. student %d going Home",id);
				//studentStatus[id-1]=2;
				studentStatus[id-1]=0;
				//killing extra threads if chair is not available i.e. semaphore is not available 
				//pthread_exit(NULL);
			}
		
		
		}
	}
	}
	
	void* ta_teaching()
	{
	while(1)
	{
	//waiting for students while sleeping in between
	sem_wait(&sem_stu);
	
	//sem_wait(&mutex);
	pthread_mutex_lock(&mutex);
	
	printf("\nwaiting students : [1] %d [2] %d [3] %d\n",chair[0],chair[1],chair[2]);
	printf("\n[ta] TA is teaching student %d\n",chair[next_teach]);
	
	count--;
	j=chair[next_teach];
	studentStatus[j-1]=1;
	chair[next_teach]=0;
	
	next_teach = (next_teach + 1) % NUM_SEAT;
	
	rand_sleep();
	
	printf("\n[ta] teaching finish.\n");
	
	
	
	if(chair[0]==0&&chair[1]==0&&chair[2]==0)
	{
	printf("\nTA is Sleeping");
	}
	
	//sem_post(&mutex);
	pthread_mutex_unlock(&mutex);
	//posting TA semaphore
	sem_post(&sem_ta);
	}
	}
	void rand_sleep(void){
	int time = rand() % SLEEP_MAX + 1;
	sleep(time);
	}
