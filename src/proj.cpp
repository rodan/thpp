
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <getopt.h>
#include "tlpi_hdr.h"
#include "proj.h"
#include "palette.h"
#include "version.h"
#include "graphics.h"
#include "dtv.h"
#include "rjpg.h"

#define BUF_SIZE  32

th_db_t db;
global_preferences_t gp;

void show_usage(void)
{
    fprintf(stdout, "Usage:\n");
    fprintf(stdout,
            " thpp  --input INPUT_FILE --output OUTPUT_FILE [--palette INT] [--zoom INT]\n");
    fprintf(stdout,
            "            [--min FLOAT] [--max FLOAT] [--distance FLOAT] [--emissivity FLOAT]\n");
    fprintf(stdout, "            [-h] [-v]\n");
}

void show_version(void)
{
    fprintf(stdout, " thpp %d.%d\nbuild %d commit %d\n", VER_MAJOR, VER_MINOR, BUILD, COMMIT);
}

uint8_t parse_options(int argc, char *argv[], th_getopt_t * p)
{
    int opt;
    int option_index = 0;
    struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"palette", required_argument, 0, 'p'},
        {"zoom", required_argument, 0, 'z'},
        {"min", required_argument, 0, 'l'},
        {"max", required_argument, 0, 'a'},
        {"distance", required_argument, 0, 'd'},
        {"emissivity", required_argument, 0, 'e'},
        {"rh", required_argument, 0, 'r'},
        {"at", required_argument, 0, 't'},
        {"rtemp", required_argument, 0, OPT_SET_NEW_RT},
        {"atemp", required_argument, 0, OPT_SET_NEW_AT},
        {"iwtemp", required_argument, 0, OPT_SET_NEW_IWTEMP},
        {"interp", required_argument, 0, OPT_SET_INTERP},
        {"iwt", required_argument, 0, OPT_SET_NEW_IWT},
        {"wr", required_argument, 0, OPT_SET_NEW_WR},
        {"distcomp", no_argument, 0, 'k'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    memset(p, 0, sizeof(th_getopt_t));
    p->pal = DEF_PALETTE;
    p->zoom_level = DEF_ZOOM;
    p->zoom_interpolation = DEF_ZOOM_INTERP;

    while ((opt =
            getopt_long(argc, argv, "i:o:p:z:l:a:d:e:r:t:vkh", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'i':
            p->in_file = (char *) calloc(strlen(optarg) + 1, sizeof (char));
            memcpy(p->in_file, optarg, strlen(optarg) + 1);
            //p->in_file = optarg; // we can't do this since in_file needs to be free-able and reallocated later
            break;
        case 'o':
            p->out_file = optarg;
            break;
        case 'p':
            p->pal = atoi(optarg);
            break;
        case 'z':
            p->zoom_level = atoi(optarg);
            break;
        case 'l':
            p->flags |= OPT_SET_NEW_MIN;
            p->t_min = strtod(optarg, NULL);
            break;
        case 'a':
            p->flags |= OPT_SET_NEW_MAX;
            p->t_max = strtod(optarg, NULL);
            break;
        case 'd':
            p->flags |= OPT_SET_NEW_DISTANCE;
            p->distance = strtod(optarg, NULL);
            break;
        case 'k':
            p->flags |= OPT_SET_COMP;
            break;
        case 'e':
            p->flags |= OPT_SET_NEW_EMISSIVITY;
            p->emissivity = strtod(optarg, NULL);
            break;
        case 't':
            p->flags |= OPT_SET_NEW_AT;
            p->atm_temp = strtod(optarg, NULL);
            break;
        case 'r':
            p->flags |= OPT_SET_NEW_RH;
            p->rh = strtod(optarg, NULL);
            break;
        case OPT_SET_NEW_IWT:
            p->flags |= OPT_SET_NEW_IWT;
            p->iwt = strtod(optarg, NULL);
            break;
        case OPT_SET_NEW_IWTEMP:
            p->flags |= OPT_SET_NEW_IWTEMP;
            p->iwtemp = strtod(optarg, NULL);
            break;
        case OPT_SET_NEW_WR:
            p->flags |= OPT_SET_NEW_WR;
            p->wr = strtod(optarg, NULL);
            break;
        case OPT_SET_INTERP:
            p->zoom_interpolation = atoi(optarg);
            break;
        case OPT_SET_NEW_RT:
            p->flags |= OPT_SET_NEW_RT;
            p->refl_temp = strtod(optarg, NULL);
            break;
        case 'v':
            show_version();
            exit(0);
            break;
        case 'h':
            show_usage();
            exit(0);
            break;
        default:
            break;
        }
    }

#if 0
    if ((p->in_file == NULL) || (p->out_file == NULL)) {
        fprintf(stderr, "Error: provide input and output files\n");
        show_usage();
        exit(1);
    }
#endif

    return EXIT_SUCCESS;
}

uint16_t get_file_type(const char *in_file, uint16_t *type, uint16_t *subtype)
{
    int fd;
    uint8_t *buf;
    uint8_t ret = FT_UNK;
    static const uint8_t sig_exif[4] = { 0x45, 0x78, 0x69, 0x66 };      // appears at file offset 0x18 or 0x06
    static const uint8_t sig_dtv_v0[1] = { 0x00 };   // appears at offset 0x1
    static const uint8_t sig_dtv_v2[1] = { 0x02 };   // appears at offset 0x1
    static const uint8_t sig_dtv_v3[1] = { 0x03 };   // appears at offset 0x1
    static const uint8_t sig_flir[4] = {0x46, 0x4c, 0x49, 0x52}; // sometimes at offset 0x18
    static const uint8_t sig_jfif[4] = {0x4a, 0x46, 0x49, 0x46}; // at offset 0x6

    // read input file
    if ((fd = open(in_file, O_RDONLY)) < 0) {
        errExit("opening input file");
    }

    buf = (uint8_t *) calloc(BUF_SIZE, sizeof(uint8_t));
    if (buf == NULL) {
        errExit("allocating buffer");
        exit(EXIT_FAILURE);
    }

    if (read(fd, buf, BUF_SIZE) != BUF_SIZE) {
        fprintf(stderr, "error while reading input file\n");
        exit(EXIT_FAILURE);
    }

    if (memcmp(buf + 1, sig_dtv_v2, 1) == 0) {
        ret = TH_IRTIS_DTV;
        if (type) {
            *type = TH_IRTIS_DTV;
        }
        if (subtype) {
            *subtype = TH_DTV_VER2;
        }
    } else if (memcmp(buf + 1, sig_dtv_v3, 1) == 0) {
        ret = TH_IRTIS_DTV;
        if (type) {
            *type = TH_IRTIS_DTV;
        }
        if (subtype) {
            *subtype = TH_DTV_VER3;
        }
    } else if (memcmp(buf + 1, sig_dtv_v0, 1) == 0) {
        ret = TH_IRTIS_DTV;
        if (type) {
            *type = TH_IRTIS_DTV;
        }
        if (subtype) {
            *subtype = TH_DTV_VER2;
        }
    } else if ((memcmp(buf + 0x18, sig_exif, 4) == 0) || (memcmp(buf + 0x6, sig_exif, 4) == 0)) {
        // having an exif inside the jpeg file is a requirement
        ret = TH_FLIR_RJPG;
        if (type) {
            *type = TH_FLIR_RJPG;
        }
        if (subtype) {
            *subtype = TH_UNSET;
        }
    } else if (memcmp(buf + 0x18, sig_flir, 4) == 0) {
        ret = TH_FLIR_RJPG;
        if (type) {
            *type = TH_FLIR_RJPG;
        }
        if (subtype) {
            *subtype = TH_UNSET;
        }
    } else if (memcmp(buf + 0x6, sig_jfif, 4) == 0) {
        // is it at least a jpeg file? let's hope for the best
        // should be the last 'else if' rule
        ret = TH_FLIR_RJPG;
        if (type) {
            *type = TH_FLIR_RJPG;
        }
        if (subtype) {
            *subtype = TH_UNSET;
        }
    }

    close(fd);
    free(buf);
    return ret;
}

// return true if current system is little endian
uint8_t localhost_is_le(void)
{
    uint32_t n = 1;
    return (*(char *)&n == 1);
}

uint8_t get_min_max(tgram_t *th, double *t_min, double *t_max)
{
    if (th == NULL) {
        return EXIT_FAILURE;
    }

    switch (th->type) {
        case TH_FLIR_RJPG:
            *t_min = th->head.rjpg->t_min;
            *t_max = th->head.rjpg->t_max;
            break;
        case TH_IRTIS_DTV:
            if (th->subtype == TH_DTV_VER2) {
                *t_min = th->head.dtv->tsc[1];
                *t_max = th->head.dtv->tsc[1] + 256.0 * th->head.dtv->tsc[0];
            } else if (th->subtype == TH_DTV_VER3) {
                *t_min = th->head.dtv->tsc[1];
                *t_max = th->head.dtv->tsc[1] + 65536.0 * th->head.dtv->tsc[0];
            }
            break;
        default:
            *t_min = 0.0;
            *t_max = 0.0;
            return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

uint8_t get_avg(tgram_t *th, double *t_avg)
{
    if (th == NULL) {
        return EXIT_FAILURE;
    }

    switch (th->type) {
        case TH_FLIR_RJPG:
            *t_avg = th->head.rjpg->t_avg;
            break;
        case TH_IRTIS_DTV:
            *t_avg = 0.0;
            break;
        default:
            *t_avg = 0.0;
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void draw_scale_overlay(scale_t *scale, style_t *style, const double major, const double minor, const uint8_t precission)
{
    double delta = scale->t_max - scale->t_min;
    double q = scale->height / delta; ///<   pixel = q * temperature => q = pixel / temperature
    int16_t i;
    char text[12];
    double t = 950;
    canvas_t c;

    // draw grid onto the overlay
    memset(&c, 0, sizeof(canvas_t));

    c.width = scale->width;
    c.height = scale->height;
    c.data = (uint32_t *) scale->overlay;

    // major tick marks
    for (i = (int16_t) floor (scale->t_max / major); i > (int16_t) floor(scale->t_min / major); i--) {
        t = q * (scale->t_max - (i * major)) + 0.5;
        switch (precission){
            case 1:
                snprintf(text, 11, "%3.01f", i * major);
                draw_text(&c, 80, t - 22, &text[0], style->ovl_text_color, 2);
                break;
            case 2:
                snprintf(text, 11, "%3.02f", i * major);
                draw_text(&c, 80, t - 22, &text[0], style->ovl_text_color, 2);
                break;
            default:
                snprintf(text, 11, "%3.0f", i * major);
                draw_text(&c, 80, t - 22, &text[0], style->ovl_text_color, 2);
                break;
        } 
        draw_major_tick(&c, (uint16_t) t, style->ovl_highlight_color);
    }

    snprintf(text, 11, "dC");
    draw_text(&c, 30, t - 22, text, style->ovl_text_color, 2);

    // minor tick marks
    for (i = (int16_t) scale->t_max / minor; i > (int16_t) scale->t_min / minor; i--) {
        t = q * (scale->t_max - (i * minor)) + 0.5;
        draw_minor_tick(&c, (int16_t) t, style->ovl_highlight_color);
    }
}

void generate_scale(th_db_t * db, style_t *style)
{
    uint32_t j;
    double delta;
    scale_t *scale = &db->scale;

    db->scale.width = SCALE_WIDTH;
    db->scale.height = SCALE_HEIGHT;
    db->scale.pal_id = db->p.pal;

    if (db->p.flags & (OPT_SET_NEW_MIN | OPT_SET_NEW_MAX)) {
        db->scale.t_min = db->p.t_min;
        db->scale.t_max = db->p.t_max;
    } else {
        get_min_max(db->out_th, &db->scale.t_min, &db->scale.t_max);
    }

    delta = scale->t_max - scale->t_min;

    if (scale->data) {
        free(scale->data);
    }

    scale->data = (uint8_t *) calloc (scale->width * scale->height * 4, sizeof(uint8_t));
    if (scale->data == NULL) {
        errExit("allocating buffer");
    }

    if (scale->overlay) {
        free(scale->overlay);
    }

    scale->overlay = (uint8_t *) calloc (scale->width * scale->height * 4, sizeof(uint8_t));
    if (scale->overlay == NULL) {
        errExit("allocating buffer");
    }

    if (scale->combo) {
        free(scale->combo);
    }
    scale->combo = (uint8_t *) calloc (scale->width * scale->height * 4, sizeof(uint8_t));
    if (scale->combo == NULL) {
        errExit("allocating buffer");
    }

    pal_transfer(scale->data, scale->pal_id, scale->width, scale->height);

    if (delta < 1.0) {
        draw_scale_overlay(scale, style, 0.1, 0.05, 1);
    } else if (delta < 5.0) {
        draw_scale_overlay(scale, style, 0.5, 0.5, 1); // 4-10 10
    } else if (delta < 10.0) {
        draw_scale_overlay(scale, style, 1.0, 0.5, 0); // 5-10 20
    } else if (delta < 20.0) {
        draw_scale_overlay(scale, style, 2.0, 1.0, 0); // 5-10 20
    } else if (delta < 35.0) {
        draw_scale_overlay(scale, style, 5.0, 1.0, 0); // 7-10 35
    } else if (delta < 50.0) {
        draw_scale_overlay(scale, style, 5.0, 5.0, 0); // 3.5-10 10
    } else if (delta < 100) {
        draw_scale_overlay(scale, style, 10.0, 5.0, 0); // 5-10 20
    } else if (delta < 200) {
        draw_scale_overlay(scale, style, 20.0, 10.0, 0); // 5-10 20
    } else {
        draw_scale_overlay(scale, style, 50.0, 10.0, 0);
    }

    // combine the scale and the overlay
    uint32_t *combo_ptr = (uint32_t *) scale->combo;
    uint32_t *overlay_ptr = (uint32_t *) scale->overlay;
    uint32_t *data_ptr = (uint32_t *) scale->data;

    for (j = 0; j < scale->width * scale->height; j++) {
        if (*(overlay_ptr + j)) {
            //inv = *(data_ptr + j);
            *(combo_ptr + j) = highlight_color(*(data_ptr + j), *(overlay_ptr + j));
            //printf("%08x %08x\n", inv, *(combo_ptr + j));
        } else {
            *(combo_ptr + j) = *(data_ptr + j);
        }
    }

    db->fe.si_width = SCALE_WIDTH;
    db->fe.si_height = SCALE_HEIGHT;
}

void style_init(void)
{
}

void gp_init(th_getopt_t *p)
{
    global_preferences_t *pref = gp_get_ptr();

    memset(pref, 0, sizeof(global_preferences_t));
    style_set(STYLE_DARK, &pref->style);
    pref->thumbnail_size = DEF_THUMBNAIL_SIZE;
    pref->palette_default = p->pal;
    pref->zoom_level = p->zoom_level;
    pref->zoom_interpolation = p->zoom_interpolation;
}

void style_set(const uint8_t theme, style_t *dst)
{
    //global_preferences_t *pref = gp_get_ptr();
    // colors are 0xAABBGGRR

    switch (theme) {
        case STYLE_CLASSIC:
        case STYLE_DARK:
            dst->theme = theme;
            dst->ovl_text_color = 0xffcccccc;
            dst->ovl_highlight_color = 0xffdddddd;
            dst->plot_line_color = 0xffff00ff;
            break;
        case STYLE_LIGHT:
            dst->theme = theme;
            dst->ovl_text_color = 0xff111111;
            dst->ovl_highlight_color = 0xff222222;
            dst->plot_line_color = 0xff111111;
            break;
    } 
}

style_t *style_get_ptr(void)
{
    return &gp.style;
}

global_preferences_t *gp_get_ptr(void)
{
    return &gp;
}

th_db_t *db_get_ptr(void)
{
    return &db;
}

uint8_t combine_highlight(th_db_t *db)
{
    uint32_t i;
    uint32_t *overlay = (uint32_t *) db->rgba[RGBA_HIGHLIGHT].overlay;
    uint32_t *base = (uint32_t *) db->rgba[RGBA_HIGHLIGHT].base;
    uint32_t *data = (uint32_t *) db->rgba[RGBA_HIGHLIGHT].data;

    for (i=0; i<db->rgba[RGBA_HIGHLIGHT].width * db->rgba[RGBA_HIGHLIGHT].height; i++) {
        if (* (overlay + i)) {
            *(data + i) = *( overlay + i);
        } else {
            *(data + i) = *( base + i);
        }
    }

    return EXIT_SUCCESS;
}

uint8_t refresh_highlight_overlay(th_db_t *db, const uint8_t index, const uint8_t pal_id)
{
    uint8_t ret = EXIT_SUCCESS;
    uint16_t i, j;
    uint16_t width, height;
    uint8_t *pal_rgb;
    double cur_temp;
    uint32_t loc;
    int32_t dist;
    int32_t dist_lim = db->pr.prox_pix * db->pr.prox_pix;
    double accu = 0;
    uint32_t pixel_cnt = 0;

    width = db->rgba[RGBA_HIGHLIGHT].width;
    height = db->rgba[RGBA_HIGHLIGHT].height;

    if ((db->in_th->type == TH_FLIR_RJPG) || 
         ( (db->in_th->type == TH_IRTIS_DTV) && (db->in_th->subtype == TH_DTV_VER3))) {
        pal_rgb = pal_init_lut(pal_id, PAL_16BPP);
    } else {
        pal_rgb = pal_init_lut(pal_id, PAL_8BPP);
    }

    if (pal_rgb == NULL) {
        fprintf(stderr, "palette generation error\n");
        exit(EXIT_FAILURE);
    }

    for (i=0; i<width; i++ ) {
        for (j=0; j<height; j++) {
            loc = width*j + i;
            dist = (db->pr.x1 - i) * (db->pr.x1 - i) + (db->pr.y1 - j) * (db->pr.y1 - j);
            cur_temp = db->temp_arr[loc];
            if ((cur_temp > db->pr.t_min) && (cur_temp < db->pr.t_max) && (dist < dist_lim) ) {
                if ((db->in_th->type == TH_FLIR_RJPG) || 
                     ( (db->in_th->type == TH_IRTIS_DTV) && (db->in_th->subtype == TH_DTV_VER3))) {
                    memcpy(db->rgba[RGBA_HIGHLIGHT].overlay + (loc * 4), &(pal_rgb[db->out_th->framew[loc] * 3]), 3);
                } else {
                    memcpy(db->rgba[RGBA_HIGHLIGHT].overlay + (loc * 4), &(pal_rgb[db->out_th->frame[loc] * 3]), 3);
                }
                db->rgba[RGBA_HIGHLIGHT].overlay[loc * 4 + 3] = 255; // alpha channel
                pixel_cnt++;
                accu += cur_temp;
            }
        }
    }

    db->pr.res_t_mean = accu / (double) pixel_cnt;

    return ret;
}

uint8_t generate_highlight(th_db_t *db)
{
    global_preferences_t *pref = gp_get_ptr();
    uint8_t ret = EXIT_FAILURE;

    if ((db->in_th == NULL) || (db->out_th == NULL)) {
        return EXIT_FAILURE;
    }

    if (db->rgba[RGBA_HIGHLIGHT].data) {
        free(db->rgba[RGBA_HIGHLIGHT].data);
    }
    db->rgba[RGBA_HIGHLIGHT].data = (uint8_t *) calloc(db->rgba[RGBA_ORIG].width * db->rgba[RGBA_ORIG].height * 4, 1);
    if (db->rgba[RGBA_HIGHLIGHT].data == NULL) {
        errExit("allocating buffer");
    }
    if (db->rgba[RGBA_HIGHLIGHT].overlay) {
        free(db->rgba[RGBA_HIGHLIGHT].overlay);
    }
    db->rgba[RGBA_HIGHLIGHT].overlay = (uint8_t *) calloc(db->rgba[RGBA_ORIG].width * db->rgba[RGBA_ORIG].height * 4, 1);
    if (db->rgba[RGBA_HIGHLIGHT].overlay == NULL) {
        errExit("allocating buffer");
    }
    if (db->rgba[RGBA_HIGHLIGHT].base) {
        free(db->rgba[RGBA_HIGHLIGHT].base);
    }
    db->rgba[RGBA_HIGHLIGHT].base = (uint8_t *) calloc(db->rgba[RGBA_ORIG].width * db->rgba[RGBA_ORIG].height * 4, 1);
    if (db->rgba[RGBA_HIGHLIGHT].base == NULL) {
        errExit("allocating buffer");
    }

    db->rgba[RGBA_HIGHLIGHT].width = db->rgba[RGBA_ORIG].width;
    db->rgba[RGBA_HIGHLIGHT].height = db->rgba[RGBA_ORIG].height;

    if (db->in_th->type == TH_IRTIS_DTV) {
        ret = dtv_transfer(db->out_th, db->rgba[RGBA_HIGHLIGHT].base, PAL_GREY);
    } else if (db->in_th->type == TH_FLIR_RJPG) {
        ret = rjpg_transfer(db->out_th, db->rgba[RGBA_HIGHLIGHT].base, PAL_GREY);
    }

    combine_highlight(db);

    if (ret != EXIT_SUCCESS) {
        return ret;
    }

    db->flags |= HIGHLIGHT_LAYER_GENERATED;

    if (pref->zoom_level > 1) {
        ret = image_zoom(&db->rgba[RGBA_HIGHLIGHT_ZOOMED], &db->rgba[RGBA_HIGHLIGHT], db->p.zoom_level, db->p.zoom_interpolation);
        if (ret != EXIT_SUCCESS) {
            return ret;
        }
    }

    return EXIT_SUCCESS;
}

void select_vp(th_db_t *db)
{
    global_preferences_t *pref = gp_get_ptr();

    if (pref->zoom_level == 1) {
        if ((db->flags & HIGHLIGHT_LAYER_GENERATED) && 
            (db->fe.flags & HIGHLIGHT_LAYER_EN) && 
            (db->fe.flags & HIGHLIGHT_LAYER_PREVIEW_EN)) {

            db->rgba_vp = &db->rgba[RGBA_HIGHLIGHT];
        } else {
            db->rgba_vp = &db->rgba[RGBA_ORIG];
        }
    } else {
        if ((db->flags & HIGHLIGHT_LAYER_GENERATED) && 
            (db->fe.flags & HIGHLIGHT_LAYER_EN) && 
            (db->fe.flags & HIGHLIGHT_LAYER_PREVIEW_EN)) {
            db->rgba_vp = &db->rgba[RGBA_HIGHLIGHT_ZOOMED];
        } else {
            db->rgba_vp = &db->rgba[RGBA_ORIG_ZOOMED];
        }
    }
}

// returns 1 if image needs to be refreshed
uint8_t set_zoom(th_db_t * db, const uint8_t flags)
{
    global_preferences_t *pref = gp_get_ptr();
    uint8_t initial_zoom = pref->zoom_level;

    //printf("set_zoom db %d pref %d flags %d\n", db->p.zoom_level, pref->zoom_level, flags);

    switch (flags) {
        case ZOOM_DECREMENT:
            if (pref->zoom_level > 1) {
                pref->zoom_level--;
                db->p.zoom_level = pref->zoom_level;

                if (pref->zoom_interpolation == ZOOM_INTERP_REALSR) {
                    if (pref->zoom_level > 4) {
                        pref->zoom_level = 4;
                        db->p.zoom_level = 4;
                    } else if (pref->zoom_level < 4) {
                        pref->zoom_level = 1;
                        db->p.zoom_level = 1;
                    }
                }

                if (pref->zoom_level > 1){
                    if (pref->zoom_level != initial_zoom) {
                        image_zoom(&db->rgba[RGBA_ORIG_ZOOMED], &db->rgba[RGBA_ORIG], pref->zoom_level, pref->zoom_interpolation);
                        if (db->flags & HIGHLIGHT_LAYER_GENERATED) {
                            image_zoom(&db->rgba[RGBA_HIGHLIGHT_ZOOMED], &db->rgba[RGBA_HIGHLIGHT], pref->zoom_level, pref->zoom_interpolation);
                        }
                    }
                }
            }
            break;
        case ZOOM_INCREMENT:
            if (pref->zoom_level < 16) {
                pref->zoom_level++;
                db->p.zoom_level = pref->zoom_level;

                if (pref->zoom_interpolation == ZOOM_INTERP_REALSR) {
                    pref->zoom_level = 4;
                    db->p.zoom_level = 4;
                }
                if (pref->zoom_level != initial_zoom) {
                    image_zoom(&db->rgba[RGBA_ORIG_ZOOMED], &db->rgba[RGBA_ORIG], pref->zoom_level, pref->zoom_interpolation);
                    if (db->flags & HIGHLIGHT_LAYER_GENERATED) {
                        image_zoom(&db->rgba[RGBA_HIGHLIGHT_ZOOMED], &db->rgba[RGBA_HIGHLIGHT], pref->zoom_level, pref->zoom_interpolation);
                    }
                }
            }
            break;
        case ZOOM_FORCE_REFRESH:
            if (pref->zoom_level > 1){
                image_zoom(&db->rgba[RGBA_ORIG_ZOOMED], &db->rgba[RGBA_ORIG], pref->zoom_level, pref->zoom_interpolation);
                if (db->flags & HIGHLIGHT_LAYER_GENERATED) {
                    image_zoom(&db->rgba[RGBA_HIGHLIGHT_ZOOMED], &db->rgba[RGBA_HIGHLIGHT], pref->zoom_level, pref->zoom_interpolation);
                }
            }
        default:
            break;
    }

    select_vp(db);

    if (pref->zoom_level != initial_zoom) {
        return 1;
    }

    return 0;
}

void print_buf(uint8_t * data, const uint16_t size)
{
    uint16_t bytes_remaining = size;
    uint16_t bytes_to_be_printed, bytes_printed = 0;
    uint16_t i;

    while (bytes_remaining > 0) {

        if (bytes_remaining > 16) {
            bytes_to_be_printed = 16;
        } else {
            bytes_to_be_printed = bytes_remaining;
        }

        printf("%u: ", bytes_printed);

        for (i = 0; i < bytes_to_be_printed; i++) {
            printf("%02x", data[bytes_printed + i]);
            if (i & 0x1) {
                printf(" ");
            }
        }

        printf("\n");
        bytes_printed += bytes_to_be_printed;
        bytes_remaining -= bytes_to_be_printed;
    }
}
