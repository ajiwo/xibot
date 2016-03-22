#ifndef XIBOT_SIGNAL_H
#define XIBOT_SIGNAL_H
#include <pthread.h>
#include <signal.h>

enum {
    XIBOT_SIGINT_ID = 0,
    XIBOT_SIGUSR1_ID
};

#define xibot_thr_exists(A) (((A) > 0) ? (pthread_kill((A), 0) == 0) : 0)
#define xibot_interrupted (xibot_sig_query(XIBOT_SIGINT_ID, -1) == SIGINT)
#define xibot_play_interrupted (xibot_sig_query(XIBOT_SIGUSR1_ID, -1) == SIGUSR1)
#define xibot_set_sigusr1 xibot_sig_query(XIBOT_SIGUSR1_ID, SIGUSR1)
#define xibot_clear_sigusr1 xibot_sig_query(XIBOT_SIGUSR1_ID, 0)
#define xibot_set_sigint xibot_sig_query(XIBOT_SIGINT_ID, SIGINT)
#define xibot_clear_sigint xibot_sig_query(XIBOT_SIGINT_ID, 0)

#define xibot_set_sa(A, F, B, S) \
    (A).sa_handler = (F); \
    sigemptyset(&(A).sa_mask); \
    (A).sa_flags = 0; \
    sigaction(S, NULL, &(B)); \
    if((B).sa_handler != SIG_IGN) sigaction(S, &(A), NULL)
#define xibot_restore_sa(B, S) sigaction(S, &(B), NULL)

#ifdef	__cplusplus
extern "C" {
#endif

int xibot_sig_query(int id, int newval);
void xibot_sigint_handler(int signum);
void xibot_sigusr1_handler(int signum);

#ifdef	__cplusplus
}
#endif

#endif /* XIBOT_SIGNAL_H */
