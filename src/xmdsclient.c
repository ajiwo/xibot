#include "attrs.h"
#include "state.h"
#include "simple-time.h"
#include "xignal.h"
#include <xmdsclient/xmds.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

extern thrlck_t _xibot_state;

static int register_display(xmdsConfig cfg, xmds_display_attr_t *props) {
    xmdsNode *node;
    registerDisplayEntry *entry;
    registerDisplayDetail *detail;

    int i, nd;

    node = xmdsRegisterDisplay(cfg, "libxmdsclient", "Linux", "Android", "0.0.1", 1, "ab:ac:ad:ae:af:bc");

    entry = xmdsRegisterDisplayEntry(node, &nd);
    if(nd < 1 || entry == NULL) {
        props->collectInterval = -1;
        props->ready = 0;
    } else {
        detail = xmdsRegisterDisplayDetail(entry, &i, "collectInterval");
        props->collectInterval = atoi(detail->value);

        detail = xmdsRegisterDisplayDetail(entry, &i, "screenshotRequested");
        props->scRequested = atoi(detail->value);

        props->ready = entry->status == 0;
    }
    xmdsFree(node);

    return props->ready;
}

static int md5sum_match(const char *path, const char *md5um) {
    char _md5sum[32];

    xmdsMd5sumFile(_md5sum, path);

    return !strcmp(_md5sum, md5um);
}

static int required_files(xmds_attr_t *xmds_params) {
    xmdsNode *node;
    requiredFilesEntry *entry;
    int i, len;
    size_t downloaded;
    char tmp[513];
    node = xmdsRequiredFiles(xmds_params->cfg, &len);

    for(i = 0; i < len; i++) {
        if(xibot_is_interrupted())
            break;

        entry = xmdsRequiredFilesEntry(node, i);
        if(entry) {
            /*
            fprintf(stderr, "  type '%s', id '%s', md5 '%s', size '%ld', path '%s', download '%s'\n",
                   entry->type, entry->id, entry->md5, entry->size, entry->path, entry->download);
            */
            if(!strcmp(entry->type, "media")) {
                memset(tmp, '\0', sizeof(tmp));
                sprintf(tmp, XIBOT_MEDIA_FILE_SFMT, xmds_params->cfg.saveDir, entry->path);
                if(!md5sum_match(tmp, entry->md5)) {
                    downloaded = xmdsGetFileNamed(xmds_params->cfg, atoi(entry->id), entry->type, 0, entry->size, tmp);
                    printf("donwloaded: %s %u\n", tmp, downloaded);
                } else {
                    /*printf("Skipping %s, md5sum %s match\n", tmp, entry->md5);*/
                }
            }
            if(!strcmp(entry->type, "layout")) {
                memset(tmp, '\0', sizeof(tmp));
                sprintf(tmp, XIBOT_LAYOUT_FILE_SFMT, xmds_params->cfg.saveDir, atoi(entry->id));
                if(!md5sum_match(tmp, entry->md5)) {
                    downloaded = xmdsGetFileNamed(xmds_params->cfg, atoi(entry->id), entry->type, 0, entry->size, tmp);
                    if(xmds_params->callbacks.on_layout_downloaded != NULL) {
                        xmds_params->callbacks.on_layout_downloaded(NULL);
                    }
                    fprintf(stderr, "donwloaded: %s %u\n", tmp, downloaded);
                } else {
                    /*printf("Skipping %s, md5sum %s match\n", tmp, entry->md5);*/
                }
            }
            if(!strcmp(entry->type, "resource")) {
                memset(tmp, '\0', sizeof(tmp));
                sprintf(tmp, XIBOT_RES_FILE_SFMT, xmds_params->cfg.saveDir, entry->layoutid, entry->regionid, entry->mediaid);
                /*downloaded = */xmdsGetResourceFile(xmds_params->cfg, entry->layoutid, entry->mediaid, entry->regionid, tmp);
                /* fprintf(stderr, "donwloaded: %s %u\n", tmp, downloaded); */
            }
        }
    }
    xmdsFree(node);
    if(xibot_is_interrupted())
        fprintf(stderr, "required_files() INTERRUPTED\n");

    return len;
}

