#include<linux/module.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
#define DRIVER_DESC "hello kernel module"
#define DRIVER_VERS "1.0"

#define NPAGES 1
struct m_foo_device{
    int32_t size;
    char *kmalloc_ptr;
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;
}mdev;

/*Functions prototypes*/
static int      __init chrdev_init(void);
static void     __exit chrdev_exit(void);
static int      m_open(struct inode *inode, struct file *file);
static int      m_release(struct inode *inode, struct file *file);
static ssize_t  m_read(struct file *filp, char __user *user_buf, size_t size,loff_t * offset);
static ssize_t  m_write(struct file *filp, const char *user_buf, size_t size, loff_t * offset);

static struct file_operations fops=
{
    .owner=THIS_MODULE,
    .open=m_open,
    .release=m_release,
    .read=m_read,
    .write=m_write,
};

/* This function will be called when we open the Device file */
static int m_open(struct inode *inode, struct file *file)
{
    pr_info("System call open() called...!!!\n");
    return 0;
}

/* This function will be called when we close the Device file */
static int m_release(struct inode *inode, struct file *file)
{
    pr_info("System call close() called...!!!\n");
    return 0;
}

/* This function will be called when we read the Device file */
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
    size_t to_read;
    pr_info("System call read() called...!!!\n");

    /*Check size doesn't excedd our mapped area size*/
    to_read=(size>mdev.size-*offset)?(mdev.size-*offset):size;
    /*Copy from mapped area to user buffer*/
    if(copy_to_user(user_buf,mdev.kmalloc_ptr+*offset,to_read))
        return -EFAULT;
    
    *offset+=to_read;
    return to_read;
}

/* This function will be called when we write the Device file */
static ssize_t m_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset)
{
    size_t to_write;
    pr_info("System call write() called...!!!\n");
    /*Check size doesn't excedd our mapped are size*/
    to_write=(size+*offset>NPAGES*PAGE_SIZE)?(NPAGES*PAGE_SIZE-*offset):size;
    /*Copy from user buffer to mapped area*/
    memset(mdev.kmalloc_ptr,0,NPAGES*PAGE_SIZE);
    if(copy_from_user(mdev.kmalloc_ptr+*offset,user_buf,to_write)!=0)
        return -EFAULT;

    pr_info("Data from usr: %s",mdev.kmalloc_ptr);

    *offset+=to_write;
    mdev.size=*offset;
    return to_write;
}

static int __init chrdev_init(void)
{
    /*1.0 Dynamic allocating device number (cat /proc/devices)*/
    if(alloc_chrdev_region(&mdev.dev_num,0,1,"m_device_number")<0)
    {
        pr_err("Failed to alocate device number!\n");
        return 0;
    }
    pr_info("Major:%d Minor:%d\n",MAJOR(mdev.dev_num),MINOR(mdev.dev_num));

    /*2.0-Create struct class*/
    if((mdev.m_class=class_create(THIS_MODULE,"m_class"))==NULL)
    {
        pr_err("Failed to create struct class!\n");
        goto rm_device_numb;
    }

    /*3.0-Create device*/
    if(device_create(mdev.m_class,NULL,mdev.dev_num,NULL,"m_device")==NULL)
    {
        pr_err("Can't create my device!\n");
        goto rm_class;
    }

    /*4.0-Create cdev structure*/
    cdev_init(&mdev.m_cdev,&fops);
    /*4.1-Adding character device to the system*/
    if(cdev_add(&mdev.m_cdev,mdev.dev_num,1)<0)
    {
        pr_err("Can't add the device to the system!\n");
        goto rm_device;
    }
 
    mdev.kmalloc_ptr=kmalloc(NPAGES*PAGE_SIZE, __GFP_FS);

    pr_info("Hello kernel module!\n");
    return 0;

rm_device:
    device_destroy(mdev.m_class,mdev.dev_num);
rm_class:
    class_destroy(mdev.m_class);
rm_device_numb:
    unregister_chrdev_region(mdev.dev_num,1);
    return -1;
}  

static void __exit chrdev_exit(void)
{
    kfree(mdev.kmalloc_ptr);                    /*5.0*/
    cdev_del(&mdev.m_cdev);                     /*4.0*/
    device_destroy(mdev.m_class,mdev.dev_num);  /*3.0*/
    class_destroy(mdev.m_class);                /*2.0*/
    unregister_chrdev_region(mdev.dev_num,1);   /*1.0*/
    pr_info("Goodbye!\n");
}

module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);  
MODULE_VERSION(DRIVER_VERS);