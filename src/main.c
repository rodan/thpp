
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lodepng.h"
#include "tlpi_hdr.h"
#include "dtv.h"
#include "rjpg.h"
#include "version.h"

#define   FT_UNK  0
#define   FT_DTV  1
#define  FT_RJPG  2

#define BUF_SIZE  32

static const uint8_t sig_exif[4] = {0x45, 0x78, 0x69, 0x66}; // appears at file offset 0x18
static const uint8_t sig_dtv[2] = {0x6e, 0x02}; // appears at offset 0x0

void show_usage(void)
{
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, " thpps  -i INPUT_FILE -o OUTPUT_FILE [-h] [-v]\n");
}

void show_version(void)
{
    fprintf(stdout, " thpps %d.%d\nbuild %d commit %d\n", VER_MAJOR, VER_MINOR, BUILD, COMMIT);
}

int main(int argc, char *argv[])
{
    int opt;
    int pal = 4;
    char *in_file = NULL;
    char *out_file = NULL;
    tgram_t *in_th = NULL;
    tgram_t *out_th = NULL;
    tgram_rjpg_t *in_rjpg_th = NULL;
    unsigned err = 0;
    uint8_t *image;
    uint8_t zoom = 1;
    uint16_t res_x;
    uint16_t res_y;
    double new_min = 0.0;
    double new_max = 0.0;
    uint8_t file_type = FT_UNK;
    int fd;
    uint8_t *buf;

    while ((opt = getopt(argc, argv, "i:o:p:z:l:a:vh")) != -1) {
        switch (opt) {
        case 'i':
            in_file = optarg;
            break;
        case 'o':
            out_file = optarg;
            break;
        case 'p':
            pal = atoi(optarg);
            break;
        case 'z':
            zoom = atoi(optarg);
            break;
        case 'l':
            new_min = atof(optarg);
            break;
        case 'a':
            new_max = atof(optarg);
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

    if ((in_file == NULL) || (out_file == NULL)) {
        fprintf(stderr, "Error: provide input and output files\n");
        show_usage();
        exit(1);
    }

    in_th = (tgram_t *) calloc(1, sizeof(tgram_t));
    if (in_th == NULL) {
        errExit("allocating memory");
    }

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

    if (memcmp(buf, sig_dtv, 2) == 0) {
        file_type = FT_DTV;
    }

    if (memcmp(buf + 0x18, sig_exif, 4) == 0) {
        file_type = FT_RJPG;
    }

    close(fd);
    free(buf);

    if (file_type == FT_DTV) {

        dtv_open(in_th, in_file);
        res_x = in_th->head.nst;
        res_y = in_th->head.nstv;

        printf("%dx%d image, %d frames\n", res_x, res_y, in_th->head.frn);
        printf("src temp: min %.2fdC  mult %.4fdC/q  max %.2fdC\n", in_th->head.tsc[1], in_th->head.tsc[0], in_th->head.tsc[1] + 256*in_th->head.tsc[0]);

        image = (uint8_t *) calloc(res_x * res_y * zoom * zoom * 3, 1);
        if (image == NULL) {
            errExit("allocating buffer");
        }
        
        if ((new_min != 0) || (new_max != 0)) {
            printf("recalculate to %f %f\n", new_min, new_max);

            out_th = (tgram_t *) calloc(1, sizeof(tgram_t));
            if (out_th == NULL) {
                errExit("allocating memory");
            }

            dtv_rescale(out_th, in_th, new_min, new_max);
            dtv_transfer(out_th, image, pal, zoom);
            printf("dst temp: min %.2fdC  mult %.4fdC/q  max %.2fdC\n", out_th->head.tsc[1], out_th->head.tsc[0], out_th->head.tsc[1] + 256.0 * out_th->head.tsc[0]);
        } else {
            dtv_transfer(in_th, image, pal, zoom);
        }

        err = lodepng_encode24_file(out_file, image, res_x * zoom, res_y * zoom);
        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }

        free(image);
        dtv_close(in_th);
        if (out_th) {
            dtv_close(out_th);
        }
    } else if (file_type == FT_RJPG) {
        printf("rjpg detected\n");
        rjpg_open(in_rjpg_th, in_file);


        rjpg_close(in_rjpg_th);
    } else {
        fprintf(stderr, "unknown input file type\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