static int schedule(xmds_attr_t *xmds_param) {
    xmdsNode *node;
    scheduleEntry *s_entry;
    scheduleLayoutEntry *l_entry;
    schedule_attr_t si;
    xibot_onsched_attr_t xos_attr;
    int i;
    /* scheduled layout count */
    int nl;
    /* deps count */
    int nd;

    struct tm stm;

    int def_layout = -1;

    time_t from_date, now, to_date;

    memset(&si, '\0', sizeof(schedule_attr_t));
    memset(&stm, '\0', sizeof(struct tm));
    memset(&xos_attr, '\0', sizeof(xibot_onsched_attr_t));
    si.saveDir = xmds_param->cfg.saveDir;
    node = xmdsSchedule(xmds_param->cfg, &nl, &nd);
    if(node) {
        s_entry = node->data;
        def_layout = s_entry->defaultLayout;
        si.default_id = def_layout;
        xos_attr.schedule_info = &si;
        xos_attr.layout_play_cb = xmds_param->layout_play_cb;
        xos_attr.region_play_cb = xmds_param->region_play_cb;
        xos_attr.media_play_cb = xmds_param->media_play_cb;
        for(i = 0; i < nl; i++) {
            l_entry = xmdsScheduleLayout(s_entry, i);
            if(l_entry) {
                now = time(NULL) + xmds_param->cfg.cmsTzOffset;
                from_date = str_to_epoch(l_entry->fromDate, XIBOT_DATE_SFMT);
                to_date = str_to_epoch(l_entry->toDate, XIBOT_DATE_SFMT);
                if(now >= from_date && now <= to_date) {
                    si.layout_id = atoi(l_entry->file);
                    si.from = from_date;
                    si.to = to_date;
                    xos_attr.schedule_info = &si;
                    if(xmds_param->callbacks.on_schedule_cb != NULL)
                        xmds_param->callbacks.on_schedule_cb(&xos_attr);

                }
            }
        }

        /* no scheduled layout, inform the default layout */
        if(xmds_param->callbacks.on_schedule_cb != NULL && nl == 0)
            xmds_param->callbacks.on_schedule_cb(&xos_attr);

    }

    xmdsFree(node);

    return def_layout;
}

void *xmds_cycle(void *arg) {
    xmds_attr_t *xmds_attr;
    xmds_display_attr_t di;

    time_t t1;

    if(xmds_is_running())
        return NULL;

    memset(&di, '\0', sizeof(xmds_display_attr_t));
    xmds_attr = (xmds_attr_t *) arg;
    xibot_set_state(&_xibot_state, XIBOT_STATE_XMDS, XIBOT_STATE_TRUE);
    register_display(xmds_attr->cfg, &di);

    if(di.collectInterval > 0) {
        if(required_files(xmds_attr)) {
            schedule(xmds_attr);
        }
        t1 = time(NULL) + di.collectInterval;
    } else {
        t1 = time(NULL) + xmds_attr->cfg.collectInterval;
    }

    while(time(NULL) < t1) {
        usleep(100);
        if(xibot_is_play_interrupted()) {
            usleep(50);
            break;
        }
        if(xibot_is_interrupted())
            break;
    }

    if(xibot_is_play_interrupted()) {
        while(xibot_get_state(&_xibot_state, XIBOT_STATE_LAYOUT_PLAY) == XIBOT_STATE_TRUE) {
            usleep(100);
        }
        xibot_set_state(&_xibot_state, XIBOT_PLAY_INTERRUPTED, XIBOT_STATE_FALSE);
    }

    xibot_set_state(&_xibot_state, XIBOT_STATE_XMDS, XIBOT_STATE_FALSE);
    return NULL;
}


/* invoked by xibot.c/xibot_run() */
void _xmds_run_thread(xmds_attr_t *xmds_attr) {
    static pthread_t pth;
    pthread_attr_t pth_attr;

    pthread_attr_init(&pth_attr);
    pthread_attr_setdetachstate(&pth_attr, PTHREAD_CREATE_DETACHED);

    while(1) {
        if(xibot_is_interrupted())
            break;
        if(!xmds_is_running()) {
            if(pthread_create(&pth, &pth_attr, xmds_cycle, xmds_attr) == 0) {
                fprintf(stderr, "Created xmds thread %lu\n",pth);
            }
        }
        usleep(100);
    }

    while(xmds_is_running())
        usleep(100);

    pthread_attr_destroy(&pth_attr);
}
