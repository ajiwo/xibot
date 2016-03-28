#ifndef XIBOT_STATE_H
#define XIBOT_STATE_H
#include "thrlck.h"

enum {
    XIBOT_STATE_FALSE = 0,
    XIBOT_STATE_TRUE,
    XIBOT_STATE_XMDS,
    XIBOT_STATE_MEDIA_PLAY,
    XIBOT_STATE_REGION_PLAY,
    XIBOT_STATE_LAYOUT_PLAY,

    XIBOT_PLAY_INTERRUPTED,
    XIBOT_INTERRUPTED
};

typedef struct _xibot_running {
    int media_playing;
    int region_playing;
    int layout_playing;
    int xmds_running;
    int interrupted;
    int play_interrupted;
} xibot_running_t;


thrlck_t _xibot_state;

void xibot_set_state(thrlck_t *tl, int id, int val);
int xibot_get_state(thrlck_t *tl, int id);

int xibot_is_interrupted();
int xibot_is_play_interrupted();
int xmds_is_running();

#endif
