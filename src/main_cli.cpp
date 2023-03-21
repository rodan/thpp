
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
#include "main_cli.h"

th_db_t db;

void cleanup(th_db_t *db)
{
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

    if (db->rgba.data != NULL) {
        free(db->rgba.data);
        db->rgba.data = NULL;
    }

#if 0
    if (db->rgba.overlay != NULL) {
        free(db->rgba.overlay);
        db->rgba.overlay = NULL;
    }
#endif

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

    line_plot_free();
}

void termination_handler(int)
{
    cleanup(&db);
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
    uint8_t file_type = FT_UNK;

    if (flags & SETUP_SIGHANDLER) {
        setup_sighandler();
    }

    file_type = get_file_type(db->p.in_file);

    if (lstat(db->p.in_file, &db->sb) == -1) {
        errExit("lstat");
    }

    if (file_type == FT_DTV) {

        dtv_new(&(db->in_th));

        dtv_open(db->in_th, db->p.in_file);
        dtv_populate_temp_arr(db);

        th_width = db->in_th->head.dtv->nst;
        th_height = db->in_th->head.dtv->nstv;
        //printf("%dx%d image, %d frames\n", th_width, th_height, db->in_th->head.dtv->frn);
        //printf("src temp: min %.2fdC  mult %.4fdC/q  max %.2fdC\n", db->in_th->head.dtv->tsc[1],
        //       db->in_th->head.dtv->tsc[0], db->in_th->head.dtv->tsc[1] + 256 * db->in_th->head.dtv->tsc[0]);

        if (db->rgba.data) {
            free(db->rgba.data);
        }

        db->rgba.data = (uint8_t *) calloc(th_width * th_height * db->p.zoom * db->p.zoom * 4, 1);
        if (db->rgba.data == NULL) {
            errExit("allocating buffer");
        }

        db->rgba.width = th_width * db->p.zoom;
        db->rgba.height = th_height * db->p.zoom;

        dtv_new(&(db->out_th));
        dtv_rescale(db);
        dtv_transfer(db->out_th, db->rgba.data, db->p.pal, db->p.zoom);

        //printf("dst temp: min %.2fdC  mult %.4fdC/q  max %.2fdC\n", db->out_th->head.dtv->tsc[1],
        //        db->out_th->head.dtv->tsc[0],
        //        db->out_th->head.dtv->tsc[1] + 256.0 * db->out_th->head.dtv->tsc[0]);

        if (flags & GENERATE_OUT_FILE) {
            err =
                lodepng_encode32_file(db->p.out_file, db->rgba.data, th_width * db->p.zoom,
                                      th_height * db->p.zoom);
            if (err) {
                fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
            }
        }

    } else if (file_type == FT_RJPG) {

        rjpg_new(&(db->in_th));
        rjpg_new(&(db->out_th));

        rjpg_open(db->in_th, db->p.in_file);

        th_width = db->in_th->head.rjpg->raw_th_img_width;
        th_height = db->in_th->head.rjpg->raw_th_img_height;

        // a rescale needs to happen since the radiometric data has to be converted to temperatures
        // via a very convoluted path. 
        // out_th will contain actual temperatures in ->frame instead of the radiometric raw data as in in_th->frame
        rjpg_rescale(db);

        if (db->rgba.data) {
            free(db->rgba.data);
        }

        db->rgba.data = (uint8_t *) calloc(th_width * th_height * db->p.zoom * db->p.zoom * 4, 1);
        if (db->rgba.data == NULL) {
            errExit("allocating buffer");
        }

        db->rgba.width = th_width * db->p.zoom;
        db->rgba.height = th_height * db->p.zoom;

        // create the output png file
        rjpg_transfer(db->out_th, db->rgba.data, db->p.pal, db->p.zoom);

        if (flags & GENERATE_OUT_FILE) {
            err =
                lodepng_encode32_file(db->p.out_file, db->rgba.data, th_width * db->p.zoom,
                                      th_height * db->p.zoom);
            if (err) {
                fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
            }
        }

    } else {
        fprintf(stderr, "warning: unknown input file type\n");
        return EXIT_FAILURE;
    }

    pal_free();

    return EXIT_SUCCESS;
}
