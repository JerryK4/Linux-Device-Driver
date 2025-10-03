#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int nums[10];
static int nums_count;
module_param_array(nums, int, &nums_count, 0444);
MODULE_PARM_DESC(nums, "Array of up to 10 integers");

static int __init hello_init(void) {
    int i;
    int sum = 0;
    int count = nums_count <= 10 ? nums_count : 10;
    for(i=0; i<count; i++) {
        printk(KERN_INFO "nums[%d] = %d\n", i, nums[i]);
        sum += nums[i];
    }
    printk(KERN_INFO "Sum = %d\n",sum);
    return 0;
}
static void __exit hello_exit(void) {
 printk(KERN_INFO "hello_array: unloaded\n");
}
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry");
MODULE_DESCRIPTION("Excersise 4 - Hello Array");
