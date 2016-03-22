#include "xignal.h"
#include <stdlib.h>

int xibot_sig_query(int id, int newval) {
    static int val[2];

    if(newval > -1) {
        val[id] = newval;
        return 0;
    }

    return val[id];
}

void xibot_sigint_handler(int signum) {
    xibot_sig_query(XIBOT_SIGINT_ID, signum);
}

void xibot_sigusr1_handler(int signum) {
    xibot_sig_query(XIBOT_SIGUSR1_ID, signum);
}
