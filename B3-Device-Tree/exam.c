#include<linux/module.h>
#include<linux/of.h>
#include<linux/platform_device.h>

#define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
#define DRIVER_DESC "device tree example"
#define DRIVER_VERS "1.0"

static const struct of_device_id dt_ids[]={
    {.compatible="dts_example",},
    { }
};

static int my_pdrv_probe(struct platform_device *pdev)
{
    const char *my_string = NULL;
    pr_info("%s--%d\n",__func__,__LINE__);
    of_property_read_string(pdev->dev.of_node, "pt", &my_string);
    pr_info("My string:%s\n",my_string);
    return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    pr_info("%s--%d\n",__func__,__LINE__);
    return 0;
}

/*platform driver*/
static struct platform_driver mypdrv = {
    .probe=my_pdrv_probe,
    .remove=my_pdrv_remove,
    .driver={
        .name="dts_example",
        .of_match_table=of_match_ptr(dt_ids),
        .owner=THIS_MODULE,
    },
};

module_platform_driver(mypdrv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERS);
