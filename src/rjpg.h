
#ifndef __RJPG_H__
#define __RJPG_H__

struct rjpg_header {
    uint16_t raw_th_img_width;               ///< number of horizontal pixels
    uint16_t raw_th_img_height;              ///< number of vertical pixels
    float t_min;
    float t_res;
    uint32_t raw_th_img_sz;
    uint8_t *raw_th_img;
} __attribute__((packed));

typedef struct rjpg_header rjpg_t;

struct tgram_rjpg {
    rjpg_t head;
    uint8_t *frame;
};

typedef struct tgram_rjpg tgram_rjpg_t;

#ifdef __cplusplus
extern "C" {
#endif

uint8_t rjpg_open(tgram_rjpg_t *thermo, char *dtv_file);
void rjpg_close(tgram_rjpg_t *thermo);
uint8_t rjpg_transfer(const tgram_rjpg_t *thermo, uint8_t *image, const uint8_t pal, const uint8_t zoom);
uint8_t rjpg_rescale(tgram_rjpg_t *dst_thermo, const tgram_rjpg_t *src_thermo, const float new_min, const float new_max);

#ifdef __cplusplus
}
#endif

#endif
