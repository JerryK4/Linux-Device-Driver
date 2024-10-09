#include <linux/module.h>   /* This module defines functions such as module_init/module_exit */
#include <linux/io.h>       /* This module defines functions such as ioremap/iounmap */
#include "gpio.h"           /* Gpio modules */


#define DRIVER_AUTHOR "jerry jerryfromUET@gmail.com"
#define DRIVER_DESC "gpio-driver-legacy"
#define DRIVER_VERS "1.0"

uint32_t __iomem *gpio1_base_addr;

/*Constructor*/
static int __init gpio_init(void)
{
    //Ánh xạ vùng nhớ I/O vật lý (I/O buffer) của thiết bị phần cứng vào không gian địa chỉ ảo của kernel (kernel buffer - kernel virtual address space)
    gpio1_base_addr=ioremap(GPIO_ADDR_BASE,GPIO_ADDR_SIZE);
    if(!gpio1_base_addr)
        return -ENOMEM;

    /*Set gpio1_16 as output mode and turn on*/
    *(gpio1_base_addr + GPIO_OE_OFFSET/4)&=~GPIO1_16;
    *(gpio1_base_addr+ GPIO_SETDATAOUT_OFFSET/4)|=GPIO1_16;

    pr_info("Hello! Initizliaze successfully!\n");
    return 0;
}

static void __exit gpio_exit(void)
{
    *(gpio1_base_addr+ GPIO_CLEARDATAOUT_OFFSET/4)|=GPIO1_16;
    iounmap(gpio1_base_addr);//Giải phóng vùng nhứ được ánh xạ

    pr_info("Good bye my fen !!!\n");
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);  
MODULE_VERSION(DRIVER_VERS);