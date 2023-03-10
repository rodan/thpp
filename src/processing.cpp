
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "thermogram.h"
#include "processing.h"

uint8_t check_th(tgram_t *th)
{
    if (th == NULL) {
        return EXIT_FAILURE;
    }

    if (th->frame == NULL) {
        return EXIT_FAILURE;
    }

    switch (th->type) {
        case TH_IRTIS_DTV:
            if (th->head.dtv == NULL) {
                return EXIT_FAILURE;
            }
            break;
        case TH_FLIR_RJPG:
            if (th->head.rjpg == NULL) {
                return EXIT_FAILURE;
            }
            break;
        default:
            return EXIT_FAILURE;
            break;
    }

    return EXIT_SUCCESS;
}


uint8_t proc_get_limits(tgram_t *th, proc_limits_t *data)
{
    uint32_t frame_sz = 0;
    uint32_t i;
    uint8_t c;
    uint32_t total = 0;

    if (check_th(th) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    switch (th->type) {
        case TH_IRTIS_DTV:
            frame_sz = th->head.dtv->nst * th->head.dtv->nstv;
            break;
        case TH_FLIR_RJPG:
            frame_sz = th->head.rjpg->raw_th_img_width * th->head.rjpg->raw_th_img_height;
            break;
        default:
            return EXIT_FAILURE;
            break;
    }

    if (frame_sz == 0) {
        return EXIT_FAILURE;
    }

    memset(data, 0, sizeof(proc_limits_t));

    data->umin = 255;
    data->umax = 0;
    data->fmin = 65000.0;
    data->fmax = -65000.0;
    
    for (i=0; i<frame_sz; i++) {
        c = th->frame[i];
        if (data->umin > c) {
            data->umin = c;
        }
        if (data->umax < c) {
            data->umax = c;
        }
        total += c;
    }

    data->uavg = total / frame_sz;

    return EXIT_SUCCESS;
}


