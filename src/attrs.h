#ifndef XIBOT_ATTRS_H
#define XIBOT_ATTRS_H
#include <time.h>

#define XIBOT_LAYOUT_FILE_SFMT "%s/%d.xlf"
#define XIBOT_MEDIA_FILE_SFMT "%s/%s"
#define XIBOT_RES_FILE_SFMT "%s/%d-%s-%s.html"
#define XIBOT_DATE_SFMT "%Y-%m-%d %H:%M:%S"

typedef void *(*xibot_callback_fn)(void *arg);

typedef struct _xibot_attr {
    const char *cfg_path;
    xibot_callback_fn on_schedule_cb;
    xibot_callback_fn on_layout_downloaded;
    xibot_callback_fn media_play_cb;
    xibot_callback_fn region_play_cb;
    xibot_callback_fn layout_play_cb;
} xibot_attr_t;


typedef struct _schedule_attr {
    int default_id;
    int layout_id;
    int prio;
    time_t from;
    time_t to;
    const char *saveDir;
} schedule_attr_t;

typedef struct _xibot_onsched_attr {
    schedule_attr_t *schedule_info;
    xibot_callback_fn media_play_cb;
    xibot_callback_fn region_play_cb;
    xibot_callback_fn layout_play_cb;
} xibot_onsched_attr_t;

typedef struct _xmds_callbacks {
    xibot_callback_fn on_schedule_cb;
    xibot_callback_fn on_layout_downloaded;
} xmds_callbacks_t;

typedef struct _display_info {
    int ready;
    int collectInterval;
    int scRequested;
} xibot_display_attr_t;

#endif /* XIBOT_ATTRS_H */
