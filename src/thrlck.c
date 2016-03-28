#include "thrlck.h"

void thrlck_init(thrlck_t *psl) {
    pthread_mutex_init(&psl->lock, NULL);
    pthread_cond_init(&psl->rcond, NULL);
    pthread_cond_init(&psl->wcond, NULL);
    psl->lock_count = 0;
    psl->waiting_writers = 0;
    psl->data = NULL;
    psl->_type = 0;
}

static void thrlck_wait_rcleanup(void *arg) {
    thrlck_t *psl;
    psl = (thrlck_t *) arg;

    pthread_mutex_unlock(&psl->lock);
}

void thrlck_lock_read(thrlck_t *tl) {
    pthread_mutex_lock(&tl->lock);
    pthread_cleanup_push(thrlck_wait_rcleanup, tl);

    while ((tl->lock_count < 0) || (tl->waiting_writers != 0))
        pthread_cond_wait(&tl->rcond, &tl->lock);

    tl->lock_count++;
    pthread_cleanup_pop(1);
}

void thrlck_unlock_read(thrlck_t *tl) {
    pthread_mutex_lock(&tl->lock);
    if(--tl->lock_count == 0)
        pthread_cond_signal(&tl->wcond);

    pthread_mutex_unlock(&tl->lock);
}

static void thrlck_wait_wcleanup(void *arg) {
    thrlck_t *tl;

    tl = (thrlck_t *) arg;
    if ((--tl->waiting_writers == 0) && (tl->lock_count >= 0)) {
       /*
        * This only happens if we have been canceled. If the
        * lock is not held by a writer, there may be readers who
        * were blocked because waiting_writers was positive; they
        * can now be unblocked.
        */
        pthread_cond_broadcast(&tl->rcond);
    }
    pthread_mutex_unlock(&tl->lock);
}

void thrlck_lock_write(thrlck_t *tl) {
    pthread_mutex_lock(&tl->lock);
    tl->waiting_writers++;
    pthread_cleanup_push(thrlck_wait_wcleanup, tl);
    while(tl->lock_count != 0)
        pthread_cond_wait(&tl->wcond, &tl->lock);
    tl->lock_count = -1;
    pthread_cleanup_pop(1);
}

void thrlck_unlock_write(thrlck_t *tl) {
    pthread_mutex_lock(&tl->lock);
    tl->lock_count = 0;
    if(tl->waiting_writers == 0)
        pthread_cond_broadcast(&tl->rcond);
    else
        pthread_cond_signal(&tl->wcond);
    pthread_mutex_unlock(&tl->lock);
}

void *thrlck_get_data(thrlck_t *psl) {
    void *p;
    thrlck_lock_read(psl);
    pthread_cleanup_push(thrlck_unlock_read, psl);
    p = psl->data;
    pthread_cleanup_pop(1);
    return p;
}

void thrlck_set_data(thrlck_t *tl, void *val) {
    thrlck_lock_write(tl);
    pthread_cleanup_push(thrlck_unlock_write, tl);
    tl->data = val;
    pthread_cleanup_pop(1);
}

int thrlck_get_type(thrlck_t *psl) {
    int p;
    thrlck_lock_read(psl);
    pthread_cleanup_push(thrlck_unlock_read, psl);
    p = psl->_type;
    pthread_cleanup_pop(1);
    return p;
}

void thrlck_set_type(thrlck_t *tl, int type) {
    thrlck_lock_write(tl);
    pthread_cleanup_push(thrlck_unlock_write, tl);
    tl->_type = type;
    pthread_cleanup_pop(1);
}

