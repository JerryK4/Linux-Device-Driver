#include <linux/module.h>           /* Defines functions such as module_init/module_exit */
#include <linux/kernel.h>
#include <linux/gpio.h>             /* Defines functions such as gpio_request/gpio_free */
#include <linux/platform_device.h>  /* For platform devices */
#include <linux/gpio/consumer.h>    /* For GPIO Descriptor */
#include <linux/of.h>               /* For DT */  
#include <linux/pwm.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include <linux/uaccess.h>
#define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
#define DRIVER_DESC   "gpio subsystem"
#define DEVICE_NUMBER_NAME "m_device_number"
#define CLASS_NAME "m_class"
#define DEVICE_NAME "m_device"
#define MAX_DUTY_CYCLE 100

#define HIGH 1
#define LOW 0
struct m_foo_device{
    int size;
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;
}mdev;

/*  Function Prototypes */
static int      m_open(struct inode *inode, struct file *file);
static int      m_release(struct inode *inode, struct file *file);
static ssize_t  m_read(struct file *filp, char __user *user_buf, size_t size,loff_t * offset);
static ssize_t  m_write(struct file *filp, const char *user_buf, size_t size, loff_t * offset);

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = m_read,
    .write      = m_write,
    .open       = m_open,
    .release    = m_release,
};

static struct gpio_pwm_dev {
    struct gpio_desc *gpio1_18;
    struct pinctrl *pinctrl;
    struct pinctrl_state *pin_gpio;
    struct pinctrl_state *pin_pwm;
    struct pwm_device *pwm;

}gpio_pwm;

/* This function will be called when we open the Device file */
static int m_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "System call open() called...!!!\n");
    return 0;
}

/* This function will be called when we close the Device file */
static int m_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "System call close() called...!!!\n");
    return 0;
}

/* This function will be called when we read the Device file */
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
    printk(KERN_INFO "System call read() called...!!!\n");
    return 0;
}

/* This function will be called when we write the Device file */
static ssize_t m_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset)
{
    char kernel_buf[16];
    long mode,value;
    int ret;
    unsigned long period;
    unsigned long duty_cycle;

    if(size>=sizeof(kernel_buf))
        return -EINVAL;

    if(copy_from_user(kernel_buf,user_buf,size))
        return -EINVAL;

    kernel_buf[size]='\0';

    ret=sscanf(kernel_buf,"%ld %ld",&mode,&value);
    if(ret!=2)
        return -EINVAL;

    if(mode==1){
        /*Disable pwm*/
         pwm_disable(gpio_pwm.pwm);

        //Select GPIO state
        pinctrl_select_state(gpio_pwm.pinctrl, gpio_pwm.pin_gpio);
        if(value==1){
            gpiod_set_value(gpio_pwm.gpio1_18,HIGH);
        }else if(value==0){
            gpiod_set_value(gpio_pwm.gpio1_18,LOW);
        }else{
            return -EINVAL;
        }
    }else if(mode==0){
        //Set GPIO -> LOW
        gpiod_set_value(gpio_pwm.gpio1_18,LOW);
        //Select state PWM
        pinctrl_select_state(gpio_pwm.pinctrl,gpio_pwm.pin_pwm);
        period= 5000000;
        printk(KERN_INFO "PWM period: %lu\n", period);
        if(value>=0 &&value<=MAX_DUTY_CYCLE){
            duty_cycle=value*period/MAX_DUTY_CYCLE;
            printk(KERN_INFO "Duty_cycle = %lu\n", duty_cycle);
            pwm_config(gpio_pwm.pwm,duty_cycle,period);
            pwm_enable(gpio_pwm.pwm);
        }else{
            return -EINVAL;
        }
    }else{
        return -EINVAL;
    }
    printk(KERN_INFO "System call write() called...!!!\n");
    return size;
}


static const struct of_device_id gpiod_dt_ids[]={
    {.compatible="gpio-descriptor-based",},
    { }
};

