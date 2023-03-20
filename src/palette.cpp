
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tlpi_hdr.h"
#include "palette.h"

#define  PAL_STOPS  13

struct palette {
    char name[16];
    uint8_t stop_cnt;
    uint16_t stop_offset[PAL_STOPS];
    uint8_t rgb[PAL_STOPS * 3];
};

typedef struct palette palette_t;

static const palette_t pal_db[13] = {
    {
     "256", // 0
     7,
     {0, 10260, 21040, 32891, 43865, 60032, 65535},
     {0x00, 0x00, 0x00,
      0x5f, 0x00, 0x60,
      0x00, 0x00, 0xf7,
      0x00, 0xf7, 0x00,
      0xf8, 0x00, 0x00,
      0xf8, 0xf8, 0x0c,
      0xff, 0xff, 0xff}
     },
    {
     "color", // 1
     9,
     {0, 4308, 12933, 22058, 30828, 43661, 53070, 56709, 65535},
     {0x00, 0x00, 0x00,
      0x52, 0x00, 0x52,
      0x00, 0x00, 0xa3,
      0x00, 0xa8, 0xa5,
      0x05, 0xa8, 0x00,
      0xfc, 0xa6, 0x00,
      0xfc, 0x2c, 0x00,
      0xfc, 0xfc, 0x00,
      0xff, 0xff, 0xff}
     },
    {
     "grey", // 2
     2,
     {0, 65535},
     {0x00, 0x00, 0x00,
      0xff, 0xff, 0xff}
     },
    {
     "hmetal0", // 3
     7,
     {0, 1506, 32431, 41122, 49189, 65237, 65535},
     {0x00, 0x00, 0x00,
      0x08, 0x00, 0x00,
      0xfc, 0x00, 0x00,
      0xfc, 0x80, 0x00,
      0xfc, 0xf9, 0x00,
      0xfc, 0xfc, 0xf7,
      0xff, 0xff, 0xff}
     },
    {
     "hmetal1", // 4
     8,
     {0, 1506, 22179, 29516, 44285, 50570, 65237, 65535},
     {0x00, 0x00, 0x00,
      0x0c, 0x00, 0x00,
      0xc0, 0x00, 0x00,
      0xfc, 0x44, 0x00,
      0xfc, 0xc8, 0x2d,
      0xfc, 0xfc, 0x6c,
      0xfc, 0xfc, 0xf9,
      0xff, 0xff, 0xff}
     },
    {
     "hmetal2", // 5
     8,
     {0, 1506, 22179, 29516, 44285, 50570, 65237, 65535},
     {0x00, 0x00, 0x00,
      0x0c, 0x00, 0x00,
      0xbc, 0x18, 0x00,
      0xe8, 0x50, 0x00,
      0xfc, 0xc4, 0x44,
      0xfc, 0xec, 0x6c,
      0xfc, 0xfc, 0xfc,
      0xff, 0xff, 0xff}
     },
    {
     "hotblue1", // 6
     8,
     {0, 1506, 11933, 30650, 47146, 56705, 65237, 65535},
     {0x00, 0x00, 0x00,
      0x08, 0x00, 0x16,
      0x3f, 0x00, 0x8b,
      0xab, 0x08, 0x00,
      0xfc, 0xa4, 0x1f,
      0xfc, 0xf0, 0x70,
      0xfc, 0xfc, 0xf8,
      0xff, 0xff, 0xff}
     },
    {
     "hotblue2", // 7
     8,
     {0, 1506, 6950, 30650, 47146, 56705, 65237, 65535},
     {0x00, 0x00, 0x00,
      0x08, 0x00, 0x1b,
      0x24, 0x00, 0x6b,
      0xbd, 0x1a, 0x00,
      0xfc, 0xac, 0x26,
      0xfc, 0xf0, 0x72,
      0xfc, 0xfc, 0xf0,
      0xff, 0xff, 0xff}
     },
    {
     "iron", // 8
     9,
     {0, 1506, 4713, 10768, 23321, 39105, 57443, 65237, 65535},
     {0x00, 0x00, 0x00,
      0x04, 0x06, 0x56,
      0x23, 0x0f, 0x72,
      0x62, 0x08, 0xa0,
      0xbf, 0x27, 0x8d,
      0xf8, 0x80, 0x1e,
      0xf8, 0xe2, 0x4f,
      0xea, 0xf7, 0xf0,
      0xff, 0xff, 0xff}
     },
    {
     "per_true", // 9
     7,
     {0, 3957, 20330, 24440, 41268, 45002, 65535},
     {0x00, 0x00, 0x00,
      0x64, 0x00, 0x5c,
      0x00, 0x00, 0xeb,
      0x00, 0x5b, 0xe0,
      0x07, 0xb0, 0x00,
      0xb8, 0xbc, 0x00,
      0xdc, 0x00, 0x00}
     },
    {
     "pericolor", // 10
     6,
     {0, 8192, 16652, 33006, 48110, 65535},
     {0x00, 0x00, 0x00,
      0x71, 0x00, 0x76,
      0x00, 0x00, 0xf8,
      0x02, 0xf9, 0x00,
      0xfc, 0xf2, 0x00,
      0xfc, 0x00, 0x00}
     },
    {
     "rainbow", // 11
     10,
     {0, 1938, 18382, 28542, 35837, 42575, 52725, 55191, 65237, 65535},
     {0x05, 0x06, 0x4c,
      0x04, 0x06, 0x56,
      0x03, 0x77, 0xdb,
      0x85, 0xc1, 0x0e,
      0xe6, 0xd0, 0x07,
      0xf7, 0xbc, 0x0f,
      0xf9, 0x1b, 0x43,
      0xf7, 0x40, 0x64,
      0xfc, 0xdd, 0xc9,
      0xff, 0xff, 0xff}
     },
    {
     "rainbow0", // 12
     13,
     {0, 4170, 8056, 10916, 13583, 18365, 25515, 33006, 37917, 44588, 49014, 53615, 65535},
     {0x00, 0x00, 0x00,
      0x55, 0x00, 0x49,
      0x74, 0x00, 0x80,
      0x67, 0x00, 0x9c,
      0x3d, 0x00, 0xb0,
      0x00, 0x1e, 0xbc,
      0x00, 0x78, 0xa3,
      0x00, 0xbc, 0x4b,
      0x16, 0xd6, 0x00,
      0xa2, 0xe2, 0x00,
      0xdf, 0xe0, 0x00,
      0xfc, 0xbc, 0x00,
      0xfc, 0x00, 0x00}
     }
};

