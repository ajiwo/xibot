#ifndef SIMPLE_TIME_H
#define SIMPLE_TIME_H
#define _XOPEN_SOURCE
#include <time.h>

time_t str_to_epoch(const char *str, const char *fmt);
#endif /* SIMPLE_TIME_H */