static int my_pdrv_probe(struct platform_device *pdev)
{
    struct device* dev=&pdev->dev;
    /*1.0-Dynamic allocating device number(cat /proc/devices)*/
    if(alloc_chrdev_region(&mdev.dev_num,0,1,DEVICE_NUMBER_NAME)<0){
        pr_err("Failed to allocate device number!\n");
        return -1;
    };

    printk(KERN_INFO "Major:%d, Minor:%d\n",MAJOR(mdev.dev_num),MINOR(mdev.dev_num));

    /*2.0-Create struct class*/
    if((mdev.m_class=class_create(THIS_MODULE,CLASS_NAME))==NULL){
        pr_err("Failed create struct class!\n");
        goto rm_device_numb;
    };

    /*3.0-Create device*/
    if(device_create(mdev.m_class,NULL,mdev.dev_num,NULL,DEVICE_NAME)==NULL){
        pr_err("Failed to create my device!\n");
        goto rm_class;
    };


    /*4.0-Create cdev structure*/
    cdev_init(&mdev.m_cdev,&fops);
    /*4.1-Adding character device to the system*/
    if((cdev_add(&mdev.m_cdev,mdev.dev_num,1))<0){
        pr_err("Can't add the device to the system\n");
        goto rm_device;
    };

    gpio_pwm.gpio1_18=gpiod_get(dev,"led",GPIOD_OUT_LOW);

    gpio_pwm.pinctrl=pinctrl_get(dev);
    if(IS_ERR(gpio_pwm.pinctrl))
        return PTR_ERR(gpio_pwm.pinctrl);

    gpio_pwm.pin_pwm=pinctrl_lookup_state(gpio_pwm.pinctrl, "default");
    if (IS_ERR(gpio_pwm.pin_pwm)) {
        devm_pinctrl_put(gpio_pwm.pinctrl);
        return PTR_ERR(gpio_pwm.pin_pwm);
    }

    gpio_pwm.pin_gpio=pinctrl_lookup_state(gpio_pwm.pinctrl, "sleep");
    if (IS_ERR(gpio_pwm.pin_gpio)) {
        devm_pinctrl_put(gpio_pwm.pinctrl);
        return PTR_ERR(gpio_pwm.pin_gpio);
    }

    gpio_pwm.pwm= pwm_get(dev,NULL);
    if (IS_ERR(gpio_pwm.pwm))
        return PTR_ERR(gpio_pwm.pwm);

    printk(KERN_INFO "Hello world Jerry\n");
    return 0;

rm_device:  
    device_destroy(mdev.m_class,mdev.dev_num);
rm_class:
    class_destroy(mdev.m_class);
rm_device_numb:
    unregister_chrdev_region(mdev.dev_num,1);
    return -1;
};

static int my_pdrv_remove(struct platform_device *pdev)
{
    cdev_del(&mdev.m_cdev);                     /*4.0*/
    device_destroy(mdev.m_class,mdev.dev_num);  /*3.0*/
    class_destroy(mdev.m_class);                /*2.0*/
    unregister_chrdev_region(mdev.dev_num,1);   /*1.0*/
    pwm_disable(gpio_pwm.pwm);
    printk(KERN_INFO "Goodbye Jerry\n");
    return 0;
};

/*platform_driver*/
static struct platform_driver mypdrv={
    .probe=my_pdrv_probe,
    .remove=my_pdrv_remove,
    .driver={
        .name="descriptor-based",
        .of_match_table=of_match_ptr(gpiod_dt_ids),
        .owner=THIS_MODULE,
    },
};

