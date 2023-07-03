
#ifndef __PROJ_H__
#define __PROJ_H__

#include <sys/stat.h>
#include "thermogram.h"

#define          OPT_SET_NEW_MIN  0x01
#define          OPT_SET_NEW_MAX  0x02
#define     OPT_SET_NEW_DISTANCE  0x04
#define             OPT_SET_COMP  0x08
#define   OPT_SET_NEW_EMISSIVITY  0x10
#define           OPT_SET_NEW_AT  0x20
#define           OPT_SET_NEW_RT  0x40
#define           OPT_SET_NEW_RH  0x80
#define          OPT_SET_NEW_IWT  0x100
#define       OPT_SET_NEW_IWTEMP  0x200
#define           OPT_SET_NEW_WR  0x400
#define           OPT_SET_INTERP  0x800

#define                   FT_UNK  0

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

// profile_t defines
#define                PROFILE_CNT  4
#define         PROFILE_TYPE_POINT  0x1
#define          PROFILE_TYPE_LINE  0x2
#define   PROFILE_TYPE_LEVEL_SLICE  0x3

#define   PROFILE_REQ_VIEWPORT_INT  0x1
#define   PROFILE_REQ_VIEWPORT_RDY  0x2

// rgba index types
#define                  RGBA_ORIG  0x0
#define           RGBA_ORIG_ZOOMED  0x1
#define             RGBA_HIGHLIGHT  0x2
#define      RGBA_HIGHLIGHT_ZOOMED  0x3
#define                  STAGE_CNT  4

// main_cli() flags
#define           SETUP_SIGHANDLER  0x1
#define          GENERATE_OUT_FILE  0x2

// frontend flags
#define   TOOL_EXPORT_GOT_BASENAME  0x1
#define         HIGHLIGHT_LAYER_EN  0x20
#define HIGHLIGHT_LAYER_PREVIEW_EN  0x40

// db flags
#define  HIGHLIGHT_LAYER_GENERATED  0x1

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
    double distance;        ///< distance to object
    double emissivity;      ///< object emissivity
    double atm_temp;        ///< atmospheric temperature in dC
    double refl_temp;       ///< reflected temperature in dC
    double rh;              ///< relative humidity
    double iwt;             ///< infrared window transmission
    double iwtemp;          ///< infrared window temperature
    double wr;              ///< window reflectivity (0 if anti-reflective coating is present)
};
typedef struct th_getopt th_getopt_t;

struct th_rgba {
    uint16_t width;
    uint16_t height;
    uint8_t *data;
    uint8_t *overlay;
    uint8_t *base;
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
    uint32_t flags;
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
    uint8_t type;
    uint32_t flags;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    double t_min;
    double t_max;
    double prox_temp;
    uint16_t prox_pix;
    double res_t_mean;
};
typedef profile profile_t;


struct th_db {
    th_getopt_t p;      ///< parameters gotten via getopt() from the user
    struct stat sb;     ///< input file status struct
    tgram_t *in_th;     ///< input thermogram
    tgram_t *out_th;    ///< processed thermogram
    th_rgba_t rgba[STAGE_CNT];   ///< processed image - 0 is the original, at 1x zoom, 1 is original at > 1x zoom, 2 is highlight, 3 is highlight > 1x zoom
    th_rgba_t *rgba_vp; ///< pointer to the rgba struct that will be used as a texture in the GUI
    scale_t scale;      ///< scale for the processed thermogram
    frontend_t fe;      ///< textures of the images used by the GUI
    profile_t pr;       ///< profile point, line, etc
    uint32_t flags;     ///< thermogram-related flags
    double *temp_arr;   ///< array containing actual temperatures for the processed image
};
typedef struct th_db th_db_t;

#define  ZOOM_INTERP_NEAREST  0x0
#define   ZOOM_INTERP_REALSR  0x1

#define       ZOOM_DECREMENT  0x1
#define       ZOOM_INCREMENT  0x2
#define   ZOOM_FORCE_REFRESH  0x3

#define           STYLE_DARK  0
#define          STYLE_LIGHT  1
#define        STYLE_CLASSIC  2
#define            DEF_STYLE  2
#define       DEF_FONT_SCALE  1
#define   DEF_THUMBNAIL_SIZE  128
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
    uint8_t zoom_level;
    uint8_t zoom_interpolation;
    style_t style;
};
typedef struct global_preferences global_preferences_t;

uint16_t get_file_type(const char *in_file, uint16_t *type, uint16_t *subtype);
void print_buf(uint8_t * data, const uint16_t size);
void show_usage(void);
void show_version(void);
uint8_t parse_options(int argc, char *argv[], th_getopt_t * p);
int proj_main(th_db_t *db);
uint8_t localhost_is_le(void);
void generate_scale(th_db_t *db, style_t *style);
uint8_t get_min_max(tgram_t *th, double *t_min, double *t_max);
uint8_t get_avg(tgram_t *th, double *t_avg);

style_t *style_get_ptr(void);
global_preferences_t *gp_get_ptr(void);
th_db_t *db_get_ptr(void);

void style_set(const uint8_t theme, style_t *dst);
void style_init(void);

void gp_init(th_getopt_t *p);

void select_vp(th_db_t *db);
uint8_t set_zoom(th_db_t * db, const uint8_t flags);
uint8_t generate_highlight(th_db_t *db);
uint8_t combine_highlight(th_db_t *db);
uint8_t refresh_highlight_overlay(th_db_t *db, const uint8_t index, const uint8_t pal_id);

#ifdef __cplusplus
}
#endif

#endif
