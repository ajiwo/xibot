#include "state.h"
#include "xignal.h"
#include <xmdsclient/xmds-util.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "player.h"

extern thrlck_t _xibot_state;
int xibot_wait(int *i, xibot_callback_fn finished_cb, void *arg) {
    while(*i == 0) {
        if(xibot_is_interrupted() || xibot_is_play_interrupted()) {
            *i = 2;
            fprintf(stderr, "xibot_wait INTERRUPTED\n");
            break;
        }
        usleep(100);
    }
    *i = 1;
    if(finished_cb != NULL)
        finished_cb(arg);

    return *i;
}

void media_play_param_free(media_play_param_t **mpp) {
    if(*mpp != NULL) {
        free((*mpp)->path);
        free((*mpp)->region_id);
        xlfparser_delete_media((*mpp)->media);
        free(*mpp);
        *mpp = NULL;
    }
}

void region_play_param_free(region_play_param_t **rpp) {
    if(*rpp != NULL) {
        free((*rpp)->saveDir);
        xlfparser_delete_region((*rpp)->region);
        free(*rpp);
        *rpp = NULL;
    }
}


void *xibot_media_play(void *arg) {
    media_play_param_t *mpp;
    time_t t1, t2;

    mpp = (media_play_param_t *) arg;

    /*
    fprintf(stderr, "xibot_media_play() media %s from region %s is playing for %d seconds\n",
            mpp->media->id, mpp->region_id, mpp->media->duration);
    */
    t1 = time(NULL) + mpp->media->duration;

    while((t2 = time(NULL)) < t1) {
        if(xibot_is_play_interrupted())
            break;
        usleep(100);
    }

    /*
    if(xibot_interrupted)
        fprintf(stderr, "xibot_media_play() INTERRUPTED\n");
    */
    media_play_param_free(&mpp);
    return NULL;
}

void *xibot_region_play(void *arg) {
    region_play_param_t *rpp;
    media_play_param_t *mpp;
    int i, loop;
    int interrupted;
    Media *media;
    MediaOption *ouri, *oloop;
    char *path;
    size_t path_len;

    rpp = (region_play_param_t *) arg;

    interrupted = 0;
    loop = 0;
    path = NULL;
    media = NULL;
    mpp = NULL;
    path_len = strlen(rpp->saveDir) + strlen(rpp->region->id) + digitlen(rpp->layout_id) + 9;
    do {
        for(i = 0; i < rpp->nmedia; i++) {
            if((interrupted = xibot_is_play_interrupted()))
                continue;

            media = xlfparser_get_media(rpp->region, i);
            ouri = xlfparser_get_media_option(media, 0, "uri");
            oloop = xlfparser_get_region_option(rpp->region, 0, "loop");
            mpp = malloc(sizeof(media_play_param_t));

            if(ouri && ouri->value) {
                path = malloc(path_len + 1);
                path[path_len] = '\0';
                if(!strcmp(media->type, "webpage")) {
                    sprintf(path, "%s", ouri->value);
                } else {
                    sprintf(path, XIBOT_MEDIA_FILE_SFMT, rpp->saveDir, ouri->value);
                }
            } else {
                path_len += strlen(media->id);
                path = malloc(path_len + 1);
                path[path_len] = '\0';
                sprintf(path, XIBOT_RES_FILE_SFMT,
                        rpp->saveDir, rpp->layout_id, rpp->region->id, media->id);
            }
            mpp->path = str_duplicate(path);
            mpp->media = xlfparser_media_dup(media);
            mpp->region_id = str_duplicate(rpp->region->id);
            mpp->layout_id = rpp->layout_id;
            mpp->stopped = 0;

            free(path);
            if(rpp->media_play_cb != NULL) {
                rpp->media_play_cb(mpp);
            } else {
                xibot_media_play(mpp);
            }

            loop = (oloop && atoi(oloop->value) > 0) ? 1 : 0;
        }
    } while(loop && !interrupted);
    /*
    fprintf(stderr, "region %s finished\n", rpp->region->id);
    */
    region_play_param_free(&rpp);
    pthread_exit(NULL);
    return NULL;
}

void *xibot_layout_play(void *arg) {
    int i, j;
    int nopt, nmedia, nregion;
    Region *region;
    layout_play_param_t *lpp;

    region_play_param_t *rpp;
    pthread_t *threads;
    pthread_attr_t attrs;

    lpp = (layout_play_param_t *) arg;

    nregion = lpp->nregion;

    pthread_attr_init(&attrs);
    pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);

    threads = calloc(nregion, sizeof(pthread_t));

    xibot_set_state(&_xibot_state, XIBOT_STATE_LAYOUT_PLAY, XIBOT_STATE_TRUE);
    for(i = 0; i < nregion; i++) {
        if(xibot_is_play_interrupted()) {
            /*
            fprintf(stderr, "xibot_layout_play() skipping region %d\n", i);
            */
            usleep(500);
            break;
        }

        region = xlfparser_get_region(lpp->layout, i, &nopt, &nmedia);

        rpp = malloc(sizeof(region_play_param_t));
        rpp->region = xlfparser_region_dup(region);
        rpp->nmedia = nmedia;
        rpp->media_play_cb = lpp->media_play_cb;
        rpp->saveDir = str_duplicate(lpp->schedule_info->saveDir);
        rpp->layout_id = lpp->layout_id;

        if(lpp->region_play_cb != NULL) {
            pthread_create(&threads[i], &attrs, lpp->region_play_cb, rpp);
        } else {
            pthread_create(&threads[i], &attrs, xibot_region_play, rpp);
        }
    }

    for(j = 0; j < i; j++) {
        if(pthread_join(threads[j], NULL) != 0)
            fprintf(stderr, "xibot_layout_play() pthread_join failed\n");
    }
    pthread_attr_destroy(&attrs);

    /*fprintf(stderr, "xibot_layout_play() All regions have finished\n");*/
    free(threads);
    threads = NULL;
    xlfparser_delete_layout(lpp->layout);
    free(arg);
    xibot_set_state(&_xibot_state, XIBOT_STATE_LAYOUT_PLAY, XIBOT_STATE_FALSE);

    return NULL;
}
