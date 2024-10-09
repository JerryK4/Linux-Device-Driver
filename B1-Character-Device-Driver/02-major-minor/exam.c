#include<linux/module.h>
#include<linux/fs.h>

#define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
#define DRIVER_DESC "hello kernel module"
#define DRIVER_VERS "1.0"

struct m_foo_device{
    dev_t dev_num;
}mdev;

static int __init chrdev_init(void)
{
    /*01-Dynamic allocating device number (cat /proc/devices)*/
    if(alloc_chrdev_region(&mdev.dev_num,0,1,"m_device_number")<0){
        pr_err("Failed to allocate device number!\n");
        return -1;
    }

    /*02-Static allocating device number*/
    //mdev.dev_num=MKDEV(173,0);
    //register_chrdev_region(mdev.dev_num,1,"m_device_number");
    pr_info("Major:%d Minor:%d\n",MAJOR(mdev.dev_num),MINOR(mdev.dev_num));

    pr_info("Hello kernel module!\n");
    pr_info("%s---%d\n",__func__,__LINE__);
    return 0;
}

static void __exit chrdev_exit(void)
{
    unregister_chrdev_region(mdev.dev_num,1);/*1.0*/
    pr_info("Goodbye!\n");
}
module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);  
MODULE_VERSION(DRIVER_VERS);