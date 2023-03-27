
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

// main_cli() flags
#define    SETUP_SIGHANDLER  0x1
#define   GENERATE_OUT_FILE  0x2

#ifdef __cplusplus
extern "C" {
#endif

struct th_getopt {
    char *in_file;
    char *out_file;
    uint8_t pal;
    uint8_t zoom_level;
    uint8_t zoom_interpolation;
    uint16_t flags;
    double t_min;
    double t_max;
    double distance;
    double emissivity;
    double atm_temp;        ///< atmospheric temperature in dC
    double rh;
};
typedef struct th_getopt th_getopt_t;

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
    uint8_t *combo;
};
typedef scale scale_t;

struct frontend {
    uint8_t actual_zoom;
    uint8_t return_state;
    uint16_t vp_width;
    uint16_t vp_height;
    uint32_t vp_texture;     ///< texture of the thermal image
    uint16_t si_width;
    uint16_t si_height;
    uint32_t si_texture;     ///< texture of the scale image
};
typedef struct frontend frontend_t;

struct profile {
    uint8_t do_refresh;
    uint8_t active;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
};
typedef profile profile_t;

#define STAGE_CNT 2

struct th_db {
    th_getopt_t p;      ///< parameters gotten via getopt() from the user
    struct stat sb;     ///< input file status struct
    tgram_t *in_th;     ///< input thermogram
    tgram_t *out_th;    ///< processed thermogram
    th_rgba_t rgba[STAGE_CNT];   ///< processed image - 0 is the original, at 1x zoom
    scale_t scale;      ///< scale for the processed thermogram
    frontend_t fe;      ///< textures of the images used by the GUI
    profile_t pr;       ///< profile line
    double *temp_arr;   ///< array containing actual temperatures for the processed image
};
typedef struct th_db th_db_t;

#define  ZOOM_INTERP_NEAREST  0
#define   ZOOM_INTERP_LINEAR  1
#define    ZOOM_INTERP_CUBIC  2


#define           STYLE_DARK  0
#define          STYLE_LIGHT  1
#define        STYLE_CLASSIC  2
#define            DEF_STYLE  2
#define   DEF_THUMBNAIL_SIZE  128
#define    DEF_THUMBNAIL_GEN  4     ///< number of files to analyze during each generated frame
#define          DEF_PALETTE  6
#define             DEF_ZOOM  1
#define      DEF_ZOOM_INTERP  ZOOM_INTERP_NEAREST

struct style {
    uint8_t theme; // 0 - classic dark, 1 - light
    uint32_t ovl_text_color;
    uint32_t ovl_highlight_color;
    uint32_t plot_line_color;
    //GFXfont font;
};
typedef style style_t;

struct global_preferences {
    uint8_t palette_default;
    uint16_t thumbnail_size;
    uint16_t thumbnail_gen_per_frame;
    uint8_t zoom_level;
    uint8_t zoom_interpolation;
    style_t style;
};
typedef struct global_preferences global_preferences_t;

uint8_t get_file_type(const char *in_file);
void print_buf(uint8_t * data, const uint16_t size);
void show_usage(void);
void show_version(void);
uint8_t parse_options(int argc, char *argv[], th_getopt_t * p);
int proj_main(th_db_t *db);
uint8_t localhost_is_le(void);
void generate_scale(scale_t *scale);
uint8_t get_min_max(tgram_t *th, double *t_min, double *t_max);

style_t *style_get_ptr(void);
global_preferences_t *gp_get_ptr(void);
th_db_t *db_get_ptr(void);

void style_set(uint8_t theme);
void style_init(void);

void gp_init(th_getopt_t *p);

#ifdef __cplusplus
}
#endif

#endif
