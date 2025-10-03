/* stats.h - shared header for provider & consumer */
#ifndef _STATS_H_
#define _STATS_H_

#include <linux/types.h>

struct stats_result {
    long long sum; /* tổng (có thể lớn) */
    long avg;      /* trung bình (integer) */
    int max;       /* giá trị cực đại */
    int min;       /* giá trị cực tiểu */
};

#endif /* _STATS_H_ */
