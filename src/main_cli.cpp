
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
#include "processing.h"
#include "version.h"
#include "main_cli.h"

th_db db;

void cleanup(void)
{
    printf("cleanup\n");
    if (db.in_th != NULL) {
        switch (db.in_th->type) {
            case TH_IRTIS_DTV:
                dtv_close(db.in_th);
                break;
            case TH_FLIR_RJPG:
                rjpg_close(db.in_th);
                break;
            default:
                break;
        }
    }

    if (db.out_th != NULL) {
        switch (db.out_th->type) {
            case TH_IRTIS_DTV:
                dtv_close(db.out_th);
                break;
            case TH_FLIR_RJPG:
                rjpg_close(db.out_th);
                break;
            default:
                break;
        }
    }

    if (db.rgba.data != NULL) {
        free(db.rgba.data);
    }
}

void termination_handler(int)
{
    cleanup();
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

int main_cli(th_db_t * db)
{

    //tgram_t *in_th = NULL;
    //tgram_t *out_th = NULL;
    unsigned err = 0;
    //uint8_t *image = NULL;
    uint16_t th_width;
    uint16_t th_height;
    uint8_t file_type = FT_UNK;

    file_type = get_file_type(db->p.in_file);

    setup_sighandler();

    if (file_type == FT_DTV) {

        dtv_new(&(db->in_th));

        dtv_open(db->in_th, db->p.in_file);

        th_width = db->in_th->head.dtv->nst;
        th_height = db->in_th->head.dtv->nstv;
        printf("%dx%d image, %d frames\n", th_width, th_height, db->in_th->head.dtv->frn);
        printf("src temp: min %.2fdC  mult %.4fdC/q  max %.2fdC\n", db->in_th->head.dtv->tsc[1],
               db->in_th->head.dtv->tsc[0], db->in_th->head.dtv->tsc[1] + 256 * db->in_th->head.dtv->tsc[0]);

        if (db->rgba.data) {
            free(db->rgba.data);
        }

        db->rgba.data = (uint8_t *) calloc(th_width * th_height * db->p.zoom * db->p.zoom * 4, 1);
        if (db->rgba.data == NULL) {
            errExit("allocating buffer");
        }

        if (db->p.flags & (OPT_SET_NEW_MIN | OPT_SET_NEW_MAX)) {

            dtv_new(&(db->out_th));

            dtv_rescale(db->out_th, db->in_th, &(db->p));

            dtv_transfer(db->out_th, db->rgba.data, db->p.pal, db->p.zoom);

            printf("dst temp: min %.2fdC  mult %.4fdC/q  max %.2fdC\n", db->out_th->head.dtv->tsc[1],
                   db->out_th->head.dtv->tsc[0],
                   db->out_th->head.dtv->tsc[1] + 256.0 * db->out_th->head.dtv->tsc[0]);
        } else {
            dtv_transfer(db->in_th, db->rgba.data, db->p.pal, db->p.zoom);
        }

        err =
            lodepng_encode32_file(db->p.out_file, db->rgba.data, th_width * db->p.zoom,
                                  th_height * db->p.zoom);
        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }

        //free(image);
        //dtv_close(in_th);
        //if (out_th) {
        //    dtv_close(out_th);
        //}
    } else if (file_type == FT_RJPG) {

        rjpg_new(&(db->in_th));
        rjpg_new(&(db->out_th));

        rjpg_open(db->in_th, db->p.in_file);

        th_width = db->in_th->head.rjpg->raw_th_img_width;
        th_height = db->in_th->head.rjpg->raw_th_img_height;

        // a rescale needs to happen since the radiometric data has to be converted to temperatures
        // via a very convoluted path. 
        // out_th will contain actual temperatures in ->frame instead of the radiometric raw data as in in_th->frame
        rjpg_rescale(db->out_th, db->in_th, &(db->p));

        if (db->rgba.data) {
            free(db->rgba.data);
        }

        db->rgba.data = (uint8_t *) calloc(th_width * th_height * db->p.zoom * db->p.zoom * 4, 1);
        if (db->rgba.data == NULL) {
            errExit("allocating buffer");
        }
        // create the output png file
        rjpg_transfer(db->out_th, db->rgba.data, db->p.pal, db->p.zoom);
        //print_buf(in_th->frame, in_th->head.rjpg->raw_th_img_sz);

        err =
            lodepng_encode32_file(db->p.out_file, db->rgba.data, th_width * db->p.zoom,
                                  th_height * db->p.zoom);
        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }

        //free(image);
        //rjpg_close(in_th);
        //rjpg_close(out_th);
    } else {
        fprintf(stderr, "unknown input file type\n");
        exit(EXIT_FAILURE);
    }

    pal_free();

    return EXIT_SUCCESS;
}
