#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>      
#include <linux/sched/task.h> 

static int val = 0;
module_param(val, int, 0644);
MODULE_PARM_DESC(val, "Input parameter");

static int __init hello_init(void) {
    printk(KERN_INFO "hello_sysfs: loaded with val=%d\n", val);
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "hello_sysfs: unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry");
MODULE_DESCRIPTION("Excersise 7 - sysfs");
