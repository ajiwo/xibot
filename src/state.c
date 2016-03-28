#include "state.h"

void xibot_set_state(thrlck_t *tl, int id, int val) {
    xibot_running_t *state;

    state = thrlck_get_data(tl);
    if(id == XIBOT_STATE_XMDS) {
        state->xmds_running = val;
    }
    else if(id == XIBOT_STATE_MEDIA_PLAY) {
        state->media_playing = val;
    }
    else if(id == XIBOT_STATE_REGION_PLAY) {
        state->region_playing = val;
    }
    else if(id == XIBOT_STATE_LAYOUT_PLAY) {
        state->layout_playing = val;
    }
    else if(id == XIBOT_PLAY_INTERRUPTED) {
        state->play_interrupted = val;
    }
    else if(id == XIBOT_INTERRUPTED) {
        state->interrupted = val;
    }

    thrlck_set_data(tl, state);
}

int xibot_get_state(thrlck_t *tl, int id) {
    xibot_running_t *state;
    int val;
    state = thrlck_get_data(tl);

    val = -1;
    if(id == XIBOT_STATE_XMDS) {
        val = state->xmds_running;
    }
    else if(id == XIBOT_STATE_MEDIA_PLAY) {
        val = state->media_playing;
    }
    else if(id == XIBOT_STATE_REGION_PLAY) {
        val = state->region_playing;
    }
    else if(id == XIBOT_STATE_LAYOUT_PLAY) {
        val = state->layout_playing;
    }
    else if(id == XIBOT_PLAY_INTERRUPTED) {
        val = state->play_interrupted;
    }
    else if(id == XIBOT_INTERRUPTED) {
        val = state->interrupted;
    }

    return val;
}

int xibot_is_interrupted() {
    extern thrlck_t _xibot_state;

    return xibot_get_state(&_xibot_state, XIBOT_INTERRUPTED) == XIBOT_STATE_TRUE;
}

int xibot_is_play_interrupted() {
    extern thrlck_t _xibot_state;

    return xibot_get_state(&_xibot_state, XIBOT_PLAY_INTERRUPTED) == XIBOT_STATE_TRUE;
}

int xmds_is_running() {
    extern thrlck_t _xibot_state;
    return xibot_get_state(&_xibot_state, XIBOT_STATE_XMDS) == XIBOT_STATE_TRUE;
}
