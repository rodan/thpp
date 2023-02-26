
#ifndef __PROCESSING_H__
#define __PROCESSING_H__

struct proc_limits {
    uint8_t umin;
    uint8_t umax;
    uint8_t uavg;
    float fmin;
    float fmax;
    float favg;
};

typedef struct proc_limits proc_limits_t; 

#ifdef __cplusplus
extern "C" {
#endif

uint8_t proc_get_limits(tgram_t *th, proc_limits_t *data);

#ifdef __cplusplus
}
#endif

#endif
