
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <math.h>
#include <arpa/inet.h>
#include "tinytiffreader.h"
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

const uint8_t magic_number_png[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
const uint8_t magic_number_tiff_1[4] = {0x49, 0x49, 0x2a, 0x00};
const uint8_t magic_number_tiff_2[4] = {0x4d, 0x4d, 0x00, 0x2a};

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
    char *img_name = NULL;
    uint8_t *img_contents = NULL;
    int decode_len;
    unsigned x, y;
    unsigned err = 0;
    char *model;
    uint8_t ret = EXIT_SUCCESS;
    int img_fd = -1;
    uint32_t i;

    rjpg_header_t *h = th->head.rjpg;

    root_obj = json_object_from_file(json_file);

    if (root_obj == NULL) {
        fprintf(stderr, "unable to parse json file\n");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    item_obj = json_object_array_get_idx(root_obj, 0);
    if (item_obj == NULL) {
        fprintf(stderr, "unable to parse json file\n");
        ret = EXIT_FAILURE;
        goto cleanup;
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

    img_name = (char *)calloc(strlen(json_file) + 5, sizeof(char));
    if (img_name == NULL) {
        errExit("allocating png filename");
        exit(EXIT_FAILURE);
    }

    img_contents = (uint8_t *) calloc(decode_len, sizeof(uint8_t));
    if (img_contents == NULL) {
        errExit("allocating png file");
        exit(EXIT_FAILURE);
    }

    apr_base64_decode((char *)img_contents,
                      get(item_obj, "RawThermalImage") + RJPG_EXIFTOOL_BASE64_PREFIX);

    snprintf(img_name, strlen(json_file) + 5, "%s.img", json_file);

    if ((img_fd = open(img_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
        errMsg("opening file %s", img_name);
        ret = EXIT_FAILURE;
        goto cleanup_no_close;
    }

    if (write(img_fd, img_contents, decode_len) < decode_len) {
        errMsg("writing");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    h->raw_th_img_sz = h->raw_th_img_width * h->raw_th_img_height;

    if (memcmp(img_contents, magic_number_png, 8) == 0) {
        // th->framew gets allocated by lodepng_decode_memory()
        err = lodepng_decode_memory((uint8_t **)&(th->framew), &x, &y, img_contents, decode_len, LCT_GREY, 16);
        if (err) {
            fprintf(stderr, "decoder error %u: %s\n", err, lodepng_error_text(err));
            ret = EXIT_FAILURE;
            goto cleanup;
        }
    } else if ((memcmp(img_contents, magic_number_tiff_1, 4) == 0) || 
               (memcmp(img_contents, magic_number_tiff_2, 4) == 0)) {

        TinyTIFFReaderFile* tiffr=NULL;
        tiffr=TinyTIFFReader_open(img_name); 
        if (!tiffr) {
            fprintf(stderr, "decoder error TinyTIFFReader_open() has failed\n");
            ret = EXIT_FAILURE;
            goto cleanup;
        } else {
            const uint32_t width=TinyTIFFReader_getWidth(tiffr);
            const uint32_t height=TinyTIFFReader_getHeight(tiffr);
            const uint16_t samples=TinyTIFFReader_getSamplesPerPixel(tiffr);
            const uint16_t bps=TinyTIFFReader_getBitsPerSample(tiffr, 0);

            printf("size %ux%u, %u samples per pixel, %u bits each\n", width, height, samples, bps);

            th->framew = (uint16_t*) calloc(width*height, sizeof(uint16_t));

            TinyTIFFReader_getSampleData(tiffr, th->framew, 0);
        }
        TinyTIFFReader_close(tiffr);
    } else {
        fprintf(stderr, "file format not recognized\n");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

cleanup:

    close(img_fd);

cleanup_no_close:

    if (root_obj) {
        json_object_put(root_obj);
    }
    if (img_name) {
        free(img_name);
    }
    if (img_contents) {
        free(img_contents);
    }

    return ret;
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
                errMsg("during waitpid");
                return EXIT_FAILURE;
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

uint8_t rjpg_transfer(const tgram_t * th, uint8_t * image, const uint8_t pal_id)
{
    uint32_t i = 0;
    uint16_t th_width;
    uint16_t th_height;
    uint8_t *pal_rgb;
    
    if ((th == NULL) || (image == NULL)) {
        return EXIT_FAILURE;
    }

    pal_rgb = pal_init_lut(pal_id, PAL_16BPP);
    if (pal_rgb == NULL) {
        fprintf(stderr, "palette generation error\n");
        exit(EXIT_FAILURE);
    }

    th_width = th->head.rjpg->raw_th_img_width;
    th_height = th->head.rjpg->raw_th_img_height;

    for (i = 0; i < th_width * th_height; i++) {
        memcpy(image + (i * 4), &(pal_rgb[th->framew[i] * 3]), 3);
        image[i*4 + 3] = 255; // alpha channel
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
    dst_th->framew = (uint16_t *) calloc(h->raw_th_img_sz, sizeof(uint16_t));
    if (dst_th->framew == NULL) {
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

    l_res = (l_max - l_min) / 65535.0;
    h->t_min = l_min;
    h->t_max = l_max;
    h->t_res = l_res;

    // rescale image
    for (i = 0; i < h->raw_th_img_sz; i++) {
        ftemp = ((d->temp_arr[i] - l_min) / l_res) + 0.5;
        if (ftemp < 0) {
            ftemp = 0;
        } else if (ftemp > 65535) {
            ftemp = 65535.0;
        }
        dst_th->framew[i] = ftemp;
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