module_platform_driver(mypdrv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("1.0");

// #include <linux/module.h>           /* Defines functions such as module_init/module_exit */
// #include <linux/gpio.h>             /* Defines functions such as gpio_request/gpio_free */
// #include <linux/platform_device.h>  /* For platform devices */
// #include <linux/gpio/consumer.h>    /* For GPIO Descriptor */
// #include <linux/of.h>               /* For DT */  
// #include <linux/pwm.h>
// #include <linux/fs.h>
// #include <linux/device.h>
// #include <linux/cdev.h>
// #include <linux/slab.h>
// #include <linux/uaccess.h>
// #include <linux/kernel.h>

// #define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
// #define DRIVER_DESC   "gpio subsystem"
// #define DEVICE_NUMBER_NAME "m_device_number"
// #define CLASS_NAME "m_class"
// #define DEVICE_NAME "m_device"
// #define MAX_DUTY_CYCLE 100

// #define HIGH 1
// #define LOW 0

// struct m_foo_device {
//     int size;
//     dev_t dev_num;
//     struct class *m_class;
//     struct cdev m_cdev;
// } mdev;

// /*  Function Prototypes */
// static int m_open(struct inode *inode, struct file *file);
// static int m_release(struct inode *inode, struct file *file);
// static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset);
// static ssize_t m_write(struct file *filp, const char *user_buf, size_t size, loff_t *offset);

// static struct file_operations fops = {
//     .owner = THIS_MODULE,
//     .read = m_read,
//     .write = m_write,
//     .open = m_open,
//     .release = m_release,
// };

// static struct gpio_pwm_dev {
//     struct gpio_desc *gpio1_18;
//     struct pinctrl *pinctrl;
//     struct pinctrl_state *pin_gpio;
//     struct pinctrl_state *pin_pwm;
//     struct pwm_device *pwm;
// } gpio_pwm;

// /* This function will be called when we open the Device file */
// static int m_open(struct inode *inode, struct file *file)
// {
//     printk(KERN_INFO "System call open() called...!!!\n");
//     return 0;
// }

// /* This function will be called when we close the Device file */
// static int m_release(struct inode *inode, struct file *file)
// {
//     printk(KERN_INFO "System call close() called...!!!\n");
//     return 0;
// }

// /* This function will be called when we read the Device file */
// static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
// {
//     printk(KERN_INFO "System call read() called...!!!\n");
//     return 0;
// }

// /* This function will be called when we write the Device file */
// static ssize_t m_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset)
// {
//     char kernel_buf[16];
//     long mode, value;
//     int ret;
//     unsigned long period;
//     unsigned long duty_cycle;

//     if (size >= sizeof(kernel_buf))
//         return -EINVAL;

//     if (copy_from_user(kernel_buf, user_buf, size))
//         return -EINVAL;

//     kernel_buf[size] = '\0';

//     ret = sscanf(kernel_buf, "%ld %ld", &mode, &value);
//     if (ret != 2)
//         return -EINVAL;

//     if (mode == 1) {
//         /* Disable pwm */
//         pwm_disable(gpio_pwm.pwm);

//         /* Select GPIO state */
//         pinctrl_select_state(gpio_pwm.pinctrl, gpio_pwm.pin_gpio);
//         if (value == 1) {
//             gpiod_set_value(gpio_pwm.gpio1_18, HIGH);
//         } else if (value == 0) {
//             gpiod_set_value(gpio_pwm.gpio1_18, LOW);
//         } else {
//             return -EINVAL;
//         }
//     } else if (mode == 0) {
//         /* Set GPIO -> LOW */
//         gpiod_set_value(gpio_pwm.gpio1_18, LOW);

//         /* Select state PWM */
//         pinctrl_select_state(gpio_pwm.pinctrl, gpio_pwm.pin_pwm);
//         period = 5000000;
//         printk(KERN_INFO "PWM period: %lu\n", period);
//         if (value >= 0 && value <= MAX_DUTY_CYCLE) {
//             duty_cycle = value * period / MAX_DUTY_CYCLE;
//             printk(KERN_INFO "Duty_cycle = %lu\n", duty_cycle);
//             pwm_config(gpio_pwm.pwm, duty_cycle, period);
//             pwm_enable(gpio_pwm.pwm);
//         } else {
//             return -EINVAL;
//         }
//     } else {
//         return -EINVAL;
//     }
//     printk(KERN_INFO "System call write() called...!!!\n");
//     return size;
// }

// static const struct of_device_id gpiod_dt_ids[] = {
//     { .compatible = "gpio-descriptor-based", },
//     { }
// };

// static int my_pdrv_probe(struct platform_device *pdev)
// {
//     struct device* dev = &pdev->dev;

//     /* Dynamic allocating device number */
//     if (alloc_chrdev_region(&mdev.dev_num, 0, 1, DEVICE_NUMBER_NAME) < 0) {
//         pr_err("Failed to allocate device number!\n");
//         return -1;
//     }

//     printk(KERN_INFO "Major:%d, Minor:%d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

//     /* Create struct class */
//     if ((mdev.m_class = class_create(THIS_MODULE, CLASS_NAME)) == NULL) {
//         pr_err("Failed create struct class!\n");
//         goto rm_device_numb;
//     }

//     /* Create device */
//     if (device_create(mdev.m_class, NULL, mdev.dev_num, NULL, DEVICE_NAME) == NULL) {
//         pr_err("Failed to create my device!\n");
//         goto rm_class;
//     }

//     /* Create cdev structure */
//     cdev_init(&mdev.m_cdev, &fops);

//     /* Adding character device to the system */
//     if ((cdev_add(&mdev.m_cdev, mdev.dev_num, 1)) < 0) {
//         pr_err("Can't add the device to the system\n");
//         goto rm_device;
//     }

//     gpio_pwm.gpio1_18 = gpiod_get(dev, "led", GPIOD_OUT_LOW);
//     if (IS_ERR(gpio_pwm.gpio1_18)) {
//         pr_err("Failed to get GPIO descriptor!\n");
//         goto rm_cdev;
//     }

//     gpio_pwm.pinctrl = pinctrl_get(dev);
//     if (IS_ERR(gpio_pwm.pinctrl)) {
//         pr_err("Failed to get pinctrl handle!\n");
//         goto rm_gpio;
//     }

//     gpio_pwm.pin_pwm = pinctrl_lookup_state(gpio_pwm.pinctrl, "default");
//     if (IS_ERR(gpio_pwm.pin_pwm)) {
//         pr_err("Failed to lookup PWM state!\n");
//         goto rm_pinctrl;
//     }

//     gpio_pwm.pin_gpio = pinctrl_lookup_state(gpio_pwm.pinctrl, "sleep");
//     if (IS_ERR(gpio_pwm.pin_gpio)) {
//         pr_err("Failed to lookup GPIO state!\n");
//         goto rm_pinctrl;
//     }

//     gpio_pwm.pwm = pwm_get(dev, NULL);
//     if (IS_ERR(gpio_pwm.pwm)) {
//         pr_err("Failed to get PWM device!\n");
//         goto rm_pinctrl;
//     }

//     printk(KERN_INFO "Hello world Jerry\n");
//     return 0;

// rm_pinctrl:
//     devm_pinctrl_put(gpio_pwm.pinctrl);
// rm_gpio:
//     gpiod_put(gpio_pwm.gpio1_18);
// rm_cdev:
//     cdev_del(&mdev.m_cdev);
// rm_device:
//     device_destroy(mdev.m_class, mdev.dev_num);
// rm_class:
//     class_destroy(mdev.m_class);
// rm_device_numb:
//     unregister_chrdev_region(mdev.dev_num, 1);
//     return -1;
// }

// static int my_pdrv_remove(struct platform_device *pdev)
// {
//     cdev_del(&mdev.m_cdev);
//     device_destroy(mdev.m_class, mdev.dev_num);
//     class_destroy(mdev.m_class);
//     unregister_chrdev_region(mdev.dev_num, 1);
    
//     if (!IS_ERR(gpio_pwm.gpio1_18)) {
//         gpiod_put(gpio_pwm.gpio1_18);
//     }
//     if (!IS_ERR(gpio_pwm.pinctrl)) {
//         devm_pinctrl_put(gpio_pwm.pinctrl);
//     }
//     if (!IS_ERR(gpio_pwm.pwm)) {
//         pwm_put(gpio_pwm.pwm);
//     }
    
//     pwm_disable(gpio_pwm.pwm);
//     printk(KERN_INFO "Goodbye Jerry\n");
//     return 0;
// }

// /* platform_driver */
// static struct platform_driver mypdrv = {
//     .probe = my_pdrv_probe,
//     .remove = my_pdrv_remove,
//     .driver = {
//         .name = "descriptor-based",
//         .of_match_table = of_match_ptr(gpiod_dt_ids),
//         .owner = THIS_MODULE,
//     },
// };

// static int __init chrdev_init(void)
// {
//     return platform_driver_register(&mypdrv);
// }

// static void __exit chrdev_exit(void)
// {
//     platform_driver_unregister(&mypdrv);
// }

// module_init(chrdev_init);
// module_exit(chrdev_exit);

// MODULE_LICENSE("GPL");
// MODULE_AUTHOR(DRIVER_AUTHOR);
// MODULE_DESCRIPTION(DRIVER_DESC);
// MODULE_VERSION("1.0");
