#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int N = 1;
module_param(N, int, 0644);
MODULE_PARM_DESC(N, "Number of iterations");

static char *whom = "world";
module_param(whom, charp, 0644);
MODULE_PARM_DESC(whom, "Name to greet when the module is loaded");

static int __init hello_init(void) {
    int i;
    for (i = 0; i < N; i++) {
        printk(KERN_INFO "Hello, %s!\n", whom);
    }
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye, Kernel!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry");
MODULE_DESCRIPTION("Exercise 2 - Hello Parameter Module");
