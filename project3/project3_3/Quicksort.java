import java.util.Arrays;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;
import java.util.concurrent.ThreadLocalRandom;


public class Quicksort
{
	private static final int ARRAY_SIZE_FOR_SOLO =10;
	private static final int ARRAY_SIZE=100;
	private static final int MIN_VALUE=0;
	private static final int MAX_VALUE=100;

	public static void main(String[] args)
	{
		int[] arr = ThreadLocalRandom.current().ints(ARRAY_SIZE,MIN_VALUE,MAX_VALUE).toArray();
		System.out.println(Arrays.toString(arr));
		ForkJoinPool.commonPool().invoke(new MyRecursiveAction(arr));
		System.out.println(Arrays.toString(arr));
	}

	private static class MyRecursiveAction extends RecursiveAction
	{
		private int[] arr;
		private int from;
		private int to;

		private static boolean flag=true;

		public MyRecursiveAction(int[]arr)
		{
			this(arr,0,arr.length);
		}

		private MyRecursiveAction(int[] arr,int from,int to)
		{
			this.from=from;
			this.to=to;
			this.arr=arr;
		}

		public void compute()
		{
			//System.out.println(Thread.currentThread());
			if(to-from < 2) return;

			int pivotIndex = findPivotIndex(from,to);
			invokeAll(new MyRecursiveAction(arr,from,pivotIndex),new MyRecursiveAction(arr,pivotIndex +1,to));

		}

		private int findPivotIndex(int from,int to)
		{
			if(to-from < 2) return from;

			int pivot = arr[from];
			int i=from,j=from+1;
			do
			{
				if(arr[j]<=pivot)
				{
					i++;
					swapInArr(i,j);
				}
			}while (++j < to);

			swapInArr(from,i);
		
			if(to-from <ARRAY_SIZE_FOR_SOLO)
				selection_sort(arr,from,to);

			return i;
		}

		private void swapInArr(int first,int second)
		{
			int temp=arr[first];
			arr[first]=arr[second];
			arr[second]=temp;
		}
		
		public int[] selection_sort(int[] a,int from,int to)
		{
			if(a==null) return a;

			for(int i=from;i<to;i++)
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

			
