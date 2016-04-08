#include "player.h"
#include "xibot.h"
#include "xignal.h"
#include "state.h"
#include <xmdsclient/xmds-util.h>
#include <xmdsclient/xmds.h>
#include <string.h>
#include <unistd.h>

extern thrlck_t _xibot_state;
void _xmds_run_thread(xmds_attr_t *attr);

int load_config(xmdsConfig *cfg, const char *path) {
    unsigned char *data;
    size_t len;
    int found;

    found = 0;
    data = NULL;

    data = file_get_contents(&len, path);
    if(data) {
        found = xmdsConfigParse(cfg, (const char *) data);
        free(data);
        data = NULL;
    }

    return found;
}

static int schedule_eq(const schedule_attr_t s1, const schedule_attr_t s2) {
    return (s1.default_id == s2.default_id &&
            s1.layout_id == s2.layout_id &&
            s1.from == s2.from &&
            s1.to == s2.to &&
            s1.prio == s2.prio &&
            !strcmp(s1.saveDir, s2.saveDir));
}

/* invoked by xmdsclient.c/schedule() */
void *xibot_schedule_handler(void *arg) {
    int nregion, ntag;
    char *path;
    int layout_id;
    int layout_changed;
    size_t path_len;
    xlfNode *node;
    Layout *layout;
    static int first;

    xibot_callback_fn layout_play_cb;

    static schedule_attr_t ssi;
    schedule_attr_t tsi; /* tmp */
    xibot_onsched_attr_t *xos_attr;
    layout_play_param_t *lpp;

    static pthread_t thread;
    pthread_attr_t thr_attr;

    if(xibot_is_interrupted()) {
        return NULL;
    }

    tsi.prio = -1;
    layout_changed = 0;
    pthread_attr_init(&thr_attr);
    pthread_attr_setdetachstate(&thr_attr, PTHREAD_CREATE_DETACHED);

    xos_attr = (xibot_onsched_attr_t *) arg;

    if(!schedule_eq(*xos_attr->schedule_info, ssi)) {
        tsi = ssi;
        ssi = *xos_attr->schedule_info;

        if(tsi.prio > ssi.prio) {
            ssi = tsi;
        }

        layout_changed = !first ? 0 : 1;
        first = 1;
    }

    layout_id = ssi.layout_id > 0 ?
                ssi.layout_id :
                ssi.default_id;

    path_len = strlen(ssi.saveDir) +
            digitlen(layout_id) + 6 /* slash, dot, x, l, f */;

    path = malloc(path_len + 1);
    path[path_len] = '\0';

    sprintf(path, XIBOT_LAYOUT_FILE_SFMT, ssi.saveDir, layout_id);
    node = xlfparser_parse_file(path, &nregion, &ntag);
    free(path);
    layout = xlfparser_get_layout(node);

    if(layout == NULL)
        return NULL;

    if(xos_attr->layout_play_cb != NULL) {
        layout_play_cb = xos_attr->layout_play_cb;
    } else {
        layout_play_cb = xibot_layout_play;
    }

    if(xibot_get_state(&_xibot_state, XIBOT_STATE_LAYOUT_PLAY) == XIBOT_STATE_FALSE) {
        if(!xibot_is_interrupted()) {
            lpp = malloc(sizeof(layout_play_param_t));
            lpp->layout = xlfparser_layout_dup(layout);
            lpp->layout_id = layout_id;
            lpp->nregion = nregion;
            lpp->ntag = ntag;
            lpp->schedule_info = &ssi;
            lpp->region_play_cb = xos_attr->region_play_cb;
            lpp->media_play_cb = xos_attr->media_play_cb;

            pthread_create(&thread, &thr_attr, layout_play_cb, lpp);
            fprintf(stderr, "Created play thread %lu\n", thread);
        }
    } else {
        fprintf(stderr, "Play thread %lu is still running\n", thread);
        if(layout_changed)
            pthread_kill(thread, SIGUSR1);
    }

    xlfparser_delete_layout(layout);

    if(xibot_is_interrupted()) {
        while(xibot_get_state(&_xibot_state, XIBOT_STATE_LAYOUT_PLAY) == XIBOT_STATE_TRUE) {
            fprintf(stderr, "Waiting play thread %lu\n", thread);
            usleep(100);
        }
    }

    return &thread;
}


void xibot_run(xibot_attr_t *xibot_attr) {
    xmdsConfig cfg;
    xmds_attr_t xmds_attr;
    xibot_running_t xibot_running;
    struct sigaction act, oact;

    xibot_set_sa(act, xibot_sigint_handler, oact, SIGINT);
    xibot_set_sa(act, xibot_sigusr1_handler, oact, SIGUSR1);

    memset(&xmds_attr, '\0', sizeof(xmds_attr_t));
    memset(&xibot_running, '\0', sizeof(xibot_running_t));

    _xibot_state.data = &xibot_running;

    if(xibot_attr->on_schedule_cb == NULL) {
        xibot_attr->on_schedule_cb = xibot_schedule_handler;
    }

    xmdsConfigInit(&cfg);
    load_config(&cfg, xibot_attr->cfg_path);

    xmds_attr.cfg = cfg;

    xmds_attr.media_play_cb = xibot_attr->media_play_cb;
    xmds_attr.region_play_cb = xibot_attr->region_play_cb;
    xmds_attr.layout_play_cb = xibot_attr->layout_play_cb;

    xmds_attr.callbacks.on_schedule_cb = xibot_attr->on_schedule_cb;
    xmds_attr.callbacks.on_layout_downloaded = xibot_attr->on_layout_downloaded;

    _xmds_run_thread(&xmds_attr);
    xmdsConfigFree(&cfg);

    fprintf(stderr, "\nxibot_run finished.\n");
    xibot_restore_sa(oact, SIGINT);
    xibot_restore_sa(oact, SIGUSR1);
}
