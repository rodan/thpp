
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#include "sdl_vis.h"
#include "lodepng.h"
#include "tlpi_hdr.h"
#include "thermogram.h"
#include "proj.h"
#include "dtv.h"
#include "rjpg.h"
#include "palette.h"
#include "processing.h"
#include "version.h"

th_custom_param_t p;

int main(int argc, char *argv[])
{
    tgram_t *in_th = NULL;
    tgram_t *out_th = NULL;
    unsigned err = 0;
    uint8_t *image;
    uint16_t th_width;
    uint16_t th_height;
    uint8_t file_type = FT_UNK;

    parse_options(argc, argv, &p);

    file_type = get_file_type(p.in_file);

    if (file_type == FT_DTV) {

        dtv_new(&in_th);

        dtv_open(in_th, p.in_file);

        th_width = in_th->head.dtv->nst;
        th_height = in_th->head.dtv->nstv;
        printf("%dx%d image, %d frames\n", th_width, th_height, in_th->head.dtv->frn);
        printf("src temp: min %.2fdC  mult %.4fdC/q  max %.2fdC\n", in_th->head.dtv->tsc[1],
               in_th->head.dtv->tsc[0], in_th->head.dtv->tsc[1] + 256 * in_th->head.dtv->tsc[0]);

        image = (uint8_t *) calloc(th_width * th_height * p.zoom * p.zoom * 3, 1);
        if (image == NULL) {
            errExit("allocating buffer");
        }

        if (p.flags & (OPT_SET_NEW_MIN | OPT_SET_NEW_MAX)) {

            dtv_new(&out_th);

            dtv_rescale(out_th, in_th, &p);

            dtv_transfer(out_th, image, p.pal, p.zoom);

            printf("dst temp: min %.2fdC  mult %.4fdC/q  max %.2fdC\n", out_th->head.dtv->tsc[1],
                   out_th->head.dtv->tsc[0],
                   out_th->head.dtv->tsc[1] + 256.0 * out_th->head.dtv->tsc[0]);
        } else {
            dtv_transfer(in_th, image, p.pal, p.zoom);
        }

        err = lodepng_encode24_file(p.out_file, image, th_width * p.zoom, th_height * p.zoom);
        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }

        free(image);
        dtv_close(in_th);
        if (out_th) {
            dtv_close(out_th);
        }
    } else if (file_type == FT_RJPG) {

        rjpg_new(&in_th);
        rjpg_new(&out_th);

        rjpg_open(in_th, p.in_file);

        th_width = in_th->head.rjpg->raw_th_img_width;
        th_height = in_th->head.rjpg->raw_th_img_height;

        // a rescale needs to happen since the radiometric data has to be converted to temperatures
        // via a very convoluted path. 
        // out_th will contain actual temperatures in ->frame instead of the radiometric raw data as in in_th->frame
        rjpg_rescale(out_th, in_th, &p);

        image = (uint8_t *) calloc(th_width * th_height * p.zoom * p.zoom * 3, 1);
        if (image == NULL) {
            errExit("allocating buffer");
        }
        // create the output png file
        rjpg_transfer(out_th, image, p.pal, p.zoom);
        //print_buf(in_th->frame, in_th->head.rjpg->raw_th_img_sz);

        err =
            lodepng_encode24_file(p.out_file, image, th_width * p.zoom,
                                  th_height * p.zoom);
        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }

        free(image);
        rjpg_close(in_th);
        rjpg_close(out_th);
    } else {
        fprintf(stderr, "unknown input file type\n");
        exit(EXIT_FAILURE);
    }

    pal_free();

#ifdef CONFIG_SDL
    sdl_vis_file(p.out_file, th_width * p.zoom, th_height * p.zoom);
#endif

    return EXIT_SUCCESS;
}

