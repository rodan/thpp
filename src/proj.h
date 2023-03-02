
#ifndef __PROJ_H__
#define __PROJ_H__

#ifdef __cplusplus
extern "C" {
#endif

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
