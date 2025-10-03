#include <linux/kernel.h>
#include <linux/module.h>

void helper_greet(void)
{
    printk(KERN_INFO "hello_multi: Greetings from helper_greet()!\n");
}
