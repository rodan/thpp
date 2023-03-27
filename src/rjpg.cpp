
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
#include "palette.h"
#include "proj.h"
#include "rjpg.h"

#define                  RJPG_BUF_SIZE  2048
#define    RJPG_EXIFTOOL_BASE64_PREFIX  7       ///< number of bytes that need to be skipped during base64_decode

//#define   RJPG_CREATE_INTERMEDIATE_PNG_FILE

uint8_t rjpg_new(tgram_t ** thermo)
{
    tgram_t *t;

    if (*thermo != NULL) {
        rjpg_close(*thermo);
    }

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
    char *model;

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
    model = get(item_obj, "CameraModel");

    if (memcmp(model, ID_THERMACAM_E25, min(strlen(model), strlen(ID_THERMACAM_E25))) == 0) {
        th->subtype = TH_FLIR_THERMACAM_E25;
    } else if (memcmp(model, ID_FLIR_E5, min(strlen(model), strlen(ID_FLIR_E5))) == 0) {
        th->subtype = TH_FLIR_E5;
    }

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
        close(png_fd);
        goto cleanup;
    }

    close(png_fd);
#endif

    h->raw_th_img_sz = h->raw_th_img_width * h->raw_th_img_height;

    // th->framew gets allocated by lodepng_decode_memory()
    err = lodepng_decode_memory((uint8_t **)&(th->framew), &x, &y, png_contents, decode_len, LCT_GREY, 16);
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

    umask(077);
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
                unlink(tmp_json);
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_SUCCESS;
}

uint8_t rjpg_transfer(const tgram_t * th, uint8_t * image, const uint8_t pal_id, const uint8_t zoom)
{
    uint32_t i = 0;
    uint16_t row = 0;
    uint16_t th_width;
    uint16_t th_height;
    uint8_t zc;
    uint8_t color[4];
    uint8_t *pal_rgb;
    
    if ((th == NULL) || (image == NULL)) {
        return EXIT_FAILURE;
    }

    pal_rgb = pal_init_lut(pal_id, PAL_8BPP);
    if (pal_rgb == NULL) {
        fprintf(stderr, "palette generation error\n");
        exit(EXIT_FAILURE);
    }

    th_width = th->head.rjpg->raw_th_img_width;
    th_height = th->head.rjpg->raw_th_img_height;

    if (zoom == 1) {
        for (i = 0; i < th_width * th_height; i++) {
            memcpy(image + (i * 4), &(pal_rgb[th->frame[i] * 3]), 3);
            image[i*4 + 3] = 255; // alpha channel
        }
    } else {
        // resize by multiplying pixels
        for (row = 0; row < th_height; row++) {
            for (i = 0; i < th_width; i++) {
                memcpy(color, &(pal_rgb[th->frame[row * th_width + i] * 3]), 3);
                color[3] = 255; // alpha channel
                for (zc = 0; zc < zoom; zc++) {
                    // multiply each pixel zoom times
                    memcpy(image + ((row * th_width * zoom * zoom + i * zoom + zc) * 4), color, 4);
                }
            }
            for (zc = 1; zc < zoom; zc++) {
                // copy last row zoom times
                memmove(image + ((row * th_width * zoom * zoom + zc * zoom * th_width) * 4),
                        image + ((row * th_width * zoom * zoom) * 4), th_width * zoom * 4);
            }
        }
    }

    return EXIT_SUCCESS;
}

