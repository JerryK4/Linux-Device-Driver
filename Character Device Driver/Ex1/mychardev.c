#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>       
#include <linux/cdev.h>     
#include <linux/device.h>   
#include <linux/uaccess.h>  
#include <linux/slab.h>

static dev_t devno;
static struct cdev my_cdev;

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    // Add other file operations as needed
};

static int __init mychardev_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&devno, 0, 1, "mychardev");
    if (ret) return ret;

    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;
    ret = cdev_add(&my_cdev, devno, 1);
    if (ret) {
        unregister_chrdev_region(devno, 1);
        return ret;
    }

    pr_info("mychardev: registered (major=%d, minor=%d)\n",
            MAJOR(devno), MINOR(devno));
    return 0;
}

static void __exit mychardev_exit(void)
{
    cdev_del(&my_cdev);
    unregister_chrdev_region(devno, 1);
    pr_info("mychardev: unregistered\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mychardev basic char device driver");
MODULE_AUTHOR("Doan Duc Manh - UET, VNU");