static uint8_t *pal = NULL;

//#define USE_FLOATS
#define INT_PREC 16

uint8_t *pal_init_lut(const uint8_t id, const uint8_t bpp)
{
    uint32_t pal_sz;
    uint32_t i;
    uint8_t c;
    uint8_t interval;
    uint16_t stop_begin = 0, stop_end = 0;
    uint32_t col_begin = 0, col_end = 0;
    uint8_t col_cur = 0;

#ifdef USE_FLOATS
    double fcol_cur = 0;
#else
    uint64_t lcol_cur = 0;
    uint8_t corr = 0;
#endif

    if ((bpp > PAL_16BPP) || (bpp < PAL_2BPP)) {
        return NULL;
    }

    pal_free();

    pal_sz = (1 << bpp);

    pal = (uint8_t *) calloc (pal_sz * 3, sizeof(uint8_t));
    if (pal == NULL) {
        errExit("allocating buffer");
    }

    // printf("%u allocated\n", pal_sz * 3);

    for (i=0; i<pal_sz; i++) {
        // find interval that contains i
        for (interval = 0; interval < pal_db[id].stop_cnt - 1; interval++) {
            stop_begin = pal_db[id].stop_offset[interval] >> (16 - bpp);
            stop_end = pal_db[id].stop_offset[interval + 1] >> (16 - bpp);
            if ((i >= stop_begin) && (i <= stop_end)) {
                break;
            }
        }

        if (stop_end == stop_begin) {
            fprintf(stderr, "invalid stop offsets in interval %d\n", interval);
            return NULL;
        }

        for (c = 0; c<3; c++) {
#ifdef USE_FLOATS
            col_begin = pal_db[id].rgb[interval * 3 + c];
            col_end = pal_db[id].rgb[(interval + 1) * 3 + c];
#else
            col_begin = pal_db[id].rgb[interval * 3 + c] << INT_PREC;
            col_end = pal_db[id].rgb[(interval + 1) * 3 + c] << INT_PREC;
#endif

            if (pal_db[id].rgb[interval * 3 + c] < pal_db[id].rgb[(interval + 1) * 3 + c]) {
#ifdef USE_FLOATS
                fcol_cur = 1.0 * (i - stop_begin) / (stop_end - stop_begin) * (col_end - col_begin) + col_begin;
                col_cur = fcol_cur + 0.5;
#else
                lcol_cur = ((uint64_t)(i - stop_begin) * (uint64_t)(col_end - col_begin)) / (stop_end - stop_begin) + col_begin;
                if (lcol_cur & (1 << (INT_PREC - 1))) {
                    corr = 1;
                } else {
                    corr = 0;
                }
                col_cur = (lcol_cur >> INT_PREC) + corr;
#endif
            } else if (pal_db[id].rgb[interval * 3 + c] > pal_db[id].rgb[(interval + 1) * 3 + c]) {
#ifdef USE_FLOATS
                fcol_cur = (1.0 - (1.0 * (i - stop_begin) / (stop_end - stop_begin))) * (col_begin - col_end) + col_end;
                col_cur = fcol_cur + 0.5;
#else
                lcol_cur = (col_begin - col_end) - ((uint64_t)(i - stop_begin) * (uint64_t)(col_begin - col_end)) / (stop_end - stop_begin) + col_end;
                if (lcol_cur & (1 << (INT_PREC - 1))) {
                    corr = 1;
                } else {
                    corr = 0;
                }
                col_cur = (lcol_cur >> INT_PREC) + corr;
#endif
            } else {
                col_cur = pal_db[id].rgb[interval * 3 + c];
            }
            pal[ i * 3 + c] = col_cur;
        }
    }

    return pal;
}

