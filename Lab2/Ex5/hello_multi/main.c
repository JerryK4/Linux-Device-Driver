#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

/* khai báo prototype của hàm helper_greet ở helper.c */
void helper_greet(void);

static int __init hello_multi_init(void)
{
    printk(KERN_INFO "hello_multi: module loaded\n");

    /* gọi hàm trong helper.c */
    helper_greet();

    return 0;
}

static void __exit hello_multi_exit(void)
{
    printk(KERN_INFO "hello_multi: module unloaded\n");
}

module_init(hello_multi_init);
module_exit(hello_multi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry");
MODULE_DESCRIPTION("Exercise 5 - Hello Multi-file Module");
