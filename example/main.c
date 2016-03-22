#include "../src/xibot.h"
#include "../src/player.h"
#include <xlfparser/xlfparser.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void *my_media_play(void *arg) {
    media_play_param_t *mpp;
    time_t t1;
    mpp = (media_play_param_t *) arg;
    fprintf(stderr, "media %s playing for %d seconds\n", mpp->media->id, mpp->media->duration);
    t1 = time(NULL) + mpp->media->duration;

    /* play simulation */
    while(time(NULL) < t1)
        sleep(1);

    media_play_param_free(&mpp);
    return NULL;
}

void *my_region_play(void *arg) {
    region_play_param_t *rpp;
    rpp = (region_play_param_t *) arg;
    fprintf(stderr, "region %s playing\n", rpp->region->id);
    return xibot_region_play(arg);
}

void *my_layout_play(void *arg) {
    layout_play_param_t *lpp;
    lpp = (layout_play_param_t *) arg;
    fprintf(stderr, "layout w: %d, h: %d, bgcolor: %s\n",
            lpp->layout->width,
            lpp->layout->height,
            lpp->layout->bgcolor);
    return xibot_layout_play(arg);
}

int main() {
    xibot_attr_t attr;
    memset(&attr, '\0', sizeof(xibot_attr_t));
    attr.cfg_path = "/tmp/xibot/xibot.cfg";

    attr.layout_play_cb = my_layout_play;
    attr.region_play_cb = my_region_play;
    attr.media_play_cb = my_media_play;
    xibot_run(attr);
    return 0;
}
