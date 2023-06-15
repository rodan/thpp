#ifndef __RJPG_HEADER_H__
#define __RJPG_HEADER_H__

#define  TAG_SZ_MAX  32

struct rjpg_header {
    double emissivity;
    double distance;
    double rh;
    double alpha1;
    double alpha2;
    double beta1;
    double beta2;
    double planckR1;
    double planckR2;
    double planckB;
    double planckF;
    double planckO;
    double atm_trans_X;
    double atm_temp;
    double refl_temp;
    uint16_t raw_th_img_width;               ///< number of horizontal pixels
    uint16_t raw_th_img_height;              ///< number of vertical pixels
    double t_min;
    double t_res;
    double t_max;
    double t_avg;
    double iwt;
    double iwtemp;
    double wr;
    uint32_t raw_th_img_sz;
    uint8_t *raw_th_img;
    uint8_t byte_order;
    char camera_make[TAG_SZ_MAX];
    char camera_model[TAG_SZ_MAX];
    char create_ts[TAG_SZ_MAX];
};

typedef struct rjpg_header rjpg_header_t;

#endif

