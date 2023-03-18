
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <getopt.h>
#include "tlpi_hdr.h"
#include "palette.h"
#include "version.h"
#include "graphics.h"
#include "proj.h"

#define BUF_SIZE  32
#define  DEFAULT_PALETTE  6

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

uint8_t parse_options(int argc, char *argv[], th_custom_param_t * p)
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
        {"distcomp", no_argument, 0, 'k'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    memset(p, 0, sizeof(th_custom_param_t));
    p->pal = DEFAULT_PALETTE;
    p->zoom = 1;

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
            p->zoom = atoi(optarg);
            break;
        case 'l':
            p->flags |= OPT_SET_NEW_MIN;
            p->t_min = atof(optarg);
            break;
        case 'a':
            p->flags |= OPT_SET_NEW_MAX;
            p->t_max = atof(optarg);
            break;
        case 'd':
            p->flags |= OPT_SET_NEW_DISTANCE;
            p->distance = atof(optarg);
            break;
        case 'k':
            p->flags |= OPT_SET_DISTANCE_COMP;
            break;
        case 'e':
            p->flags |= OPT_SET_NEW_EMISSIVITY;
            p->emissivity = atof(optarg);
            break;
        case 't':
            p->flags |= OPT_SET_NEW_AT;
            p->atm_temp = atof(optarg);
            break;
        case 'r':
            p->flags |= OPT_SET_NEW_RH;
            p->rh = atof(optarg);
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

    if ((p->in_file == NULL) || (p->out_file == NULL)) {
        fprintf(stderr, "Error: provide input and output files\n");
        show_usage();
        exit(1);
    }

    return EXIT_SUCCESS;
}

uint8_t get_file_type(const char *in_file)
{
    int fd;
    uint8_t *buf;
    uint8_t ret = FT_UNK;
    static const uint8_t sig_exif[4] = { 0x45, 0x78, 0x69, 0x66 };      // appears at file offset 0x18
    static const uint8_t sig_dtv[2] = { 0x6e, 0x02 };   // appears at offset 0x0

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
        ret = FT_DTV;
    }

    if (memcmp(buf + 0x18, sig_exif, 4) == 0) {
        ret = FT_RJPG;
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
            *t_min = th->head.dtv->tsc[1];
            *t_max = th->head.dtv->tsc[1] + 256.0 * th->head.dtv->tsc[0];
            break;
        default:
            *t_min = 0.0;
            *t_max = 0.0;
            return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

void draw_scale_overlay(scale_t *scale, const double major, const double minor, const uint8_t precission)
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
                draw_text(&c, 80, t - 22, &text[0], WHITE, 2);
                break;
            case 2:
                snprintf(text, 11, "%3.02f", i * major);
                draw_text(&c, 80, t - 22, &text[0], WHITE, 2);
                break;
            default:
                snprintf(text, 11, "%3.0f", i * major);
                draw_text(&c, 80, t - 22, &text[0], WHITE, 2);
                break;
        } 
        draw_major_tick(&c, (uint16_t) t);
    }

    snprintf(text, 11, "dC");
    draw_text(&c, 30, t - 22, text, WHITE, 2);

    // minor tick marks
    for (i = (int16_t) scale->t_max / minor; i > (int16_t) scale->t_min / minor; i--) {
        t = q * (scale->t_max - (i * minor)) + 0.5;
        draw_minor_tick(&c, (int16_t) t);
    }
}

void generate_scale(scale_t *scale)
{
    uint32_t j;
    double delta = scale->t_max - scale->t_min;

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
        draw_scale_overlay(scale, 0.1, 0.05, 1);
    } else if (delta < 5.0) {
        draw_scale_overlay(scale, 0.5, 0.1, 1); // 4-10
    } else if (delta < 10.0) {
        draw_scale_overlay(scale, 1.0, 0.5, 0); // 5-10
    } else if (delta < 20.0) {
        draw_scale_overlay(scale, 2.0, 0.5, 0); // 5-10
    } else if (delta < 50.0) {
        draw_scale_overlay(scale, 5.0, 1.0, 0); // 4-10
    } else if (delta < 100) {
        draw_scale_overlay(scale, 10.0, 5.0, 0); // 5-10
    } else if (delta < 200) {
        draw_scale_overlay(scale, 20.0, 10.0, 0); // 5-10
    } else {
        draw_scale_overlay(scale, 50.0, 10.0, 0);
    }

    // combine the scale and the overlay
    uint32_t *combo_ptr = (uint32_t *) scale->combo;
    uint32_t *overlay_ptr = (uint32_t *) scale->overlay;
    uint32_t *data_ptr = (uint32_t *) scale->data;

    for (j = 0; j < scale->width * scale->height; j++) {
        if (*(overlay_ptr + j)) {
            //inv = *(data_ptr + j);
            *(combo_ptr + j) = highlight_color(*(data_ptr + j), WHITE);
            //printf("%08x %08x\n", inv, *(combo_ptr + j));
        } else {
            *(combo_ptr + j) = *(data_ptr + j);
        }
    }

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
