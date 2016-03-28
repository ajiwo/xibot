#include "xignal.h"
#include "state.h"

void xibot_sigint_handler(int signum) {
    if(signum == SIGINT) {
        xibot_set_state(&_xibot_state, XIBOT_INTERRUPTED, XIBOT_STATE_TRUE);
        xibot_set_state(&_xibot_state, XIBOT_PLAY_INTERRUPTED, XIBOT_STATE_TRUE);
    }
}

void xibot_sigusr1_handler(int signum) {
    if(signum == SIGUSR1) {
        xibot_set_state(&_xibot_state, XIBOT_PLAY_INTERRUPTED, XIBOT_STATE_TRUE);
    }
}
