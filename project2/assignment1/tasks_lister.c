#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>


int tasks_lister_init(void)
{
	printk(KERN_INFO "Loading module...Please wait\n");

	struct task_struct *task;

	for_each_process(task)
	{
		
		printk(KERN_INFO "pid = [%d] command = [%s] state = [%ld] \n",task->pid,task->comm,task->state);
	}

	printk(KERN_INFO "Module tasks_lister loaded.\n");
	
	return 0;
}

void tasks_lister_exit(void)
{
	printk(KERN_INFO "Module tasks_lister removed.\n");
}

module_init(tasks_lister_init);
module_exit(tasks_lister_exit);
