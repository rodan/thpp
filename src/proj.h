
#ifndef __PROJ_H__
#define __PROJ_H__

#include <sys/stat.h>
#include "thermogram.h"

#define          OPT_SET_NEW_MIN  0x01
#define          OPT_SET_NEW_MAX  0x02
#define     OPT_SET_NEW_DISTANCE  0x04
#define    OPT_SET_DISTANCE_COMP  0x08
#define   OPT_SET_NEW_EMISSIVITY  0x10
#define           OPT_SET_NEW_AT  0x20
#define           OPT_SET_NEW_RH  0x40

#define                   FT_UNK  0
#define                   FT_DTV  1
#define                  FT_RJPG  2

#define                   RET_OK  0
#define    RET_OK_REFRESH_NEEDED  0x1
#define                  RET_RST  0x2
#define              RET_FAILURE  0x4
#define                 RET_EXIT  0x8

#define                   RJPG_K  273.15
#define             WINDOW_WIDTH  1300
#define            WINDOW_HEIGHT  1200

#define              SCALE_WIDTH  128
#define             SCALE_HEIGHT  1024

#ifdef __cplusplus
extern "C" {
#endif

struct th_custom_param {
    char *in_file;
    char *out_file;
    uint8_t pal;
    uint8_t zoom;
    uint16_t flags;
    double t_min;
    double t_max;
    double distance;
    double emissivity;
    double atm_temp;        ///< atmospheric temperature in dC
    double rh;
};
typedef struct th_custom_param th_custom_param_t;

struct th_rgba {
    uint16_t width;
    uint16_t height;
    uint8_t *data;
    uint8_t *overlay;
};
typedef th_rgba th_rgba_t;

struct scale {
    uint8_t pal_id;
    uint16_t width;
    uint16_t height;
    double t_min;
    double t_max;
    uint8_t *data;
    uint8_t *overlay;
};
typedef scale scale_t;

struct th_db {
    th_custom_param_t p;
    struct stat sb;
    tgram_t *in_th;
    tgram_t *out_th;
    th_rgba_t rgba;
    scale_t scale;
    double *temp_arr;
};

struct idb_t {
    uint8_t actual_zoom;
    uint8_t return_state;
    unsigned int vp_width = 0;
    unsigned int vp_height = 0;
    unsigned int vp_texture = 0;    ///< texture of the thermal image
    unsigned int si_width = 0;
    unsigned int si_height = 0;
    unsigned int si_texture = 0;     ///< texture of the scale image
};

typedef struct th_db th_db_t;

uint8_t get_file_type(const char *in_file);
void print_buf(uint8_t * data, const uint16_t size);
void show_usage(void);
void show_version(void);
uint8_t parse_options(int argc, char *argv[], th_custom_param_t * p);
int proj_main(th_db_t *db);
uint8_t localhost_is_le(void);
void generate_scale(scale_t *scale);
uint8_t get_min_max(tgram_t *th, double *t_min, double *t_max);

#ifdef __cplusplus
}
#endif

#endif
