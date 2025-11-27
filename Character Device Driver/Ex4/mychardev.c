#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/slab.h> // kmalloc, kfree
#include <linux/mutex.h> // Để đồng bộ hóa truy cập

/* Các hằng số định nghĩa kích thước bộ nhớ theo mô hình Scull */
#define SCULL_QUANTUM 4000
#define SCULL_QSET    1000

/* Cấu trúc 1 node trong danh sách liên kết (Quantum Set) */
struct scull_qset {
    void **data;              // Mảng các con trỏ trỏ tới dữ liệu thực
    struct scull_qset *next;  // Con trỏ tới node tiếp theo
};

/* Cấu trúc thiết bị chính (Device Structure) */
struct mychardev_data {
    struct scull_qset *data;  // Con trỏ tới node đầu tiên của danh sách
    int quantum;              // Kích thước 1 quantum (4000 bytes)
    int qset;                 // Kích thước mảng qset (1000 items)
    unsigned long size;       // Tổng lượng dữ liệu đang lưu
    dev_t devno;              // Số hiệu thiết bị
    struct mutex lock;        // Mutex để tránh race condition
    struct cdev my_cdev;
    struct class *my_class;
    struct device *my_device;
};

/* Khai báo biến toàn cục quản lý thiết bị */
static struct mychardev_data mdev;

/* Hàm cấp quyền rw-rw-rw- */
static char *my_devnode(const struct device *dev, umode_t *mode)
{
    if (mode) *mode = 0666;
    return NULL;
}

/* ================================================================
 * MEMORY MANAGEMENT FUNCTIONS (MÔ HÌNH SCULL)
 * ================================================================ */

/* Hàm giải phóng toàn bộ bộ nhớ của thiết bị */
static int mychardev_trim(struct mychardev_data *dev)
{
    struct scull_qset *next, *dptr;
    int qset = dev->qset; /* "dev" is not null */
    int i;

    /* Duyệt qua danh sách liên kết */
    for (dptr = dev->data; dptr; dptr = next) {
        if (dptr->data) {
            /* Giải phóng từng quantum trong mảng */
            for (i = 0; i < qset; i++)
                kfree(dptr->data[i]);
            /* Giải phóng mảng con trỏ */
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        /* Giải phóng chính cấu trúc qset */
        kfree(dptr);
    }
    
    dev->size = 0;
    dev->quantum = SCULL_QUANTUM;
    dev->qset = SCULL_QSET;
    dev->data = NULL;
    return 0;
}

/* Hàm di chuyển tới node Qset mong muốn (Tạo mới nếu chưa có) */
static struct scull_qset *mychardev_follow(struct mychardev_data *dev, int n)
{
    struct scull_qset *qs = dev->data;
    struct scull_qset *next_qs = NULL;

    /* Nếu chưa có node đầu tiên, cấp phát nó */
    if (!qs) {
        qs = kzalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (!qs) return NULL;
        dev->data = qs;
    }

    /* Di chuyển dọc theo danh sách liên kết */
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

/* ================================================================
 * FILE OPERATIONS
 * ================================================================ */

static int mychardev_open(struct inode *inode, struct file *file)
{
    struct mychardev_data *dev;

    /* Lấy cấu trúc dữ liệu từ inode */
    dev = container_of(inode->i_cdev, struct mychardev_data, my_cdev);
    file->private_data = dev;

    /* --- SỬA ĐỔI QUAN TRỌNG --- */
    /* Chỉ Trim (xóa) khi có cờ O_TRUNC (tức là dùng dấu >) 
     * Nếu dùng >> (O_APPEND), cờ O_TRUNC sẽ không được bật, dữ liệu sẽ được giữ nguyên.
     */
    if (file->f_flags & O_TRUNC) {
        if (mutex_lock_interruptible(&dev->lock))
            return -ERESTARTSYS;
        mychardev_trim(dev);
        mutex_unlock(&dev->lock);
    }

    pr_info("Mychardev device opened\n");
    return 0;
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    pr_info("Mychardev device released\n");
    return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct mychardev_data *dev = filp->private_data;
    struct scull_qset *dptr; // Con trỏ tới node list hiện tại
    int quantum = dev->quantum;
    int qset = dev->qset;
    int item_size = quantum * qset; /* Số byte trong 1 list item */
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    /* Tìm vị trí trong danh sách (item), trong mảng qset (s_pos) và trong quantum (q_pos) */
    item = (long)*f_pos / item_size;
    rest = (long)*f_pos % item_size;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    /* Di chuyển tới node chứa dữ liệu */
    dptr = mychardev_follow(dev, item);

    if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
        goto out; /* Lỗ hổng trong file (sparse file) -> đọc ra 0 nhưng ở đây ta dừng */

    /* Chỉ đọc tối đa đến hết quantum hiện tại */
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

    pr_info("mychardev: Read called. Request: %zu bytes, Offset: %lld\n", count, *f_pos);
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

    /* --- SỬA ĐỔI QUAN TRỌNG --- */
    /* Xử lý O_APPEND: Nếu mở chế độ nối đuôi (>>), dời con trỏ về cuối file */
    if (filp->f_flags & O_APPEND) {
        *f_pos = dev->size;
    }

    /* Tính toán vị trí cần ghi */
    item = (long)*f_pos / item_size;
    rest = (long)*f_pos % item_size;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    /* Tìm node list tương ứng */
    dptr = mychardev_follow(dev, item);
    if (dptr == NULL)
        goto out;

    /* Cấp phát mảng con trỏ */
    if (!dptr->data) {
        dptr->data = kzalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
    }

    /* Cấp phát quantum */
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

    /* Cập nhật kích thước file */
    if (dev->size < *f_pos)
        dev->size = *f_pos;

    pr_info("mychardev: Write called. Request: %zu bytes, Offset: %lld\n", count, *f_pos);

out:
    mutex_unlock(&dev->lock);
    return retval;
}
/* Các thao tác file */
static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = mychardev_open,
    .release = mychardev_release,
    .read = my_read,
    .write = my_write,
};

/* ================================================================
 * INIT & EXIT
 * ================================================================ */

static int __init mychardev_init(void)
{
    int ret;

    /* Khởi tạo các giá trị mặc định cho cấu trúc Scull */
    mdev.quantum = SCULL_QUANTUM;
    mdev.qset = SCULL_QSET;
    mdev.size = 0;
    mdev.data = NULL;
    mutex_init(&mdev.lock); // Khởi tạo Mutex

    /* 1. Cấp phát số hiệu thiết bị */
    ret = alloc_chrdev_region(&mdev.devno, 0, 1, "mychardev");
    if (ret) {
        pr_err("Failed to allocate device number\n");
        return ret;
    }

    /* 2. Khởi tạo cdev */
    cdev_init(&mdev.my_cdev, &my_fops);
    mdev.my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&mdev.my_cdev, mdev.devno, 1);
    if (ret) {
        pr_err("Failed to add cdev\n");
        goto unreg_chrdev;
    }

    /* 3. Tạo Class và Device Node */
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
    /* Giải phóng bộ nhớ động Scull trước khi thoát */
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
MODULE_DESCRIPTION("mychardev dynamic memory scull model");
MODULE_AUTHOR("Doan Duc Manh - UET, VNU");