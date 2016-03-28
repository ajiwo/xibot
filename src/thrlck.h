#ifndef THRLCK_H
#define THRLCK_H
#include <pthread.h>

typedef struct _thrlck thrlck_t;
struct _thrlck {
    pthread_mutex_t lock;
    pthread_cond_t rcond;
    pthread_cond_t wcond;
    int lock_count;     /* = 0 unlocked, < 0 lockd by writer, > 0 lockd by reader */
    int waiting_writers;
    void *data;         /* lock protected data, don't access this directly */
    int _type;
};


void thrlck_init(thrlck_t *tl);

void thrlck_lock_read(thrlck_t *tl);
void thrlck_unlock_read(thrlck_t *tl);

void thrlck_lock_write(thrlck_t *tl);
void thrlck_unlock_write(thrlck_t *tl);

void thrlck_set_data(thrlck_t *tl, void *val);
void *thrlck_get_data(thrlck_t *tl);

void thrlck_set_type(thrlck_t *tl, int type);
int thrlck_get_type(thrlck_t *tl);


#endif /* THRLCK_H */
