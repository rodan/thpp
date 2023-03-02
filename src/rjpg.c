
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <math.h>
#include <arpa/inet.h>

#include "apr_base64.h"

#include "json_helper.h"
#include "lodepng.h"
#include "tlpi_hdr.h"
#include "thermogram.h"
#include "proj.h"
#include "rjpg.h"

#define                  RJPG_BUF_SIZE  2048
#define    RJPG_EXIFTOOL_BASE64_PREFIX  7       ///< number of bytes that need to be skipped during base64_decode

#define                         RJPG_K  273.15

#define   RJPG_CREATE_INTERMEDIATE_PNG_FILE

extern uint8_t vpl_data[12][768];

uint8_t rjpg_new(tgram_t ** thermo)
{
    tgram_t *t;

    *thermo = (tgram_t *) calloc(1, sizeof(tgram_t));
    if (*thermo == NULL) {
        errExit("allocating memory");
    }
    t = *thermo;

    t->head.rjpg = (rjpg_header_t *) calloc(1, sizeof(rjpg_header_t));
    if (t->head.rjpg == NULL) {
        errExit("allocating memory");
    }

    t->type = TH_FLIR_RJPG;

    return EXIT_SUCCESS;
}

uint8_t rjpg_extract_json(tgram_t * th, char *json_file)
{
    json_object *root_obj = NULL;
    json_object *item_obj = NULL;
    char *png_name;
    uint8_t *png_contents;
    int decode_len;
    unsigned x, y;
    unsigned err = 0;

    rjpg_header_t *h = th->head.rjpg;

    root_obj = json_object_from_file(json_file);

    if (root_obj == NULL) {
        fprintf(stderr, "unable to parse json file\n");
        return EXIT_FAILURE;
    }

    item_obj = json_object_array_get_idx(root_obj, 0);
    if (item_obj == NULL) {
        fprintf(stderr, "unable to parse json file\n");
        return EXIT_FAILURE;
    }

    h->emissivity = strtof(get(item_obj, "Emissivity"), NULL);
    h->distance = strtof(get(item_obj, "ObjectDistance"), NULL);
    h->rh = strtof(get(item_obj, "RelativeHumidity"), NULL) / 100.0;
    h->alpha1 = strtof(get(item_obj, "AtmosphericTransAlpha1"), NULL);
    h->alpha2 = strtof(get(item_obj, "AtmosphericTransAlpha2"), NULL);
    h->beta1 = strtof(get(item_obj, "AtmosphericTransBeta1"), NULL);
    h->beta2 = strtof(get(item_obj, "AtmosphericTransBeta2"), NULL);
    h->planckR1 = strtof(get(item_obj, "PlanckR1"), NULL);
    h->planckR2 = strtof(get(item_obj, "PlanckR2"), NULL);
    h->planckB = strtof(get(item_obj, "PlanckB"), NULL);
    h->planckF = strtof(get(item_obj, "PlanckF"), NULL);
    h->planckO = strtof(get(item_obj, "PlanckO"), NULL);
    h->atm_trans_X = strtof(get(item_obj, "AtmosphericTransX"), NULL);
    h->air_temp = strtof(get(item_obj, "AtmosphericTemperature"), NULL) + RJPG_K;
    h->refl_temp = strtof(get(item_obj, "ReflectedApparentTemperature"), NULL) + RJPG_K;
    h->raw_th_img_width = strtol(get(item_obj, "RawThermalImageWidth"), NULL, 10);
    h->raw_th_img_height = strtol(get(item_obj, "RawThermalImageHeight"), NULL, 10);

    h->atm_temp = h->air_temp - RJPG_K;
    h->h2o = h->rh * exp(1.5587 + 0.06939 * h->atm_temp - 0.00027816 * pow(h->atm_temp,2) + 0.00000068455 * pow(h->atm_temp,3)); //  # 8.563981576

    printf("emissivity = %f\n", h->emissivity);
    printf("distance = %f\n", h->distance);
    printf("rh = %f\n", h->rh);
    printf("alpha1 = %f\n", h->alpha1);
    printf("alpha2 = %f\n", h->alpha2);
    printf("beta1 = %f\n", h->beta1);
    printf("beta2 = %f\n", h->beta2);
    printf("planckR1 = %f\n", h->planckR1);
    printf("planckR2 = %f\n", h->planckR2);
    printf("planckB = %f\n", h->planckB);
    printf("planckF = %f\n", h->planckF);
    printf("planckO = %f\n", h->planckO);
    printf("atm_trans_X = %f\n", h->atm_trans_X);
    printf("air_temp = %f\n", h->air_temp);
    printf("refl_temp = %f\n", h->refl_temp);
    printf("raw_th_img_width = %u\n", h->raw_th_img_width);
    printf("raw_th_img_height = %u\n", h->raw_th_img_height);
    printf("atm_temp = %f\n", h->atm_temp);
    printf("h2o = %f\n", h->h2o);

    // fill raw_th_img
    decode_len =
        apr_base64_decode_len(get(item_obj, "RawThermalImage") + RJPG_EXIFTOOL_BASE64_PREFIX);

    png_name = (char *)calloc(strlen(json_file) + 5, sizeof(char));
    if (png_name == NULL) {
        errExit("allocating png filename");
        exit(EXIT_FAILURE);
    }

    png_contents = (uint8_t *) calloc(decode_len, sizeof(uint8_t));
    if (png_contents == NULL) {
        errExit("allocating png file");
        exit(EXIT_FAILURE);
    }

    apr_base64_decode((char *)png_contents,
                      get(item_obj, "RawThermalImage") + RJPG_EXIFTOOL_BASE64_PREFIX);

#ifdef RJPG_CREATE_INTERMEDIATE_PNG_FILE
    int png_fd;

    snprintf(png_name, strlen(json_file) + 5, "%s.png", json_file);

    if ((png_fd = open(png_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
        errMsg("opening file %s", png_name);
        goto cleanup;
    }

    if (write(png_fd, png_contents, decode_len) < decode_len) {
        errMsg("writing");
        goto cleanup;
    }

    close(png_fd);
#endif

    h->raw_th_img_sz = h->raw_th_img_width * h->raw_th_img_height;

    // th->frame gets allocated by lodepng_decode_memory()
    err = lodepng_decode_memory(&(th->frame), &x, &y, png_contents, decode_len, LCT_GREY, 8);
    if (err) {
        fprintf(stderr, "decoder error %u: %s\n", err, lodepng_error_text(err));
        goto cleanup;
    }

 cleanup:
    json_object_put(root_obj);

    free(png_name);
    free(png_contents);
    return EXIT_SUCCESS;
}

uint8_t rjpg_open(tgram_t * th, char *in_file)
{
    int status;
    pid_t pid;
    int fd_json;
    char tmp_json[] = "/tmp/thpp_json_XXXXXX";
    fd_json = mkstemp(tmp_json);

    switch (fork()) {
    case -1:
        errExit("fork");

    case 0:
        if (fd_json < 0) {
            errExit("during mkstemp");
        }
        dup2(fd_json, 1);
        execlp("exiftool", "exiftool", "-b", "-json", in_file, (char *)NULL);
        exit(EXIT_SUCCESS);
    default:
        for (;;) {
            pid = waitpid(-1, &status, WUNTRACED);
            if (pid == -1) {
                errExit("during waitpid");
            }

            if (status != 0) {
                fprintf(stderr, "exiftool exited in error\n");
                return EXIT_FAILURE;
            }

            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                // populated rjpg header with info from the json file
                if (rjpg_extract_json(th, tmp_json) == EXIT_FAILURE) {
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_SUCCESS;
}

uint8_t rjpg_transfer(const tgram_t * th, uint8_t * image, const uint8_t pal, const uint8_t zoom)
{
    uint16_t i = 0;
    uint16_t row = 0;
    uint16_t th_width = th->head.rjpg->raw_th_img_width;
    uint16_t th_height = th->head.rjpg->raw_th_img_height;
    uint8_t zc;
    uint8_t *color;

    if (zoom == 1) {
        for (i = 0; i < th_width * th_height; i++) {
            memcpy(image + (i * 3), &(vpl_data[pal][th->frame[i] * 3]), 3);
        }
    } else {
        // resize by multiplying pixels
        for (row = 0; row < th_height; row++) {
            for (i = 0; i < th_width; i++) {
                color = &(vpl_data[pal][th->frame[row * th_width + i] * 3]);
                for (zc = 0; zc < zoom; zc++) {
                    // multiply each pixel zoom times
                    memcpy(image + ((row * th_width * zoom * zoom + i * zoom + zc) * 3), color, 3);
                }
            }
            for (zc = 1; zc < zoom; zc++) {
                // copy last row zoom times
                memmove(image + ((row * th_width * zoom * zoom + zc * zoom * th_width) * 3),
                        image + ((row * th_width * zoom * zoom) * 3), th_width * zoom * 3);
            }
        }
    }

    return EXIT_SUCCESS;
}

uint8_t rjpg_rescale(tgram_t * dst_th, const tgram_t * src_th, const th_custom_param_t *p)
{
    ssize_t i;
    //double ft;
    double *dframe = NULL;
    double raw_refl;
    double ep_raw_refl;
    double raw_obj;
    double t_obj_c;
    double tau;
    double raw_atm;
    double tau_raw_atm;
    double epsilon_tau_raw_refl;
    uint16_t temp;
    double ftemp;
    double l_min;
    double l_max;
    double l_res;
    double l_distance;
    double l_emissivity;

    rjpg_header_t *h = dst_th->head.rjpg;

    // populate dst thermo header
    memcpy((uint8_t *) dst_th->head.rjpg, (uint8_t *) src_th->head.rjpg, sizeof(rjpg_header_t));

    // alloc frame mem
    dst_th->frame = (uint8_t *) calloc(h->raw_th_img_sz, sizeof(uint8_t));
    if (dst_th->frame == NULL) {
        errMsg("allocating buffer");
        goto cleanup;
    }

    // alloc float calculation buffer
    dframe = (double *)calloc(h->raw_th_img_sz, sizeof(double));
    if (dframe == NULL) {
        errMsg("allocating buffer");
        goto cleanup;
    }

    h->t_min = 32000.0;
    h->t_max = -32000.0;

    if (p->flags & OPT_SET_NEW_DISTANCE) {
        l_distance = p->distance;
    } else {
        l_distance = h->distance;
    }

    if (p->flags & OPT_SET_NEW_EMISSIVITY) {
        l_emissivity = p->emissivity;
    } else {
        l_emissivity = h->emissivity;
    }

    if (p->flags & OPT_SET_DISTANCE_COMP) {
        tau = h->atm_trans_X * exp(-sqrt(l_distance) * (h->alpha1 + h->beta1 * sqrt(h->h2o))) + (1 - h->atm_trans_X) * 
                  exp( -sqrt(l_distance) * (h->alpha2 + h->beta2 * sqrt(h->h2o)));
        raw_atm = h->planckR1 / (h->planckR2 * (exp(h->planckB / (h->air_temp)) - h->planckF)) - h->planckO;
        tau_raw_atm = raw_atm * (1 - tau);
        raw_refl = h->planckR1 / (h->planckR2 * (exp(h->planckB / (h->refl_temp)) - h->planckF)) - h->planckO;
        epsilon_tau_raw_refl = raw_refl * (1 - l_emissivity) * tau;

        for (i = 0; i < h->raw_th_img_sz; i++) {
            temp = src_th->frame[i] << 8;
            raw_obj = (temp - tau_raw_atm - epsilon_tau_raw_refl) / l_emissivity / tau;
            t_obj_c = h->planckB / log(h->planckR1 / (h->planckR2 * (raw_obj + h->planckO)) + h->planckF) - RJPG_K;
            dframe[i] = t_obj_c;
            if (h->t_min > t_obj_c) {
                h->t_min = t_obj_c;
            }
            if (h->t_max < t_obj_c) {
                h->t_max = t_obj_c;
            }
        }

    } else {
        raw_refl =
                h->planckR1 / (h->planckR2 * (exp(h->planckB / h->refl_temp) - h->planckF)) -
                h->planckO;
        ep_raw_refl = raw_refl * (1 - h->emissivity);

        for (i = 0; i < h->raw_th_img_sz; i++) {
            temp = src_th->frame[i] << 8;
            raw_obj = 1.0 * (temp - ep_raw_refl) / h->emissivity;
            t_obj_c =
                h->planckB / log(h->planckR1 / (h->planckR2 * (raw_obj + h->planckO)) + h->planckF) -
                RJPG_K;
            dframe[i] = t_obj_c;
            if (h->t_min > t_obj_c) {
                h->t_min = t_obj_c;
            }
            if (h->t_max < t_obj_c) {
                h->t_max = t_obj_c;
            }
        }
    }

    h->t_res = (h->t_max - h->t_min) / 255;

    printf("t_min %.2f, t_max %.2f, t_res %.2f\n", h->t_min, h->t_max, h->t_res);

    if (p->flags & OPT_SET_NEW_MIN) {
        l_min = p->t_min;
    } else {
        l_min = h->t_min;
    }

    if (p->flags & OPT_SET_NEW_MAX) {
        l_max = p->t_max;
    } else {
        l_max = h->t_max;
    }

    if (l_max <= l_min) {
        fprintf(stderr, "invalid min %.2f/max %.2f values\n", l_min, l_max);
        goto cleanup;
    }

    l_res = (l_max - l_min) / 255;

    // rescale image
    for (i = 0; i < h->raw_th_img_sz; i++) {
        ftemp = ((dframe[i] - l_min) / l_res) + 0.5;
        if (ftemp < 0) {
            ftemp = 0;
        } else if (ftemp > 255) {
            ftemp = 255;
        }
        dst_th->frame[i] = ftemp;
    }

 cleanup:
    if (dframe) {
        free(dframe);
    }

    return EXIT_SUCCESS;
}

void rjpg_close(tgram_t * thermo)
{
    if (thermo) {
        if (thermo->frame) {
            free(thermo->frame);
        }
        if (thermo->head.rjpg) {
            free(thermo->head.rjpg);
        }

    }

    if (thermo) {
        free(thermo);
    }
}
