
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <libexif/exif-data.h>
#include "tlpi_hdr.h"
#include "rjpg.h"

#define   RJPG_BUF_SIZE  2048

extern uint8_t vpl_data[12][768];

static void trim_spaces(char *buf)
{
    char *s = buf - 1;
    for (; *buf; ++buf) {
        if (*buf != ' ')
            s = buf;
    }
    *++s = 0;                   /* nul terminate the string on the first of the final spaces */
}

static void show_mnote_tag(ExifData * d, unsigned tag)
{
    ExifMnoteData *mn = exif_data_get_mnote_data(d);
    if (mn) {
        int num = exif_mnote_data_count(mn);
        int i;

        printf("found %d makernotes\n", num);
        /* Loop through all MakerNote tags, searching for the desired one */
        for (i = 0; i < num; ++i) {
            char buf[1024];
            if (exif_mnote_data_get_id(mn, i) == tag) {
                if (exif_mnote_data_get_value(mn, i, buf, sizeof(buf))) {
                    /* Don't bother printing it if it's entirely blank */
                    trim_spaces(buf);
                    if (*buf) {
                        printf("%s: %s\n", exif_mnote_data_get_title(mn, i), buf);
                    }
                }
            }
        }
    } else {
        printf("no makernotes found\n");
    }
}

uint8_t rjpg_open(tgram_rjpg_t *thermo, char *in_file)
{
    ExifData *ed;
    ExifEntry *entry;
    
    ed = exif_data_new_from_file(in_file);
    if (!ed) {
        fprintf(stderr, "file not readable or no EXIF data in file %s\n", in_file);
        return EXIT_FAILURE;
    }

    exif_data_unset_option(ed, EXIF_DATA_OPTION_IGNORE_UNKNOWN_TAGS);
    exif_data_unset_option(ed, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);

    //exif_data_dump(ed);
    show_mnote_tag(ed, 0x1);

    return EXIT_SUCCESS;
}

#if 0

uint8_t dtv_transfer(const tgram_t *th, uint8_t *image, const uint8_t pal, const uint8_t zoom)
{
    uint16_t i = 0;
    uint16_t row = 0;
    uint16_t res_x = th->head.nst;
    uint16_t res_y = th->head.nstv;
    uint8_t zc;
    uint8_t *color;

    if (zoom == 1) {
        for (i = 0; i < res_x * res_y; i++) {
            memcpy(image + (i * 3), &(vpl_data[pal][th->frame[i] * 3]), 3);
        }
    } else {
        // resize by multiplying pixels
        for (row = 0; row < res_y; row++) {
            for (i = 0; i < res_x; i++) {
                color = &(vpl_data[pal][th->frame[row * res_x + i] * 3]);
                for (zc = 0; zc<zoom; zc++) {
                    // multiply each pixel zoom times
                    memcpy(image + ((row * res_x * zoom * zoom + i * zoom + zc) * 3), color, 3);
                }
            }
            for (zc = 1; zc<zoom; zc++) {
                // copy last row zoom times
                memmove(image + ((row * res_x * zoom * zoom + zc * zoom * res_x) * 3), image + ((row * res_x * zoom * zoom) * 3), res_x * zoom * 3);
            }
        }
    }

    return EXIT_SUCCESS;
}

uint8_t dtv_rescale(tgram_t *dst_th, const tgram_t *src_th, const float new_min, const float new_max)
{
    ssize_t frame_sz;
    ssize_t i;
    double ft;
    uint8_t ut;

    // populate dst thermo header
    memcpy(&(dst_th->head), &(src_th->head), DTV_HEADER_SZ);

    frame_sz = src_th->head.nst * src_th->head.nstv * src_th->head.frn;
    if (frame_sz < 256*248) {
        fprintf(stderr, "warning: unexpected image size %dx%dx%d\n", src_th->head.nst, src_th->head.nstv, src_th->head.frn);
    }
    dst_th->head.tsc[1] = new_min;
    dst_th->head.tsc[0] = (new_max - new_min) / 256.0;

    // populate dst thermo frame
    dst_th->frame = (uint8_t *) calloc(frame_sz, sizeof(uint8_t));
    if (dst_th->frame == NULL) {
        errExit("allocating buffer");
    }

    for (i=0; i<frame_sz; i++) {
        ft = ((src_th->head.tsc[0] * src_th->frame[i] + src_th->head.tsc[1] - dst_th->head.tsc[1]) / dst_th->head.tsc[0]) + 0.5;
        if (ft < 0) {
            ut = 0;
        } else if (ft > 255) {
            ut = 255;
        } else {
            ut = (uint8_t) ft;
        }
        dst_th->frame[i] = ut;
    }

    return EXIT_SUCCESS;
}
#endif

void rjpg_close(tgram_rjpg_t *thermo)
{
    if (thermo) {
        if (thermo->frame) {
            free(thermo->frame);
        }
    }

    if (thermo) {
        free(thermo);
    }
}

