
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>

//#include "sdl_vis.h"
#include "lodepng.h"
#include "tlpi_hdr.h"
#include "thermogram.h"
#include "proj.h"
#include "dtv.h"
#include "rjpg.h"
#include "palette.h"
#include "opengl_helper.h"
#include "version.h"
#include "tool_profile.h"
#include "file_library.h"
#include "graphics.h"
#include "main_cli.h"

void cleanup(th_db_t *db)
{
    uint8_t c;

    if (db->in_th != NULL) {
        switch (db->in_th->type) {
            case TH_IRTIS_DTV:
                dtv_close(db->in_th);
                break;
            case TH_FLIR_RJPG:
                rjpg_close(db->in_th);
                break;
            default:
                break;
        }
        db->in_th = NULL;
    }

    if (db->out_th != NULL) {
        switch (db->out_th->type) {
            case TH_IRTIS_DTV:
                dtv_close(db->out_th);
                break;
            case TH_FLIR_RJPG:
                rjpg_close(db->out_th);
                break;
            default:
                break;
        }
        db->out_th = NULL;
    }

    for (c = 0; c < STAGE_CNT; c++) {
        if (db->rgba[c].data != NULL) {
            free(db->rgba[c].data);
            db->rgba[c].data = NULL;
        }
#if 0
        if (db->rgba[c].overlay != NULL) {
            free(db->rgba[c].overlay);
            db->rgba[c].overlay = NULL;
        }
#endif

    }

    if (db->temp_arr != NULL) {
        free(db->temp_arr);
        db->temp_arr = NULL;
    }

    if (db->scale.data != NULL) {
        free(db->scale.data);
        db->scale.data = NULL;
    }

    if (db->scale.overlay != NULL) {
        free(db->scale.overlay);
        db->scale.overlay = NULL;
    }

    if (db->scale.combo != NULL) {
        free(db->scale.combo);
        db->scale.combo = NULL;
    }

    if (db->fe.vp_texture) {
        free_textures(1, &db->fe.vp_texture);
    }

    //memset(db, 0, sizeof(th_db_t));
}

void termination_handler(int)
{
    th_db_t *db = db_get_ptr();

    cleanup(db);
    line_plot_free();
    file_library_free();
    _exit(EXIT_SUCCESS);
}

void setup_sighandler(void)
{
    struct sigaction new_action, old_action;

    /* Set up the structure to specify the new action. */
    new_action.sa_handler = termination_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction(SIGINT, &new_action, NULL);
    }

    sigaction(SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction(SIGTERM, &new_action, NULL);
    }
}

int main_cli(th_db_t * db, uint8_t flags)
{
    unsigned err = 0;
    uint16_t th_width;
    uint16_t th_height;
    uint16_t file_type = FT_UNK;
    uint16_t file_subtype = FT_UNK;
    uint8_t flip_byte_order = 0;

    if (flags & SETUP_SIGHANDLER) {
        setup_sighandler();
    }

    get_file_type(db->p.in_file, &file_type, &file_subtype);

    if (lstat(db->p.in_file, &db->sb) == -1) {
        return EXIT_FAILURE;
    }

    if (file_type == TH_IRTIS_DTV) {

        dtv_new(&(db->in_th));
        db->in_th->subtype = file_subtype;

        if (dtv_open(db->in_th, db->p.in_file) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        dtv_populate_temp_arr(db);

        th_width = db->in_th->head.dtv->nst;
        th_height = db->in_th->head.dtv->nstv;
        //printf("%dx%d image, %d frames\n", th_width, th_height, db->in_th->head.dtv->frn);
        //printf("src temp: min %.2fdC  mult %.4fdC/q  max %.2fdC\n", db->in_th->head.dtv->tsc[1],
        //       db->in_th->head.dtv->tsc[0], db->in_th->head.dtv->tsc[1] + 256 * db->in_th->head.dtv->tsc[0]);

        // original image
        if (db->rgba[0].data) {
            free(db->rgba[0].data);
        }
        db->rgba[0].data = (uint8_t *) calloc(th_width * th_height * 4, 1);
        if (db->rgba[0].data == NULL) {
            errExit("allocating buffer");
        }
        db->rgba[0].width = th_width;
        db->rgba[0].height = th_height;

        dtv_new(&(db->out_th));
        db->out_th->subtype = file_subtype;

        dtv_rescale(db);
        dtv_transfer(db->out_th, db->rgba[0].data, db->p.pal);

    } else if (file_type == TH_FLIR_RJPG) {

        rjpg_new(&(db->in_th));
        rjpg_new(&(db->out_th));

        if (rjpg_open(db->in_th, db->p.in_file) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }

        th_width = db->in_th->head.rjpg->raw_th_img_width;
        th_height = db->in_th->head.rjpg->raw_th_img_height;

        // a rescale needs to happen since the radiometric data has to be converted to temperatures
        // via a very convoluted path. 
        // out_th will contain actual temperatures in ->frame instead of the radiometric raw data as in in_th->frame
        if (rjpg_rescale(db, &flip_byte_order) == RJPG_RET_FLIP_BYTE_ORDER) {
            fprintf(stderr,"warning: invalid values detected in %s, retrying with bytes flipped\n", db->p.in_file);
            // retry with the byte order flipped
            if (rjpg_rescale(db, &flip_byte_order) != EXIT_SUCCESS) {
                fprintf(stderr, "warning: image contains invalid data\n");
                return EXIT_FAILURE;
            }
        }

        if (db->rgba[0].data) {
            free(db->rgba[0].data);
        }
        db->rgba[0].data = (uint8_t *) calloc(th_width * th_height * 4, 1);
        if (db->rgba[0].data == NULL) {
            errExit("allocating buffer");
        }
        db->rgba[0].width = th_width;
        db->rgba[0].height = th_height;

        // create the output png file
        rjpg_transfer(db->out_th, db->rgba[0].data, db->p.pal);

    } else {
        fprintf(stderr, "warning: unknown input file type\n");
        return EXIT_FAILURE;
    }

    if (db->p.zoom_level > 1) {
        image_zoom(&db->rgba[1], &db->rgba[0], db->p.zoom_level, db->p.zoom_interpolation);
        db->rgba_vp = &db->rgba[1];
    } else {
        db->rgba_vp = &db->rgba[0];
    }

    if ((flags & GENERATE_OUT_FILE) && (db->p.out_file != NULL)) {
        err =
            lodepng_encode32_file(db->p.out_file, db->rgba_vp->data, th_width * db->p.zoom_level,
                                  th_height * db->p.zoom_level);

        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }
    }

    return EXIT_SUCCESS;
}

