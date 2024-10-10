#include<linux/module.h>
#include<linux/gpio.h>


#define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
#define DRIVER_DESC "gpio-integer-based"
#define DRIVER_VERS "1.0"

#define GPIO1_16 48

#define LOW 0
#define HIGH 1


static int __init gpio_init(void)
{
    gpio_request(GPIO1_16, "gpio1_16");
    gpio_direction_output(GPIO1_16, LOW);
    gpio_set_value(GPIO1_16, HIGH);

    pr_info("Hello GPIO-integer-based!\n");
    pr_info("GPIO status:%d!\n",gpio_get_value(GPIO1_16));
    return 0;
}

static void __exit gpio_exit(void)
{
    gpio_set_value(GPIO1_16, LOW);
    gpio_free(GPIO1_16);
    pr_info("Goodbye!\n");

}

module_init(gpio_init);
module_exit(gpio_exit);




MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);  
MODULE_VERSION(DRIVER_VERS);