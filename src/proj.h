
#ifndef __PROJ_H__
#define __PROJ_H__

#include "thermogram.h"

#define          OPT_SET_NEW_MIN  0x01
#define          OPT_SET_NEW_MAX  0x02
#define     OPT_SET_NEW_DISTANCE  0x04
#define    OPT_SET_DISTANCE_COMP  0x08
#define   OPT_SET_NEW_EMISSIVITY  0x10

#define                   FT_UNK  0
#define                   FT_DTV  1
#define                  FT_RJPG  2

#define    RET_OK_REFRESH_NEEDED  1
#define                   RET_OK  0
#define              RET_FAILURE  -1

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
};

typedef struct th_custom_param th_custom_param_t;

struct th_db {
    th_custom_param_t p;
    tgram_t *in_th;
    tgram_t *out_th;
};

typedef struct th_db th_db_t;

uint8_t get_file_type(const char *in_file);
void print_buf(uint8_t * data, const uint16_t size);
void show_usage(void);
void show_version(void);
uint8_t parse_options(int argc, char *argv[], th_custom_param_t * p);
int proj_main(th_db_t *db);

#ifdef __cplusplus
}
#endif

#endif
