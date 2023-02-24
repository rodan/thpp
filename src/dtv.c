
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tlpi_hdr.h"
#include "dtv.h"

#define   DTV_BUF_SIZE  2048

extern uint8_t vpl_data[12][768];

uint8_t dtv_open(tgram_t *thermo, char *dtv_file)
{
    struct stat st;     ///< stat structure that contains the dtv file size
    int fd;             ///< file descriptor for dtv file
    uint8_t *fm;        ///< dtv file contents copied to memory
    uint8_t *buf;       ///< read buffer
    ssize_t cnt, rcnt;  ///< read counters
    ssize_t frame_sz;   ///< total size of frames in the input file

    // get file size
    if (stat(dtv_file, &st) < 0) {
        errExit("reading input file");
    }    

    fm = (uint8_t *) calloc(st.st_size, sizeof(uint8_t));
    if (fm == NULL) {
        errExit("allocating buffer");
        exit(EXIT_FAILURE);
    }

    // read input file
    if ((fd = open(dtv_file, O_RDONLY)) < 0) {
        errExit("opening input file");
    }

    buf = (uint8_t *) calloc(DTV_BUF_SIZE, sizeof(uint8_t));
    if (buf == NULL) {
        errExit("allocating buffer");
        exit(EXIT_FAILURE);
    }

    // copy file to memory
    rcnt = 0;
    while ((cnt = read(fd, buf, DTV_BUF_SIZE)) > 0) {
        if (rcnt + cnt <= st.st_size) {
            memcpy(fm + rcnt, buf, cnt);
        } else {
            fprintf(stderr, "write attempt beyond end of buffer");
        }
        rcnt += cnt;
    }

    close(fd);

    // populate thermo header
    memcpy(&(thermo->head), fm, DTV_HEADER_SZ);

    frame_sz = thermo->head.nst * thermo->head.nstv * thermo->head.frn;
    if (frame_sz < 256*248) {
        fprintf(stderr, "warning: unexpected image size %dx%dx%d\n", thermo->head.nst, thermo->head.nstv, thermo->head.frn);
    }

    // populate thermo frame
    thermo->frame = (uint8_t *) calloc(frame_sz, sizeof(uint8_t));
    if (buf == NULL) {
        errExit("allocating buffer");
    }
    memcpy(thermo->frame, fm + DTV_HEADER_SZ, frame_sz);

    free(buf);
    free(fm);

    return EXIT_SUCCESS;
}

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


void dtv_close(tgram_t *thermo)
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