uint8_t rjpg_rescale(th_db_t *d)
{
    ssize_t i;
    uint8_t framew_needs_flippage = 0;
    double raw_refl;
    double ep_raw_refl;
    double raw_obj;
    double h2o;
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
    double l_atm_temp;
    double l_rh;
    tgram_t * src_th = d->in_th;
    tgram_t * dst_th = d->out_th;
    th_getopt_t *p = &(d->p);

    rjpg_header_t *h = dst_th->head.rjpg;

    // populate dst thermo header
    memcpy((uint8_t *) dst_th->head.rjpg, (uint8_t *) src_th->head.rjpg, sizeof(rjpg_header_t));

    // alloc frame mem
    dst_th->frame = (uint8_t *) calloc(h->raw_th_img_sz, sizeof(uint8_t));
    if (dst_th->frame == NULL) {
        errMsg("allocating buffer");
        return EXIT_FAILURE;
    }

    if (d->temp_arr != NULL) {
        free(d->temp_arr);
    }

    // alloc float calculation buffer
    d->temp_arr = (double *)calloc(h->raw_th_img_sz, sizeof(double));
    if (d->temp_arr == NULL) {
        errMsg("allocating buffer");
        return EXIT_FAILURE;
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

    if (p->flags & OPT_SET_NEW_AT) {
        l_atm_temp = p->atm_temp;
    } else {
        l_atm_temp = h->air_temp - RJPG_K;
    }

    if (p->flags & OPT_SET_NEW_RH) {
        l_rh = p->rh;
    } else {
        l_rh = h->rh;
    }

    if (src_th->subtype == TH_FLIR_THERMACAM_E25) {
        if (localhost_is_le()) {
            framew_needs_flippage = 1;
        }
    } else {
        if (!localhost_is_le()) {
            framew_needs_flippage = 1;
        }
    }

    if (p->flags & OPT_SET_DISTANCE_COMP) {
        h2o = l_rh * exp(1.5587 + 0.06939 * l_atm_temp - 0.00027816 * pow(l_atm_temp,2) + 0.00000068455 * pow(l_atm_temp,3)); //  # 8.563981576
        tau = h->atm_trans_X * exp(-sqrt(l_distance) * (h->alpha1 + h->beta1 * sqrt(h2o))) + (1 - h->atm_trans_X) * 
                  exp( -sqrt(l_distance) * (h->alpha2 + h->beta2 * sqrt(h2o)));
        raw_atm = h->planckR1 / (h->planckR2 * (exp(h->planckB / (h->air_temp)) - h->planckF)) - h->planckO;
        tau_raw_atm = raw_atm * (1 - tau);
        raw_refl = h->planckR1 / (h->planckR2 * (exp(h->planckB / (h->refl_temp)) - h->planckF)) - h->planckO;
        epsilon_tau_raw_refl = raw_refl * (1 - l_emissivity) * tau;

        for (i = 0; i < h->raw_th_img_sz; i++) {
            if (framew_needs_flippage) {
                temp = htons(src_th->framew[i]);
            } else {
                temp = src_th->framew[i];
            }
            raw_obj = (temp - tau_raw_atm - epsilon_tau_raw_refl) / l_emissivity / tau;
            t_obj_c = h->planckB / log(h->planckR1 / (h->planckR2 * (raw_obj + h->planckO)) + h->planckF) - RJPG_K;
            d->temp_arr[i] = t_obj_c;
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
        ep_raw_refl = raw_refl * (1 - l_emissivity);

        for (i = 0; i < h->raw_th_img_sz; i++) {
            if (framew_needs_flippage) {
                // data in the exif-png gets here in big endian words
                temp = htons(src_th->framew[i]);
            } else {
                temp = src_th->framew[i];
            }
            raw_obj = 1.0 * (temp - ep_raw_refl) / l_emissivity;
            t_obj_c =
                h->planckB / log(h->planckR1 / (h->planckR2 * (raw_obj + h->planckO)) + h->planckF) -
                RJPG_K;
            d->temp_arr[i] = t_obj_c;
            if (h->t_min > t_obj_c) {
                h->t_min = t_obj_c;
            }
            if (h->t_max < t_obj_c) {
                h->t_max = t_obj_c;
            }
        }
    }

    h->t_res = (h->t_max - h->t_min) / 255;

    //printf("t_min %.2f, t_max %.2f, t_res %.2f\n", h->t_min, h->t_max, h->t_res);

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
        return EXIT_FAILURE;
    }

    l_res = (l_max - l_min) / 255;

    // rescale image
    for (i = 0; i < h->raw_th_img_sz; i++) {
        ftemp = ((d->temp_arr[i] - l_min) / l_res) + 0.5;
        if (ftemp < 0) {
            ftemp = 0;
        } else if (ftemp > 255) {
            ftemp = 255;
        }
        dst_th->frame[i] = ftemp;
    }

    return EXIT_SUCCESS;
}

void rjpg_close(tgram_t * thermo)
{
    if (thermo) {
        if (thermo->frame) {
            free(thermo->frame);
            thermo->frame = NULL;
        }
        if (thermo->framew) {
            free(thermo->framew);
            thermo->framew = NULL;
        }
        if (thermo->head.rjpg) {
            free(thermo->head.rjpg);
            thermo->head.rjpg = NULL;
        }
    }

    if (thermo) {
        free(thermo);
    }
}
