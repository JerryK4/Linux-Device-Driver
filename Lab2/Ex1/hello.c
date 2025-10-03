#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int __init hello_init(void) {
 printk(KERN_INFO "Hello, Kernel!\n");
 return 0;
}
static void __exit hello_exit(void) {
 printk(KERN_INFO "Goodbye, Kernel!\n");
}
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry");
MODULE_DESCRIPTION("Excersise 1 - Hello Kernel Module");