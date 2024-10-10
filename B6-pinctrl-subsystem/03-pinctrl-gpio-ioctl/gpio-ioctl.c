#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/gpio.h>
#include<linux/platform_device.h>
#include<linux/gpio/consumer.h>
#include<linux/of.h>
#include<linux/pwm.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/err.h>
#include <linux/kdev_t.h>

#define DEVICE_NUMBER_NAME "m_device_number"
#define CLASS_NAME "m_class"
#define DEVICE_NAME "m_device"

#define MAX_DUTY_CYCLE 100

#define HIGH 1
#define LOW 0 


#define PWM_STATE _IOW('a','1',int32_t*)
#define GPIO_STATE _IOW('a','0',int32_t*)


struct m_foo_device{
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;
}mdev;

/*  Function Prototypes */
static int      m_open(struct inode *inode, struct file *file);
static int      m_release(struct inode *inode, struct file *file);
static ssize_t  m_read(struct file *filp, char __user *user_buf, size_t size,loff_t * offset);
static ssize_t  m_write(struct file *filp, const char *user_buf, size_t size, loff_t * offset);
static long      m_ioctl(struct file *file,unsigned int cmd,unsigned long arg);
static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = m_read,
    .write      = m_write,
    .open       = m_open,
    .release    = m_release,
    .unlocked_ioctl=m_ioctl,
};

static struct gpio_pwm_dev{
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
    pr_info("System call write() called...!!!\n");
    return size;
}

static long m_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
    long period,duty_cycle;
    int32_t value=0;
    switch(cmd){
        case PWM_STATE:
            //Set GPIO->LOW
            gpiod_set_value(gpio_pwm.gpio1_18,LOW);
            //Select state PWM 
            pinctrl_select_state(gpio_pwm.pinctrl,gpio_pwm.pin_pwm);
            period=5000000;
            printk(KERN_INFO "PWM period : %lu\n",period);
            if(copy_from_user(&value,(int32_t*)arg,sizeof(value)))
            {
                pr_err("Data write:Err!\n");
                return -EFAULT;
            }
            printk(KERN_INFO "PWM value : %d\n",value);
            if(value>=0&&value<=MAX_DUTY_CYCLE){
                duty_cycle=value*period/MAX_DUTY_CYCLE;
                printk(KERN_INFO "Duty_cycle = %lu\n", duty_cycle);
                pwm_config(gpio_pwm.pwm,duty_cycle,period);
                pwm_enable(gpio_pwm.pwm);
            }
            break;
        case GPIO_STATE:
            /*Disable pwm_state*/
            pwm_disable(gpio_pwm.pwm);
            /*Select gpio_state*/
            pinctrl_select_state(gpio_pwm.pinctrl,gpio_pwm.pin_gpio);
            if(copy_from_user(&value,(int32_t*)arg,sizeof(value)))
            {
                pr_err("Data write:Err!\n");
                return -EFAULT;
            }
            printk(KERN_INFO "GPIO value : %d\n",value);
            if(value==1)
            {
                gpiod_set_value(gpio_pwm.gpio1_18,HIGH);
            }else if (value==0)
            {
                gpiod_set_value(gpio_pwm.gpio1_18,LOW);
            }
            break;
        default:
            pr_info("Default\n");
            return -EINVAL;
    }
    return 0;
}

static const struct of_device_id gpiod_dt_ids[]={
    {.compatible="gpio-descriptor-based",},
    { }
};

static int my_pdrv_probe(struct platform_device *pdev)
{
    struct device* dev=&pdev->dev;
    /*1.0-Dynamic allocating device number*/
    if(alloc_chrdev_region(&mdev.dev_num,0,1,DEVICE_NUMBER_NAME)<0)
    {
        pr_err("Failed to allocate device number!\n");
        return -1;
    }
    pr_info("Major:%d , Minor:%d\n",MAJOR(mdev.dev_num),MINOR(mdev.dev_num));
    /*2.0-Create struct class*/
    if((mdev.m_class=class_create(THIS_MODULE,CLASS_NAME))==NULL)
    {
        pr_err("Failed to create struct class!\n");
        goto rm_device_number;
    }
    /*3.0-Create device*/
    if(device_create(mdev.m_class,NULL,mdev.dev_num,NULL,DEVICE_NAME)==NULL)
    {
        pr_err("Failed to create my device!\n");
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

    gpio_pwm.gpio1_18=gpiod_get(dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(gpio_pwm.gpio1_18)) {
        pr_err("Failed to get GPIO\n");
        goto rm_cdev;
    }
    gpio_pwm.pinctrl=pinctrl_get(dev);
    if (IS_ERR(gpio_pwm.pinctrl)) {
        pr_err("Failed to get pinctrl\n");
        goto rm_gpio;
    }
    gpio_pwm.pin_gpio= pinctrl_lookup_state(gpio_pwm.pinctrl, "sleep");
    if (IS_ERR(gpio_pwm.pin_gpio)) {
        pr_err("Failed to lookup pin GPIO\n");
        goto rm_pinctrl;
    }
    gpio_pwm.pin_pwm= pinctrl_lookup_state(gpio_pwm.pinctrl, "default");
     if (IS_ERR(gpio_pwm.pin_pwm)) {
        pr_err("Failed to lookup pin PWM\n");
        goto rm_pinctrl;
    }
    gpio_pwm.pwm=pwm_get(dev,NULL);
     if (IS_ERR(gpio_pwm.pwm)) {
        pr_err("Failed to get PWM\n");
        goto rm_pinctrl;
    }
    pr_info("Hello Jerry!\n");
    return 0;

rm_pinctrl:
    pinctrl_put(gpio_pwm.pinctrl);
rm_gpio:
    gpiod_put(gpio_pwm.gpio1_18);
rm_cdev:
    cdev_del(&mdev.m_cdev);
rm_device:  
    device_destroy(mdev.m_class,mdev.dev_num);
rm_class:
    class_destroy(mdev.m_class);
rm_device_number:
    unregister_chrdev_region(mdev.dev_num,1);
    return -1;
}

static int my_pdrv_remove(struct platform_device *pdev)
{   
    pwm_disable(gpio_pwm.pwm);
    pwm_put(gpio_pwm.pwm);
    pinctrl_put(gpio_pwm.pinctrl);
    gpiod_put(gpio_pwm.gpio1_18);
    cdev_del(&mdev.m_cdev);
    device_destroy(mdev.m_class,mdev.dev_num);
    class_destroy(mdev.m_class);
    unregister_chrdev_region(mdev.dev_num,1);
    pr_info("Goodbye Jerry!\n");
    return 0;
}

/*Platform driver*/
static struct platform_driver mypdrv={
    .probe=my_pdrv_probe,
    .remove=my_pdrv_remove,
    .driver={
        .name="gpio-descriptor-based",
        .of_match_table=of_match_ptr(gpiod_dt_ids),
        .owner=THIS_MODULE,
    },
};
module_platform_driver(mypdrv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry <JerryFromUET@gmail.com>");
MODULE_DESCRIPTION("Linux device driver (IOCTL)-GPIO");
MODULE_VERSION("1.0");

