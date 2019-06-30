#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>

static int start=25;
module_param(start,int,0);

typedef struct collatz
{
	int number;
	
	struct list_head list;
}collatz;

LIST_HEAD(collatz_list); //Declare and init the head of the linked list

int collatz_function(int n)
{
	if (n%2==0) return n/2;
	else return (3*n+1);
}

int collatz_list_init(void)
{
	printk(KERN_INFO "lOADING MODULE,PLEASE WAIT...\n");

	collatz *c;
	
	int previous_start=start;
	while(!(previous_start==1&&start==4))
	{
		c=kmalloc(sizeof(*c),GFP_KERNEL);
		c->number=start;
		

		INIT_LIST_HEAD(&c->list);

		list_add_tail(&c->list,&collatz_list);

		previous_start=start;
		start=collatz_function(start);
	}
	/*//start==1
	c=kmalloc(sizeof(*c),GFP_KERNEL);
	c->number=start;

	INIT_LIST_HEAD(&c->list);

	list_add_tail(&c->list,&collatz_list);*/

	//go through the list and print
	collatz *ptr;
	list_for_each_entry(ptr,&collatz_list,list)
	{
		printk(KERN_INFO "%d ",ptr->number);
	}

	return 0;
}

void collatz_list_exit(void)
{
	printk(KERN_INFO "Removing Module collatz_list\n");

	collatz *ptr,*next;
	list_for_each_entry_safe(ptr,next,&collatz_list,list)
	{
		printk(KERN_INFO "Removing number %d \n",ptr->number);

		list_del(&ptr->list);

		kfree(ptr);
	}

	printk(KERN_INFO "Memory free done\n");
}

module_init(collatz_list_init);
module_exit(collatz_list_exit);
