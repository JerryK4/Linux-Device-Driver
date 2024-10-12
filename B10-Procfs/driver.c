#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/ioctl.h>
#include<linux/proc_fs.h>
#include<linux/err.h> 

#define LINUX_KERNEL_VERSION 515 

#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

int32_t value = 0;
char m_array[20]="try_proc_array\n";
static int len = 1;

struct m_foo_dev{
    dev_t dev;
    struct class *m_class;
    struct cdev m_cdev;
    struct proc_dir_entry *parent;
}mdev;

/*
** Function Prototypes
*/
static int      __init m_driver_init(void);
static void     __exit m_driver_exit(void);
/*************** Driver Functions **********************/
static int      m_open(struct inode *inode, struct file *file);
static int      m_release(struct inode *inode, struct file *file);
static ssize_t  m_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  m_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     m_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 
/***************** Procfs Functions *******************/
static int      open_proc(struct inode *inode, struct file *file);
static int      release_proc(struct inode *inode, struct file *file);
static ssize_t  read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset);
static ssize_t  write_proc(struct file *filp, const char *buff, size_t len, loff_t * off);

/*
** File operation sturcture
*/
static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .read = m_read,
    .write = m_write,
    .open = m_open,
    .release = m_release,
    .unlocked_ioctl = m_ioctl,
};

/*
** procfs operation sturcture
*/
static struct proc_ops proc_fops = {
    .proc_open = open_proc,
    .proc_read = read_proc,
    .proc_write = write_proc,
    .proc_release = release_proc
};

/*This function will be called when we open the procfs file*/
static int open_proc(struct inode *inode, struct file *file)
{
    pr_info("proc file opend...\n");
    return 0;
}
/*This function will be called when we release the procfs file*/
static int release_proc(struct inode *indoe,struct file *file)
{
    pr_info("proc file released...\n");
    return 0;
}
/*This function will be called when we read the procfs file*/
static ssize_t  read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset)
{
    pr_info("proc file read...\n");
    if(len){
        len=0;
    }else{
        len=1;
        return 0;
    }
    if(copy_to_user(buffer,m_array,20)){
        pr_err("Data send: Err!\n");
    }
    return length;
}
/*This function will be called when we write the procfs file*/
static ssize_t  write_proc(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    pr_info("proc file write...\n");
    if(copy_from_user(m_array,buff,len)){
        pr_err("Data write: Err!\n");
    }
    return len;
}
/*
** This function will be called when we open the Device file
*/
static int m_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*
** This function will be called when we close the Device file
*/
static int m_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t m_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read function\n");
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t m_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write Function\n");
        return len;
}

/*
** This function will be called when we write IOCTL on the Device file
*/
static long     m_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd){
        case WR_VALUE:
            if(copy_from_user(&value,(int32_t*)arg,sizeof(value))){
                pr_err("Data write: Err!\n");
            }
            pr_info("Value = %d\n",value);
            break;
        case RD_VALUE:
            if(copy_to_user((int32_t*)arg,&value,sizeof(value))){
                pr_err("Data read: Err!\n");
            }
            break;
        default:
            pr_info("Default\n");
            break;
    }
    return 0;
}

static int      __init m_driver_init(void){
    /*Allocate device number*/
    if(alloc_chrdev_region(&mdev.dev,0,1,"m_dev")<0){
        pr_info("Cannot allocate device number\n");
        return -1;
    }
    /*Creating cdev structure*/
    cdev_init(&mdev.m_cdev,&fops);
    /*Adding character device to the system*/
    if(cdev_add(&mdev.m_cdev,mdev.dev,1)<0){
        pr_info("Cannot add the device to the system\n");
        goto rm_class;
    }
    /*Creating struct class*/
    if(IS_ERR(mdev.m_class=class_create(THIS_MODULE,"m_class"))){
        pr_info("Cannot create the struct class\n");
        goto rm_class;
    }
    /*Creating device*/
    if(IS_ERR(device_create(mdev.m_class,NULL,mdev.dev,NULL,"m_device"))){
        pr_info("Cannot create the Divce\n");
        goto rm_device;
    }
    /*Create proc directory. It will create a directory under "/proc" */
    mdev.parent = proc_mkdir("jerry",NULL);
    if(mdev.parent==NULL)
    {
        pr_info("Error creating proc entry");
        goto rm_device;
    }
    /*Creating Proc entry under "/proc/etx/" */
    proc_create("m_proc", 0666, mdev.parent, &proc_fops);
 
    pr_info("Device Driver Insert...Done!!!\n");
    return 0;
rm_device:
    class_destroy(mdev.m_class);
rm_class:
    unregister_chrdev_region(mdev.dev,1);
    return -1;
}

static void __exit m_driver_exit(void)
{
    proc_remove(mdev.parent);
    device_destroy(mdev.m_class,mdev.dev);
    class_destroy(mdev.m_class);
    cdev_del(&mdev.m_cdev);
    unregister_chrdev_region(mdev.dev,1);
    pr_info("Device Driver Remove...Done!!!\n");
}

module_init(m_driver_init);
module_exit(m_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry <JerryFromUET@gmail.com>");
MODULE_DESCRIPTION("Simple Linux device driver (procfs)");
MODULE_VERSION("1.6");
