import java.util.Arrays;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;
import java.util.concurrent.ThreadLocalRandom;


public class Mergesort
{
	private static final int ARRAY_SIZE_FOR_SOLO =10;
	private static final int ARRAY_SIZE=100;
	private static final int MIN_VALUE=0;
	private static final int MAX_VALUE=100;

	public static void main(String[] args)
	{
		int[] arr = ThreadLocalRandom.current().ints(ARRAY_SIZE,MIN_VALUE,MAX_VALUE).toArray();
		System.out.println(Arrays.toString(arr));//?
		ForkJoinPool.commonPool().invoke(new MyRecursiveAction(arr));
		System.out.println(Arrays.toString(arr));
	}

	private static class MyRecursiveAction extends RecursiveAction
	{
		private int[] arr;
	
		public MyRecursiveAction(int[] arr)
		{
			this.arr=arr;
		}
		
		public void compute()
		{
			if(arr.length < 2) return;
	
			int mid = arr.length >>> 1;
			int[] arr1 = Arrays.copyOfRange(arr,0,mid);
			int[] arr2 = Arrays.copyOfRange(arr,mid,arr.length);
			if(arr.length < ARRAY_SIZE_FOR_SOLO)
				selection_sort(arr);
			else
			{
				invokeAll(new MyRecursiveAction(arr1),new MyRecursiveAction(arr2));
				merge(arr,arr1,arr2);
			}

		}

		public void merge_sort(int[] arr)
		{
			if(arr.length < 2) return;
		
			int mid=arr.length >>> 1;
			int[] arr1 = Arrays.copyOfRange(arr,0,mid);
			int[] arr2 = Arrays.copyOfRange(arr,mid,arr.length);
			merge_sort(arr1);
			merge_sort(arr2);
			merge(arr,arr1,arr2);
		}

		private void merge(int[] arr,int[] arr1,int[] arr2)
		{
			int i=0,j=0,k=0;
			while(i<arr1.length && j<arr2.length)
			{
				if(arr1[i]<=arr2[j])
					arr[k++]=arr1[i++];
				else
					arr[k++]=arr2[j++];
			}

			while(i<arr1.length)
				arr[k++]=arr1[i++];
	
			while(j<arr2.length)
				arr[k++]=arr2[j++];

		}

		public int[] selection_sort(int[] a)
		{
			if(a==null) return a;

		for(int i=0;i<a.length-1;i++)
		{
			int min=i;
			for(int j=i+1;j<a.length;j++)
			{
				if(a[min]>a[j])
					min=j;
			}
	
			int temp=a[i];
			a[i]=a[min];
			a[min]=temp;
		}

		return a;
		}
	}
}


