void pal_free(void)
{
    if (pal) {
        free(pal);
    }
    pal = NULL;
}

// generate image used by the scale
// use power of two dimensions for width/height
void pal_transfer(uint8_t *image, const uint8_t pal_id, const uint16_t width, const uint16_t height)
{
    float f;
    uint8_t bpp;
    uint8_t *pal_rgb;       ///< the palette in 32bit RGBA format
    int16_t x, y;       ///< counters
    uint8_t color[4];

    f = floor(log(height) / log(2.0)); // 1024 -> 10
    bpp = f;
    pal_rgb = pal_init_lut(pal_id, bpp);

    //printf("bpp %d\n", bpp);

    for (y = height; y > 0; y--) {
        memcpy(color, &(pal_rgb[(height - y - 1) * 3]), 3);
        color[3] = 255; // alpha channel
        for (x = width; x > 0; x--) {
#if 0
            if (x < 16) {
                f = 15.9375 * x + 0.5;
                color[3] = f;
            } else if (x < 64) {
                color[3] = 255;
            } else if (x < 80) {
                f = -15.9375 * x + 1275 + 0.5;
                color[3] = f;
#else
            if (x < 8) {
                f = 31.875 * x + 0.5;
                color[3] = f;
            } else if (x < 64) {
                color[3] = 255;
            } else if (x < 72) {
                f = -31.875 * x + 2295 + 0.5;
                color[3] = (uint8_t) f;
#endif
            } else {
                color[3] = 0;
            }
            memcpy(image + (((y - 1) * width + x - 1) * 4), color, 4);
        }
    }
}


uint8_t *pal_get_p(void)
{
    return pal;
}

