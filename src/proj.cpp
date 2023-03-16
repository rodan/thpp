
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include "tlpi_hdr.h"
#include "version.h"
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
