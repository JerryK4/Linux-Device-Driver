/* consumer.c - dùng compute_stats() từ provider, input bằng module_param_array */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include "stats.h"

#define MAX_NUMS 10 

/* array parameter */
static int nums[MAX_NUMS];
static int nums_count = 0;
module_param_array(nums, int, &nums_count, 0444);
MODULE_PARM_DESC(nums, "Array of integers (max 10) to compute stats on");

extern int compute_stats(const int *arr, size_t len, struct stats_result *out);

static int __init consumer_init(void)
{
    struct stats_result res;
    int ret;
    int count = nums_count;

    if (count == 0) {
        printk(KERN_ERR "consumer: no input nums provided (use nums=... when insmod)\n");
        return -EINVAL;
    }
    if (count > MAX_NUMS) {
        printk(KERN_WARNING "consumer: nums_count (%d) > MAX_NUMS (%d), truncating\n",
               count, MAX_NUMS);
        count = MAX_NUMS;
    }

    printk(KERN_INFO "consumer: loaded - calling compute_stats() on %d element(s)\n",
           count);

    ret = compute_stats(nums, (size_t)count, &res);
    if (ret) {
        printk(KERN_ERR "consumer: compute_stats() failed: %d\n", ret);
        return ret;
    }

    printk(KERN_INFO "consumer: stats -> sum=%lld avg=%ld max=%d min=%d\n",
           res.sum, res.avg, res.max, res.min);

    return 0;
}

static void __exit consumer_exit(void)
{
    printk(KERN_INFO "consumer: unloaded\n");
}

module_init(consumer_init);
module_exit(consumer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry");
MODULE_DESCRIPTION("Consumer module using compute_stats from provider with module_param_array input");
