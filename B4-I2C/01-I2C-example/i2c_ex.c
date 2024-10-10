#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/i2c.h>
#define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
#define DRIVER_DESC   "i2c example"

static struct of_device_id i2c_driver_of_id[]={
    {.compatible="i2c_ex_compatible",},
    { }
};

static int i2c_driver_probe_new(struct i2c_client *client)
{
    pr_info("Jerry : %s--%d\n",__func__,__LINE__);
    pr_info("i2c_ex reg: %X\n",client->addr);
    return 0;
}

static int i2c_driver_remove(struct i2c_client *client)
{
    pr_info("Jerry Goodbye!\n");
    return 0;
}

MODULE_DEVICE_TABLE(of,i2c_driver_of_id);

static struct i2c_driver i2c_driver={
    .probe_new=i2c_driver_probe_new,
    .remove=i2c_driver_remove,
    .driver={
        .name="i2c_example",
        .of_match_table=i2c_driver_of_id,
        .owner=THIS_MODULE,
    },
};

module_i2c_driver(i2c_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("1.0");