/* provider.c - cung cấp compute_stats() và export symbol */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include "stats.h"

int compute_stats(const int *arr, size_t len, struct stats_result *out)
{
    size_t i;
    long long sum = 0;
    int cur_max, cur_min;

    if (!arr || !out)
        return -EINVAL;
    if (len == 0)
        return -EINVAL;

    cur_max = arr[0];
    cur_min = arr[0];
    sum = arr[0];

    for (i = 1; i < len; i++) {
        int v = arr[i];
        sum += v;
        if (v > cur_max)
            cur_max = v;
        if (v < cur_min)
            cur_min = v;
    }

    out->sum = sum;
    out->avg = (long)(sum / (long long)len); /* integer average */
    out->max = cur_max;
    out->min = cur_min;

    return 0;
}
EXPORT_SYMBOL(compute_stats);

static int __init provider_init(void)
{
    pr_info("provider: loaded\n");
    return 0;
}

static void __exit provider_exit(void)
{
    pr_info("provider: unloaded\n");
}

module_init(provider_init);
module_exit(provider_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry");
MODULE_DESCRIPTION("Provider module exporting compute_stats");
 