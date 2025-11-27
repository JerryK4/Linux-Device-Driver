#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>   
#include <linux/err.h>      
#include <linux/slab.h>

#define BUF_LEN 1024
static char device_buf[BUF_LEN];
static size_t data_size;        
/* Structure to keep device data */
static struct mychardev_data {
    dev_t devno;
    struct cdev my_cdev;
    struct class *my_class;
    struct device *my_device;
} mdev;

static char *my_devnode(const struct device *dev, umode_t *mode)
{
    if (mode)
        *mode = 0666; /* rw-rw-rw- */
    return NULL;
}


static int mychardev_open(struct inode *inode, struct file *file)
{
    pr_info("Mychardev device opened\n");
    return 0; 
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    pr_info("Mychardev device released\n");
    return 0; 
}

/* --------- read/write --------------*/
static ssize_t my_read(struct file *filp, char __user *buf,
                       size_t count, loff_t *ppos)
{
    pr_info("my_read is called\n");
    if (*ppos >= data_size)
        return 0;

    
    if (count > data_size - *ppos)
        count = data_size - *ppos;

    if (copy_to_user(buf, device_buf + *ppos, count))
        return -EFAULT;

    *ppos += count;
    return count;
}

static ssize_t my_write(struct file *filp, const char __user *buf,
                        size_t count, loff_t *ppos)
{
    size_t space, to_copy;
    pr_info("my_write is called\n");
    
    if (*ppos >= BUF_LEN)
        return -ENOSPC;

    space   = BUF_LEN - *ppos;
    to_copy = (count > space) ? space : count;

    if (copy_from_user(device_buf + *ppos, buf, to_copy))
        return -EFAULT;

    *ppos += to_copy;
    if (data_size < *ppos)
        data_size = *ppos;

    return to_copy;
}

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = mychardev_open,       
    .release = mychardev_release, 
    .write = my_write,
    .read = my_read,
};

/* Module Initialization */
static int __init mychardev_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&mdev.devno, 0, 1, "mychardev");
    if (ret) {
        pr_err("Failed to allocate device number\n");
        return ret;
    }

    cdev_init(&mdev.my_cdev, &my_fops);
    mdev.my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&mdev.my_cdev, mdev.devno, 1);
    if (ret) {
        pr_err("Failed to add cdev\n");
        goto unreg_chrdev;
    }

    mdev.my_class = class_create("mychardev_class");
    if (IS_ERR(mdev.my_class)) {
        pr_err("Failed to create class\n");
        ret = PTR_ERR(mdev.my_class);
        goto del_cdev;
    }
    
    
    mdev.my_class->devnode = my_devnode;

   
    mdev.my_device = device_create(mdev.my_class, NULL, mdev.devno, NULL, "mychardev0");
    if (IS_ERR(mdev.my_device)) {
        pr_err("Failed to create device\n");
        ret = PTR_ERR(mdev.my_device);
        goto destroy_class;
    }

    pr_info("mychardev: registered (major=%d, minor=%d), node /dev/mychardev0 created\n",
            MAJOR(mdev.devno), MINOR(mdev.devno));
    return 0;

destroy_class:
    class_destroy(mdev.my_class);
del_cdev:
    cdev_del(&mdev.my_cdev);
unreg_chrdev:
    unregister_chrdev_region(mdev.devno, 1);
    return ret;
}

/* Module Cleanup */
static void __exit mychardev_exit(void)
{
    device_destroy(mdev.my_class, mdev.devno);
    class_destroy(mdev.my_class);
    cdev_del(&mdev.my_cdev);
    unregister_chrdev_region(mdev.devno, 1);
    pr_info("mychardev: unregistered and /dev/mychardev0 removed\n"); 
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mychardev char driver with auto-create device node");
MODULE_AUTHOR("Doan Duc Manh - UET, VNU");