#ifndef __RJPG_HEADER_H__
#define __RJPG_HEADER_H__

struct rjpg_header {
    uint16_t raw_th_img_width;               ///< number of horizontal pixels
    uint16_t raw_th_img_height;              ///< number of vertical pixels
    float t_min;
    float t_res;
    uint32_t raw_th_img_sz;
    uint8_t *raw_th_img;
};

typedef struct rjpg_header rjpg_header_t;

#endif

