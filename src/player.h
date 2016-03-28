#ifndef XIBOT_PLAYER_H
#define XIBOT_PLAYER_H
#include <xlfparser/xlfparser.h>
#include "attrs.h"
#include <pthread.h>

typedef struct _media_stop_param media_stop_param_t;
typedef struct _media_play_param media_play_param_t;
typedef struct _region_play_param region_play_param_t;
typedef struct _layout_play_param layout_play_param_t;

struct _media_stop_param {
    const char *media_id;
    const char *region_id;
    int layout_id;
    const char *path;
};

struct _media_play_param {
    Media *media;
    char *path;
    char *region_id;
    int layout_id;
    int stopped;
};

struct _region_play_param {
    int layout_id;
    Region *region;
    int nmedia;
    int nopt;
    char *saveDir;
    xibot_callback_fn media_play_cb;
};

struct _layout_play_param {
    Layout *layout;
    int layout_id;
    int nregion;
    int ntag;
    schedule_attr_t *schedule_info;
    xibot_callback_fn region_play_cb;
    xibot_callback_fn media_play_cb;
};

#ifdef	__cplusplus
extern "C" {
#endif
void media_play_param_free(media_play_param_t **mpp);
void region_play_param_free(region_play_param_t **rpp);

void *xibot_media_play(void *media_play_param);
void *xibot_region_play(void *region_play_param);
void *xibot_layout_play(void *layout_play_param);
int xibot_wait(int *i, xibot_callback_fn finished_cb, void *arg);
#ifdef	__cplusplus
}
#endif

#endif /* XIBOT_PLAYER_H */
