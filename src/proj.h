
#ifndef __PROJ_H__
#define __PROJ_H__

#ifdef __cplusplus
extern "C" {
#endif

#define          OPT_SET_NEW_MIN  0x01
#define          OPT_SET_NEW_MAX  0x02
#define     OPT_SET_NEW_DISTANCE  0x04
#define    OPT_SET_DISTANCE_COMP  0x08
#define   OPT_SET_NEW_EMISSIVITY  0x10

#define WINDOW_WIDTH 2000
#define WINDOW_HEIGHT 2000

struct th_custom_param {
    uint16_t flags;
    double t_min;
    double t_max;
    double distance;
    double emissivity;
};

typedef struct th_custom_param th_custom_param_t;

void print_buf(uint8_t * data, const uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
