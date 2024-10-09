#include<linux/module.h>

#define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
#define DRIVER_DESC "hello kernel module"
#define DRIVER_VERS "1.0"

static int __init chrdev_init(void)
{
    pr_info("Hello kernel module!\n");
    pr_info("%s---%d\n",__func__,__LINE__);
    return 0;
}

static void __exit chrdev_exit(void)
{
    pr_info("Goodbye!\n");
}
module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);  
MODULE_VERSION(DRIVER_VERS);