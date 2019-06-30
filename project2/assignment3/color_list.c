#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>

#define NUM_OF_COLOR 4

typedef struct color
{
	int red;
	int blue;
	int green;

	struct list_head list;
}color;

LIST_HEAD(color_list); //Declare and init the head of the linked list

int color_list_init(void)
{
	printk(KERN_INFO "lOADING MODULE,PLEASE WAIT...\n");

	color *yanse;
	int i=0;
	for(i=0;i<NUM_OF_COLOR;i++)
	{
		yanse=kmalloc(sizeof(*yanse),GFP_KERNEL);
		yanse->red=138+i;
		yanse->blue=43+i;
		yanse->green=226+i;

		INIT_LIST_HEAD(&yanse->list);

		list_add_tail(&yanse->list,&color_list);
	}

	//go through the list and print
	color *ptr;
	list_for_each_entry(ptr,&color_list,list)
	{
		printk(KERN_INFO "color: red=%d blue=%d green=%d\n",ptr->red,ptr->blue,ptr->green);
	}

	return 0;
}

void color_list_exit(void)
{
	printk(KERN_INFO "Removing Module color_list\n");

	color *ptr,*next;
	list_for_each_entry_safe(ptr,next,&color_list,list)
	{
		printk(KERN_INFO "Removing color: red=%d blue=%d green=%d\n",ptr->red,ptr->blue,ptr->green);

		list_del(&ptr->list);

		kfree(ptr);
	}

	printk(KERN_INFO "Memory free done\n");
}

module_init(color_list_init);
module_exit(color_list_exit);
