#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

void *runner(void *param);

int a[]={10,9,8,7,6,5,4,3,2,1};
int array_length=sizeof(a)/sizeof(a[0]);
int *array_master;

int* copy(int a[])
{
	int *input=malloc(array_length*sizeof(int));	
	for(int i=0;i < array_length;i ++)
	{input[i]=a[i];
	//printf("%d",input[i];
	}

	return input;
}

void merge(int a[],int low,int high)
{
	int mid=(low+high)/2;
	int left=low;
	int right=mid+1;
	
	int b[high-low+1];
	int i,temp=0;

	while(left<=mid &&right<=high)
	{
		if(a[left]>a[right])
			b[temp++]=a[right++];
		else
			b[temp++]=a[left++];
	}
	
	while(left<=mid)
		b[temp++]=a[left++];

	while(right<=high)
		b[temp++]=a[right++];

	for(i=0;i<(high-low+1);i++)
		{
			//printf("%d",b[i]);
			a[low+i]=b[i];}
	//printf("%s\n","merged");

}
/*
void merge(int arr[],int left,int middle,int right)
{
	int half1 = middle-left+1;
	int half2 = right-middle;
	
	int firsthalf[half1],secondhalf[half2];//temp arrays

	int i,j;
	for(i=0;i<half1;i++)
	{
		firsthalf[i]=arr[left+i];
	}

	for(j=0;j<half2;j++)
	{
		secondhalf[j]=arr[middle+j+1];
	}

	i=0;j=0;
	int k=left;

	while(i<half1 && j<half2)
	{
		if(firsthalf[i]<=secondhalf[j])
		{
			arr[k]=firsthalf[i++];
			
		}
		else
		{
			arr[k]=secondhalf[j++];
		}
		k++;
	}

	while(i<half1)
	{
		arr[k]=firsthalf[i++];
		k++;
	}
	
	while(j<half2)
	{
		arr[k]=secondhalf[j++];
		k++;
	}

}
*/
void merge_sort(int arr[],int l,int r)
{
	if(l<r)
	{	
		int middle=(r+l)/2;
		merge_sort(arr,l,middle);
		
		merge_sort(arr,middle+1,r);
		merge(arr,l,r);

	}
}

int main(void)
{
	//thread 1
	pthread_t tid;//thread identifier
	pthread_attr_t attr;//set of thread attributes

	//thread 2
	pthread_t tid2;
	pthread_attr_t attr2;

	//thread 3
	pthread_t tid3;
	pthread_attr_t attr3;


	array_master = copy(a);

	char *thread1="first";
	pthread_attr_init(&attr);
	pthread_create(&tid,&attr,runner,thread1);

	char*thread2="second";
	pthread_attr_init(&attr2);
	pthread_create(&tid2,&attr2,runner,thread2);

	char*thread3="third";
	pthread_attr_init(&attr3);
	pthread_create(&tid3,&attr3,runner,thread3);

	pthread_join(tid,NULL);
	pthread_join(tid2,NULL);
	pthread_join(tid3,NULL);

	
	for(int i=0;i<array_length;i++)
	{
		printf("%d \t",array_master[i]);
	}

	pthread_exit(0);

	return 0;
}

void *runner(void *param)
{
	int mid=(array_length-1)/2;
	if(strcmp(param,"first")==0)
		{merge_sort(array_master,0,mid);}
	if(strcmp(param,"second")==0)
		{merge_sort(array_master,mid+1,array_length-1);}
	if(strcmp(param,"third")==0)
		{
			//printf("%s","thread3");
			merge(array_master,0,(array_length-1));}

	pthread_exit(0);
}
