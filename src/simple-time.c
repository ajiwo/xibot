#include "simple-time.h"

#ifndef _XOPEN_SOURCE
#define is_leap(A) !((A) & 3)
time_t str_to_epoch(const char *str, const char *fmt) {

    long s_epoch;
    int day_count;
    int i;
    char c;
    int year, month, day, hour, min, second;
    ((void) fmt);

    /* the character at index of fmt expected to be a number/digit */
    const int digits[] = {0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18};
    /* day count each month */
    const int days_num[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* check format */
    for(i = 0; i < 14; i++) {
        c = str[digits[i]];
        if(!(c >= '0' && c <= '9'))
            break;
    }
    /* invalid format */
    if(i != 14)
        return 0;


    year = (str[0] - '0') * 1000 + (str[1] - '0') * 100 + (str[2] - '0') * 10 + (str[3] - '0');
    month = (str[5] - '0') * 10 + str[6] - '0';
    day = (str[8] - '0') * 10 + str[9] - '0';
    hour = (str[11] - '0') * 10 + str[12] - '0';
    min = (str[14] - '0') * 10 + str[15] - '0';
    second = (str[17] - '0') * 10 + str[18] - '0';

    s_epoch = 0;
    /* seconds until last year */
    day_count = 0;
    for(i = 1970; i < year; i++)
        day_count += is_leap(i) ? 366 : 365;

    s_epoch += day_count * 86400L;

    /* seconds until last month*/
    day_count = 0;
    if(month - 1 > 0) {
        for(i = 0; i < month - 1; i++) {
            day_count += (is_leap(year) && i == 1) ? days_num[i] + 1 : days_num[i];
        }

        s_epoch += day_count * 86400L;
    }

    /* seconds until yesterday */
    s_epoch += (day - 1) * 86400L;

    s_epoch += second + min * 60 + hour * 3600;

    return s_epoch;
}
#else
#include <string.h>
time_t str_to_epoch(const char *str, const char *fmt) {
    struct tm stm;
    time_t ret;

    memset(&stm, '\0', sizeof(struct tm));
    strptime(str, fmt, &stm);
    ret = mktime(&stm);
#ifdef	__USE_BSD
    ret += stm.tm_gmtoff;
#else
    ret += stm.__tm_gmtoff;
#endif
    return ret;
}
#endif

