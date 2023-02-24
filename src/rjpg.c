
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "lodepng.h"
#include "tlpi_hdr.h"
#include "rjpg.h"

#define                  RJPG_BUF_SIZE  2048

#define                IFD_FLIR_OFFSET  0xb76 // root exif image file directory containing flir-related info
#define       OFFSET_RawThermalChunkSz  (0x50 + IFD_FLIR_OFFSET)
#define    OFFSET_RawThermalImageWidth  (0x202 + IFD_FLIR_OFFSET)
#define   OFFSET_RawThermalImageHeight  (0x204 + IFD_FLIR_OFFSET)
#define         OFFSET_RawThermalImage  (0x220 + IFD_FLIR_OFFSET)

static const uint8_t rjpg_ifd_magic[4] = {0x46, 0x4c, 0x49, 0x52}; // 'FLIR' @b6e
extern uint8_t vpl_data[12][768];

uint16_t flip_u16(const uint16_t val)
{
    uint16_t ret = 0;

    ret = (( val & 0xff ) << 8 ) | (( val & 0xff00) >> 8); 

    return ret;
}

void print_buf(uint8_t * data, const uint16_t size)
{
    uint16_t bytes_remaining = size;
    uint16_t bytes_to_be_printed, bytes_printed = 0;
    uint16_t i;

    while (bytes_remaining > 0) {

        if (bytes_remaining > 16) {
            bytes_to_be_printed = 16;
        } else {
            bytes_to_be_printed = bytes_remaining;
        }

        printf("%u: ", bytes_printed);

        for (i = 0; i < bytes_to_be_printed; i++) {
            printf("%02x", data[bytes_printed + i]);
            if (i & 0x1) {
                printf(" ");
            }
        }

        printf("\n");
        bytes_printed += bytes_to_be_printed;
        bytes_remaining -= bytes_to_be_printed;
    }
}

uint8_t rjpg_open(tgram_rjpg_t *th, char *in_file)
{
    struct stat st;     ///< stat structure that contains the input file size
    int fd;             ///< file descriptor for dtv file
    uint8_t *fm;        ///< dtv file contents copied to memory
    uint8_t *buf;       ///< read buffer
    ssize_t cnt, rcnt;  ///< read counters
    uint16_t *utemp;
    uint32_t *ltemp;
    uint32_t frame_sz;
    unsigned w, h;
    unsigned err = 0;

    // get file size
    if (stat(in_file, &st) < 0) {
        errExit("reading input file");
    }    

    fm = (uint8_t *) calloc(st.st_size, sizeof(uint8_t));
    if (fm == NULL) {
        errExit("allocating buffer");
        exit(EXIT_FAILURE);
    }

    // read input file
    if ((fd = open(in_file, O_RDONLY)) < 0) {
        errExit("opening input file");
    }

    buf = (uint8_t *) calloc(RJPG_BUF_SIZE, sizeof(uint8_t));
    if (buf == NULL) {
        errExit("allocating buffer");
        exit(EXIT_FAILURE);
    }

    // copy file to memory
    rcnt = 0;
    while ((cnt = read(fd, buf, RJPG_BUF_SIZE)) > 0) {
        if (rcnt + cnt <= st.st_size) {
            memcpy(fm + rcnt, buf, cnt);
        } else {
            fprintf(stderr, "write attempt beyond end of buffer");
        }
        rcnt += cnt;
    }

    if (memcmp(fm + IFD_FLIR_OFFSET - 8, rjpg_ifd_magic, 4) != 0) {
        fprintf(stderr, "unknown file type\n");
        exit(EXIT_FAILURE);
    }

    // populate rjpg header
    utemp = (uint16_t *)(fm + OFFSET_RawThermalImageWidth);
    th->head.raw_th_img_width = ntohs(*utemp);
    utemp = (uint16_t *)(fm + OFFSET_RawThermalImageHeight);
    th->head.raw_th_img_height = ntohs(*utemp);
    ltemp = (uint32_t *)(fm + OFFSET_RawThermalChunkSz);
    th->head.raw_th_img_sz = ntohl(*ltemp);
    th->head.raw_th_img = fm + OFFSET_RawThermalImage;

    frame_sz = th->head.raw_th_img_width * th->head.raw_th_img_height;
    
    // populate thermo frame
    th->frame = (uint8_t *) calloc(frame_sz, sizeof(uint8_t));
    if (buf == NULL) {
        errExit("allocating buffer");
    }

    err = lodepng_decode_memory(&(th->frame), &w, &h, th->head.raw_th_img, th->head.raw_th_img_sz, LCT_GREY, 8);

    if (err) {
        fprintf(stderr, "decoder error %u: %s\n", err, lodepng_error_text(err));
        return EXIT_FAILURE;
    }

    close(fd);
    free(buf);
    free(fm);

    return EXIT_SUCCESS;
}

uint8_t rjpg_transfer(const tgram_rjpg_t *th, uint8_t *image, const uint8_t pal, const uint8_t zoom)
{
    uint16_t i = 0;
    uint16_t row = 0;
    uint16_t th_width = th->head.raw_th_img_width;
    uint16_t th_height = th->head.raw_th_img_height;
    uint8_t zc;
    uint8_t *color;

    if (zoom == 1) {
        for (i = 0; i < th_width * th_height; i++) {
            memcpy(image + (i * 3), &(vpl_data[pal][th->frame[i] * 3]), 3);
        }
    } else {
        // resize by multiplying pixels
        for (row = 0; row < th_height; row++) {
            for (i = 0; i < th_width; i++) {
                color = &(vpl_data[pal][th->frame[row * th_width + i] * 3]);
                for (zc = 0; zc<zoom; zc++) {
                    // multiply each pixel zoom times
                    memcpy(image + ((row * th_width * zoom * zoom + i * zoom + zc) * 3), color, 3);
                }
            }
            for (zc = 1; zc<zoom; zc++) {
                // copy last row zoom times
                memmove(image + ((row * th_width * zoom * zoom + zc * zoom * th_width) * 3), image + ((row * th_width * zoom * zoom) * 3), th_width * zoom * 3);
            }
        }
    }

    return EXIT_SUCCESS;
}

#if 0

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

