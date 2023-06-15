#ifndef __FILE_PROPERTIES_H__
#define __FILE_PROPERTIES_H__

#ifdef __cplusplus
extern "C" {
#endif

struct fp_visibility {
    bool emissivity;
    bool distance;
    bool rh;
    bool alpha1;
    bool alpha2;
    bool beta1;
    bool beta2;
    bool planckR1;
    bool planckR2;
    bool planckB;
    bool planckF;
    bool planckO;
    bool atm_trans_X;
    bool atm_temp;
    bool refl_temp;
    bool iwt;
    bool iwtemp;
    bool raw_th_img_res;
    bool ir_timestamp;
    bool ir_fname;
    bool ir_comment;
    bool ir_min;
    bool ir_max;
    bool ir_avg;
    bool line_min;
    bool line_max;
    bool line_avg;
    bool camera_make;
    bool camera_model;
    bool byte_order;
};
typedef struct fp_visibility fp_visibility_t;

void file_properties(bool *p_open, th_db_t * db);
fp_visibility_t *file_properties_get_ptr(void);

#ifdef __cplusplus
}
#endif

#endif

