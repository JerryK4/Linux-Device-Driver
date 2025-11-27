#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/slab.h> 
#include <linux/mutex.h>
#include <linux/moduleparam.h> // Cần thiết cho module_param

#define SCULL_QUANTUM 4000
#define SCULL_QSET    1000

/* --- YÊU CẦU 1: Thêm tham số module max_size --- */
static int max_size = 1024 * 1024; // Mặc định 1MB
module_param(max_size, int, 0644); // Cho phép root thay đổi qua sysfs
MODULE_PARM_DESC(max_size, "Maximum memory size for the device (bytes)");

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
    int open_count; 
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

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    if ((file->f_flags & O_ACCMODE) == O_WRONLY && (file->f_flags & O_TRUNC)) {
        mychardev_trim(dev);
    }

    dev->open_count++;
    pr_info("Mychardev: Device opened (Minor: %d) | Open Count: %d\n", 
            iminor(inode), dev->open_count);
    
    mutex_unlock(&dev->lock);
    return 0;
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    struct mychardev_data *dev = file->private_data;

    mutex_lock(&dev->lock);
    dev->open_count--;
    pr_info("Mychardev: Device closed | Open Count: %d\n", dev->open_count);
    mutex_unlock(&dev->lock);
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

    pr_info("Mychardev Read: Copied %zu bytes\n", count);

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

    /* Xử lý Append mode */
    if (filp->f_flags & O_APPEND) {
        *f_pos = dev->size;
    }

    /* --- YÊU CẦU 2: Kiểm tra giới hạn bộ nhớ --- */
    
    // 1. Nếu đã đầy -> Báo lỗi hết chỗ
    if (*f_pos >= max_size) {
        pr_warn("Mychardev: Device full! (Current: %lld, Max: %d)\n", *f_pos, max_size);
        retval = -ENOSPC; // Error: No space left on device
        goto out;
    }

    // 2. Nếu ghi thêm sẽ tràn -> Cắt bớt dữ liệu
    if (*f_pos + count > max_size) {
        pr_warn("Mychardev Warning: Data exceeds limit (%d). Truncating write.\n", max_size);
        count = max_size - *f_pos; // Chỉ ghi phần còn lại
    }

    /* Bắt đầu ghi dữ liệu */
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

    pr_info("Mychardev Write: Copied %zu bytes (Total size: %ld)\n", count, dev->size);

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

static int __init mychardev_init(void)
{
    int ret;

    mdev.quantum = SCULL_QUANTUM;
    mdev.qset = SCULL_QSET;
    mdev.size = 0;
    mdev.open_count = 0;
    mdev.data = NULL;
    mutex_init(&mdev.lock);

    ret = alloc_chrdev_region(&mdev.devno, 0, 1, "mychardev");
    if (ret) return ret;

    cdev_init(&mdev.my_cdev, &my_fops);
    mdev.my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&mdev.my_cdev, mdev.devno, 1);
    if (ret) goto unreg_chrdev;

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

    pr_info("mychardev: registered (max_size=%d bytes)\n", max_size);
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
MODULE_DESCRIPTION("mychardev scull with max_size limit");
MODULE_AUTHOR("Doan Duc Manh - UET, VNU");