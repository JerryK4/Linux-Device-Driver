#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int n = 1;
module_param(n, int, 0644);
MODULE_PARM_DESC(n, "Number of iterations");

static int __init hello_init(void) {
    int i;
    int sum = 0;
    for(i=1; i<=n; i++) {
        printk(KERN_INFO "i = %d\n", i);
        sum += i*i;
    }
    printk(KERN_INFO "Sum = %d\n",sum);
    return 0;
}
static void __exit hello_exit(void) {
 printk(KERN_INFO "Done with n = %d\n", n);
}
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry");
MODULE_DESCRIPTION("Excersise 3");
