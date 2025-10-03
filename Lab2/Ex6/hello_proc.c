#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>      
#include <linux/sched/task.h> 

static int __init hello_init(void) {
    printk(KERN_INFO "hello_proc: loaded by process \"%s\" (pid=%d)\n",
           current->comm, current->pid);
    return 0;
}
static void __exit hello_exit(void) {
 printk(KERN_INFO "hello_proc: unloaded\n");
}
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry");
MODULE_DESCRIPTION("Excersise 6");