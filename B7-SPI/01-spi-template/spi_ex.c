#include<linux/module.h>
#include<linux/spi/spi.h>
#include<linux/init.h>

#define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
#define DRIVER_DESC   "spi_example"

static struct of_device_id spi_of_match[]={
    {.compatible="nokia5110",},
    { }
};

static int my_probe(struct spi_device *spi)
{
    pr_info("Jerry : %s -- %d\n",__func__,__LINE__);
    return 0;
}
static int  my_remove(struct spi_device *spi)
{
    pr_info("Jerry : %s -- %d\n",__func__,__LINE__);
    return 0;
}

MODULE_DEVICE_TABLE(of, spi_of_match);

static struct spi_driver my_spi_driver={
    .probe=my_probe,
    .remove=my_remove,
    .driver={
        .name="nokia5110",
        .owner=THIS_MODULE,
        .of_match_table=spi_of_match,
    },
};

static int __init spi_init(void)
{
    return spi_register_driver(&my_spi_driver);
}

static void __exit spi_cleanup(void)
{
    spi_unregister_driver(&my_spi_driver);
}

module_init(spi_init);
module_exit(spi_cleanup);

//OR  module_spi_driver(my_spi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("1.0");
