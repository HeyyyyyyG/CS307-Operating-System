#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>

#define BUFFER_SIZE 128
#define PROC_NAME "pid"

ssize_t proc_read (struct file * file, char __user * usr_buf,
		size_t count, loff_t * pos);

ssize_t proc_write (struct file * file, const char __user * usr_buf,
		size_t count, loff_t * pos);

long pid;

static struct file_operations proc_ops = {
	.owner = THIS_MODULE,
	.read = proc_read,
	.write = proc_write
};

int proc_init (void) {
	proc_create (PROC_NAME, 0666, NULL, &proc_ops);
	return 0;
}

void proc_exit (void) {
	remove_proc_entry (PROC_NAME, NULL);
}

ssize_t proc_write (struct file * file, const char __user * usr_buf,
                size_t count, loff_t * pos) {
	//int rv = 0;
	char * k_mem;

	/* allocate kernel memory */
	k_mem = kmalloc (count, GFP_KERNEL);

	/* copy user space usr_buf to kernel memoey */
	copy_from_user (k_mem, usr_buf, count);
	
	k_mem [count-1] = '\0';
	printk (KERN_INFO "count=%d\n", (int)count);
	printk (KERN_INFO "%s\n", k_mem);
	kstrtol (k_mem, 10, &pid);
	printk (KERN_INFO "pid=%ld\n", pid);

	/* free kernel memory */
	kfree (k_mem);

	return count;
}

ssize_t proc_read (struct file * file, char __user * usr_buf,
                size_t count, loff_t * pos) {
	int rv = 0;
	char buffer [BUFFER_SIZE];
	static int completed = 0;
	struct task_struct * pcb;

	if (completed) {
		completed = 0;
		return 0;
	}

	completed = 1;

	pcb = pid_task (find_vpid (pid), PIDTYPE_PID);
	if (pcb == NULL) {
		rv = sprintf (buffer, "process pid=%ld not found.\n", pid);
	} else {
		rv = sprintf (buffer, "command = [%s] pid = [%ld] state = [%ld]\n", 
				pcb->comm, pid, pcb->state);
	}
	copy_to_user (usr_buf, buffer, rv);

	return rv;
}

module_init (proc_init);
module_exit (proc_exit);

MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("Task information.");
MODULE_AUTHOR ("SGG");
