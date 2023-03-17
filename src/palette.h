#ifndef __PALETTE_H__
#define __PALETTE_H__

#define     PAL_2BPP  2
#define     PAL_4BPP  4
#define     PAL_8BPP  8
#define    PAL_10BPP  10
#define    PAL_12BPP  12
#define    PAL_14BPP  14
#define    PAL_16BPP  16

#define       PAL_256  0
#define     PAL_COLOR  1
#define      PAL_GREY  2
#define   PAL_HMETAL0  3
#define   PAL_HMETAL1  4
#define   PAL_HMETAL2  5
#define  PAL_HOTBLUE1  6
#define  PAL_HOTBLUE2  7
#define      PAL_IRON  8
#define  PAL_PER_TRUE  9
#define PAL_PERICOLOR  10
#define   PAL_RAINBOW  11
#define  PAL_RAINBOW0  12

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *pal_init_lut(const uint8_t palette_id, const uint8_t bits_per_pixel);
void pal_free(void);
void pal_transfer(uint8_t *image, const uint8_t pal_id, const uint16_t width, const uint16_t height);
uint8_t *pal_get_p(void);

#ifdef __cplusplus
}
#endif

#endif
