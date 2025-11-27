#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/slab.h> 
#include <linux/mutex.h>

#define SCULL_QUANTUM 4000
#define SCULL_QSET    1000

struct scull_qset {
    void **data;
    struct scull_qset *next;
};

struct mychardev_data {
    struct scull_qset *data;
    int quantum;
    int qset;
    unsigned long size;
    dev_t devno;
    struct mutex lock;
    struct cdev my_cdev;
    struct class *my_class;
    struct device *my_device;
};

static struct mychardev_data mdev;

static char *my_devnode(const struct device *dev, umode_t *mode)
{
    if (mode) *mode = 0666;
    return NULL;
}

/* --- Memory Management --- */

static int mychardev_trim(struct mychardev_data *dev)
{
    struct scull_qset *next, *dptr;
    int qset = dev->qset;
    int i;

    for (dptr = dev->data; dptr; dptr = next) {
        if (dptr->data) {
            for (i = 0; i < qset; i++)
                kfree(dptr->data[i]);
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        kfree(dptr);
    }
    
    dev->size = 0;
    dev->quantum = SCULL_QUANTUM;
    dev->qset = SCULL_QSET;
    dev->data = NULL;
    return 0;
}

static struct scull_qset *mychardev_follow(struct mychardev_data *dev, int n)
{
    struct scull_qset *qs = dev->data;
    struct scull_qset *next_qs = NULL;

    if (!qs) {
        qs = kzalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (!qs) return NULL;
        dev->data = qs;
    }

    while (n--) {
        next_qs = qs->next;
        if (!next_qs) {
            next_qs = kzalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (!next_qs) return NULL;
            qs->next = next_qs;
        }
        qs = next_qs;
    }
    return qs;
}

/* --- File Operations --- */

static int mychardev_open(struct inode *inode, struct file *file)
{
    struct mychardev_data *dev;
    
    dev = container_of(inode->i_cdev, struct mychardev_data, my_cdev);
    file->private_data = dev;

    if ((file->f_flags & O_ACCMODE) == O_WRONLY && (file->f_flags & O_TRUNC)) {
        if (mutex_lock_interruptible(&dev->lock))
            return -ERESTARTSYS;
        mychardev_trim(dev);
        mutex_unlock(&dev->lock);
    }

    /* YÊU CẦU 1: In ra Minor number khi mở thiết bị */
    pr_info("Mychardev: Device opened (Minor: %d)\n", iminor(inode));
    
    return 0;
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    pr_info("Mychardev: Device released\n");
    return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct mychardev_data *dev = filp->private_data;
    struct scull_qset *dptr;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int item_size = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    item = (long)*f_pos / item_size;
    rest = (long)*f_pos % item_size;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    dptr = mychardev_follow(dev, item);

    if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
        goto out;

    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

    /* YÊU CẦU 2: In log số byte thực tế đã copy thành công tới user space */
    pr_info("Mychardev Read: Successfully copied %zu bytes to user\n", count);

out:
    mutex_unlock(&dev->lock);
    return retval;
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct mychardev_data *dev = filp->private_data;
    struct scull_qset *dptr;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int item_size = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = -ENOMEM;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    if (filp->f_flags & O_APPEND) {
        *f_pos = dev->size;
    }

    item = (long)*f_pos / item_size;
    rest = (long)*f_pos % item_size;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    dptr = mychardev_follow(dev, item);
    if (dptr == NULL)
        goto out;

    if (!dptr->data) {
        dptr->data = kzalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
    }

    if (!dptr->data[s_pos]) {
        dptr->data[s_pos] = kzalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos])
            goto out;
    }

    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_from_user(dptr->data[s_pos] + q_pos, buf, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

    if (dev->size < *f_pos)
        dev->size = *f_pos;

    /* YÊU CẦU 2: In log số byte thực tế đã copy thành công từ user space */
    pr_info("Mychardev Write: Successfully copied %zu bytes from user\n", count);

out:
    mutex_unlock(&dev->lock);
    return retval;
}

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = mychardev_open,
    .release = mychardev_release,
    .read = my_read,
    .write = my_write,
};

/* --- Init & Exit --- */

static int __init mychardev_init(void)
{
    int ret;

    mdev.quantum = SCULL_QUANTUM;
    mdev.qset = SCULL_QSET;
    mdev.size = 0;
    mdev.data = NULL;
    mutex_init(&mdev.lock);

    ret = alloc_chrdev_region(&mdev.devno, 0, 1, "mychardev");
    if (ret) return ret;

    cdev_init(&mdev.my_cdev, &my_fops);
    mdev.my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&mdev.my_cdev, mdev.devno, 1);
    if (ret) goto unreg_chrdev;

    /* Sử dụng macro kiểm tra version kernel để code chạy được trên mọi máy */
    mdev.my_class = class_create("mychardev_class");
    if (IS_ERR(mdev.my_class)) {
        ret = PTR_ERR(mdev.my_class);
        goto del_cdev;
    }
    mdev.my_class->devnode = my_devnode;

    mdev.my_device = device_create(mdev.my_class, NULL, mdev.devno, NULL, "mychardev0");
    if (IS_ERR(mdev.my_device)) {
        ret = PTR_ERR(mdev.my_device);
        goto destroy_class;
    }

    pr_info("mychardev: registered dynamic scull model (major=%d, minor=%d)\n",
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

static void __exit mychardev_exit(void)
{
    mychardev_trim(&mdev);
    device_destroy(mdev.my_class, mdev.devno);
    class_destroy(mdev.my_class);
    cdev_del(&mdev.my_cdev);
    unregister_chrdev_region(mdev.devno, 1);
    pr_info("mychardev: unregistered\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mychardev dynamic memory scull model with debug info");
MODULE_AUTHOR("Doan Duc Manh - UET, VNU");