
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <math.h>
#include <arpa/inet.h>
#include <locale.h>
#include "tinytiffreader.h"
#include "apr_base64.h"
#include "json_helper.h"
#include "lodepng.h"
#include "tlpi_hdr.h"
#include "thermogram.h"
#include "palette.h"
#include "proj.h"
#include "rjpg.h"

// the digital-to-analog conversion algorithm is not provided by Flir
// this compile-type define is provided to pick one of the open-source formulas to calculate the temperature from the raw data
// enable USE_GLENN_ALGO in order to use the algorithm described in https://github.com/gtatters/Thermimage
// otherwise the simplified https://github.com/kentavv/flir-batch-process algo will be applied
#define USE_GLENN_ALGO

#define    RJPG_EXIFTOOL_BASE64_PREFIX  7       ///< number of bytes that need to be skipped during base64_decode

const uint8_t magic_number_png[8] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
const uint8_t magic_number_tiff_1[4] = { 0x49, 0x49, 0x2a, 0x00 };
const uint8_t magic_number_tiff_2[4] = { 0x4d, 0x4d, 0x00, 0x2a };

double E;                       ///< Emissivity - default 1, should be above 0.95 for sources that resemble a black body
double OD;                      ///< Object Distance in metres
double RTemp;                   ///< apparent Reflected Temperature - one value from FLIR file (dC), default 20dC
double ATemp;                   ///< Atmospheric Temperature for transmission loss - one value from FLIR file (dC) - default = RTemp
double IRWTemp;                 ///< Infrared Window Temperature - default = RTemp (dC)
double IWT;                     ///< Infrared Window Transmission - default 1.  likely ~0.95-0.96. Should be empirically determined.
double RH;                      ///< Relative humidity - float between 0 and 1
double PR1;                     ///< PlanckR1 calibration constant
double PB;                      ///< PlanckB calibration constant
double PF;                      ///< PlanckF calibration constant
double PO;                      ///< PlanckO calibration constant
double PR2;                     ///< PlanckR2 calibration constant
double ATA1;                    ///< Atmospheric Trans Alpha 1
double ATA2;                    ///< Atmospheric Trans Alpha 2
double ATB1;                    ///< Atmospheric Trans Beta 1
double ATB2;                    ///< Atmospheric Trans Beta 2
double ATX;                     ///< Atmospheric Trans X
double WR;                      ///< Window Reflectivity (0 for window covered in anti-reflective coating)

double h2o;

#ifdef USE_GLENN_ALGO
double tau1, tau2;
double raw_refl1, raw_refl2;
double raw_refl1_attn, raw_refl2_attn;
double raw_atm1, raw_atm2, raw_wind;
double raw_atm1_attn, raw_atm2_attn, raw_wind_attn;
#else
double tau;
double raw_atm;
double tau_raw_atm;
double raw_refl;
double epsilon_tau_raw_refl;
double ep_raw_refl;
#endif

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
    char *camera_make;
    char *camera_model;
    char *rti;
    uint8_t ret = EXIT_FAILURE;
    int img_fd = -1;

    rjpg_header_t *h = th->head.rjpg;

    root_obj = json_object_from_file(json_file);

    if (root_obj == NULL) {
        fprintf(stderr, "unable to parse json file\n");
        goto cleanup;
    }

    item_obj = json_object_array_get_idx(root_obj, 0);
    if (item_obj == NULL) {
        fprintf(stderr, "unable to parse json file\n");
        goto cleanup;
    }

    setlocale(LC_ALL | ~LC_NUMERIC, "");

    rti = json_get(item_obj, "RawThermalImage");
    if (rti == NULL) {
        fprintf(stderr, "RawThermalImage tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "Emissivity", &h->emissivity) != EXIT_SUCCESS) {
        fprintf(stderr, "Emissivity tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "ObjectDistance", &h->distance) != EXIT_SUCCESS) {
        fprintf(stderr, "ObjectDistance tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "RelativeHumidity", &h->rh) != EXIT_SUCCESS) {
        fprintf(stderr, "RelativeHumidity tag is missing\n");
        goto cleanup;
    }
    h->rh /= 100.0;

    if (json_getd(item_obj, "AtmosphericTransAlpha1", &h->alpha1) != EXIT_SUCCESS) {
        fprintf(stderr, "AtmosphericTransAlpha1 tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "AtmosphericTransAlpha2", &h->alpha2) != EXIT_SUCCESS) {
        fprintf(stderr, "AtmosphericTransAlpha2 tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "AtmosphericTransBeta1", &h->beta1) != EXIT_SUCCESS) {
        fprintf(stderr, "AtmosphericTransBeta1 tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "AtmosphericTransBeta2", &h->beta2) != EXIT_SUCCESS) {
        fprintf(stderr, "AtmosphericTransBeta2 tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "PlanckR1", &h->planckR1) != EXIT_SUCCESS) {
        fprintf(stderr, "PlanckR1 tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "PlanckR2", &h->planckR2) != EXIT_SUCCESS) {
        fprintf(stderr, "PlanckR2 tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "PlanckB", &h->planckB) != EXIT_SUCCESS) {
        fprintf(stderr, "PlanckB tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "PlanckF", &h->planckF) != EXIT_SUCCESS) {
        fprintf(stderr, "PlanckF tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "PlanckO", &h->planckO) != EXIT_SUCCESS) {
        fprintf(stderr, "PlanckO tag is missing\n");
        goto cleanup;
    }

    if (json_getd(item_obj, "AtmosphericTransX", &h->atm_trans_X) != EXIT_SUCCESS) {
        fprintf(stderr, "AtmosphericTransX tag is missing\n");
        goto cleanup;
    }

    // air_temp keeps temperature in degrees C
    if (json_getd(item_obj, "AtmosphericTemperature", &h->air_temp) != EXIT_SUCCESS) {
        fprintf(stderr, "AtmosphericTemperature tag is missing\n");
        goto cleanup;
    }

    // refl_temp keeps temperature in degrees C
    if (json_getd(item_obj, "ReflectedApparentTemperature", &h->refl_temp) != EXIT_SUCCESS) {
        fprintf(stderr, "ReflectedApparentTemperature tag is missing\n");
        goto cleanup;
    }

    if (json_getw(item_obj, "RawThermalImageWidth", &h->raw_th_img_width) != EXIT_SUCCESS) {
        fprintf(stderr, "RawThermalImageWidth tag is missing\n");
        goto cleanup;
    }

    if (json_getw(item_obj, "RawThermalImageHeight", &h->raw_th_img_height) != EXIT_SUCCESS) {
        fprintf(stderr, "RawThermalImageHeight tag is missing\n");
        goto cleanup;
    }

    h->iwt = 1.0;
    h->wr = 0;

    camera_make = json_get(item_obj, "Make");
    camera_model = json_get(item_obj, "Model");

    snprintf(h->camera_make, 32, "%s", camera_make);
    snprintf(h->camera_model, 32, "%s", camera_model);

    if (strlen(camera_model) > 2) {
        if (memcmp
            (camera_model, ID_FLIR_THERMACAM_E25,
             min(strlen(camera_model), strlen(ID_FLIR_THERMACAM_E25))) == 0) {
            th->subtype = TH_FLIR_THERMACAM_E25;
        } else if (memcmp
                   (camera_model, ID_FLIR_THERMACAM_E65,
                    min(strlen(camera_model), strlen(ID_FLIR_THERMACAM_E65))) == 0) {
            th->subtype = TH_FLIR_THERMACAM_E65;
        } else if (memcmp
                   (camera_model, ID_FLIR_THERMACAM_EX320,
                    min(strlen(camera_model), strlen(ID_FLIR_THERMACAM_EX320))) == 0) {
            th->subtype = TH_FLIR_THERMACAM_EX320;
        } else if (memcmp
                   (camera_model, ID_FLIR_P20_NTSC,
                    min(strlen(camera_model), strlen(ID_FLIR_P20_NTSC))) == 0) {
            th->subtype = TH_FLIR_P20_NTSC;
        } else if (memcmp
                   (camera_model, ID_FLIR_S65_NTSC,
                    min(strlen(camera_model), strlen(ID_FLIR_S65_NTSC))) == 0) {
            th->subtype = TH_FLIR_S65_NTSC;
        }
    }

    // fill raw_th_img
    decode_len = apr_base64_decode_len(rti + RJPG_EXIFTOOL_BASE64_PREFIX);

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

    apr_base64_decode((char *)img_contents, rti + RJPG_EXIFTOOL_BASE64_PREFIX);

    snprintf(img_name, strlen(json_file) + 5, "%s.img", json_file);

    if ((img_fd = open(img_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
        errMsg("opening file %s", img_name);
        goto cleanup_no_close;
    }

    if (write(img_fd, img_contents, decode_len) < decode_len) {
        errMsg("writing");
        goto cleanup;
    }

    h->raw_th_img_sz = h->raw_th_img_width * h->raw_th_img_height;

    if (memcmp(img_contents, magic_number_png, 8) == 0) {
        // th->framew gets allocated by lodepng_decode_memory()
        err =
            lodepng_decode_memory((uint8_t **) & (th->framew), &x, &y, img_contents, decode_len,
                                  LCT_GREY, 16);
        if (err) {
            fprintf(stderr, "decoder error %u: %s\n", err, lodepng_error_text(err));
            goto cleanup;
        }
        ret = EXIT_SUCCESS;
    } else if ((memcmp(img_contents, magic_number_tiff_1, 4) == 0) ||
               (memcmp(img_contents, magic_number_tiff_2, 4) == 0)) {

        TinyTIFFReaderFile *tiffr = NULL;
        tiffr = TinyTIFFReader_open(img_name);
        if (!tiffr) {
            fprintf(stderr, "decoder error TinyTIFFReader_open() has failed\n");
            goto cleanup;
        } else {
            const uint32_t tiff_width = TinyTIFFReader_getWidth(tiffr);
            const uint32_t tiff_height = TinyTIFFReader_getHeight(tiffr);

            if ((tiff_width != h->raw_th_img_width) || (tiff_height != h->raw_th_img_height)) {
                fprintf(stderr, "unexpected image size %u != %u or %u ! %u\n", tiff_width,
                        h->raw_th_img_width, tiff_height, h->raw_th_img_height);
                goto cleanup;
            }

            th->framew = (uint16_t *) calloc(tiff_width * tiff_height, sizeof(uint16_t));
            TinyTIFFReader_getSampleData(tiffr, th->framew, 0);
        }
        TinyTIFFReader_close(tiffr);
        ret = EXIT_SUCCESS;
    }

 cleanup:

    if (img_name) {
        unlink(img_name);
    }
    if (img_fd >= 0) {
        close(img_fd);
    }

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
        image[i * 4 + 3] = 255; // alpha channel
    }

    return EXIT_SUCCESS;
}

#ifdef USE_GLENN_ALGO

double rjpg_calc_glenn(uint16_t raw)
{
    double raw_obj;
    double ret;

    raw_obj =
        (1.0 * raw / E / tau1 / IWT / tau2 - raw_atm1_attn - raw_atm2_attn - raw_wind_attn -
         raw_refl1_attn - raw_refl2_attn);
    ret = PB / log(PR1 / (PR2 * (raw_obj + PO)) + PF) - 273.15;

    return ret;
}

#else
double rjpg_calc_distcomp(uint16_t raw)
{
    double raw_obj;
    double ret;

    raw_obj = (1.0 * raw - tau_raw_atm - epsilon_tau_raw_refl) / E / tau;
    ret = PB / log(PR1 / (PR2 * (raw_obj + PO)) + PF) - RJPG_K;

    return ret;
}

double rjpg_calc_nodistcomp(uint16_t raw)
{
    double raw_obj;
    double ret;

    raw_obj = (1.0 * raw - ep_raw_refl) / E;
    ret = PB / log(PR1 / (PR2 * (raw_obj + PO)) + PF) - RJPG_K;

    return ret;
}
#endif


// this function can be used to generate a csv file with the temperature matrix of an image 
// this can be used as raw2temp algorithm validation by comparing to a similar csv generated by the Flir software
// see https://github.com/gtatters/ThermimageCalibration
void rjpg_temp2csv(th_db_t * d)
{
    uint16_t x,y;

    if (d->out_th == NULL) {
        return;
    }

    rjpg_header_t *h = d->out_th->head.rjpg;

    for (y=0; y<h->raw_th_img_height; y++) {
        for (x=0; x<h->raw_th_img_width; x++) {
            printf("%.06le", d->temp_arr[y*h->raw_th_img_width + x]);
            if (x < h->raw_th_img_width - 1) {
                printf(",");
            }
        }
        printf("\n");
    }
}

uint8_t rjpg_rescale(th_db_t * d)
{
    ssize_t i;
    uint8_t framew_needs_flippage = 0;
    uint16_t raw;
    double ftemp;
    double l_min;
    double l_max;
    double l_res;
    int64_t t_acc = 0;
    tgram_t *src_th = d->in_th;
    tgram_t *dst_th = d->out_th;
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
        OD = p->distance;
    } else {
        OD = h->distance;
    }

    if (p->flags & OPT_SET_NEW_EMISSIVITY) {
        E = p->emissivity;
    } else {
        E = h->emissivity;
    }

    if (p->flags & OPT_SET_NEW_AT) {
        ATemp = p->atm_temp;
    } else {
        ATemp = h->air_temp;
    }

    if (p->flags & OPT_SET_NEW_RT) {
        RTemp = p->refl_temp;
    } else {
        RTemp = h->refl_temp;
    }

    if (p->flags & OPT_SET_NEW_RH) {
        RH = p->rh;
    } else {
        RH = h->rh;
    }

    if (p->flags & OPT_SET_NEW_IWT) {
        IWT = p->iwt;
    } else {
        IWT = h->iwt;
    }

    if (p->flags & OPT_SET_NEW_WR) {
        WR = p->wr;
    } else {
        WR = h->wr;
    }

    RTemp = h->refl_temp;
    IRWTemp = RTemp;
    PR1 = h->planckR1;
    PB = h->planckB;
    PF = h->planckF;
    PO = h->planckO;
    PR2 = h->planckR2;
    ATA1 = h->alpha1;
    ATA2 = h->alpha2;
    ATB1 = h->beta1;
    ATB2 = h->beta2;
    ATX = h->atm_trans_X;

    switch (src_th->subtype) {
    case TH_FLIR_THERMACAM_E25:
    case TH_FLIR_THERMACAM_E65:
    case TH_FLIR_THERMACAM_EX320:
    case TH_FLIR_P20_NTSC:
    case TH_FLIR_S65_NTSC:
        if (localhost_is_le()) {
            framew_needs_flippage = 1;
        }
        break;
    default:
        if (!localhost_is_le()) {
            framew_needs_flippage = 1;
        }
    }

#ifdef USE_GLENN_ALGO
    // algorithm expects all temperature variable input in degrees C

    h2o =
        RH * exp(1.5587 + 0.06939 * ATemp - 0.00027816 * pow(ATemp, 2) +
                 0.00000068455 * pow(ATemp, 3));
    tau1 =
        ATX * exp(-sqrt(OD / 2) * (ATA1 + ATB1 * sqrt(h2o))) + (1 -
                                                                ATX) * exp(-sqrt(OD / 2) * (ATA2 +
                                                                                            ATB2 *
                                                                                            sqrt
                                                                                            (h2o)));
    tau2 = tau1;

    raw_refl1 = PR1 / (PR2 * (exp(PB / (RTemp + 273.15)) - PF)) - PO;
    raw_refl1_attn = (1 - E) / E * raw_refl1;

    raw_atm1 = PR1 / (PR2 * (exp(PB / (ATemp + 273.15)) - PF)) - PO;
    raw_atm1_attn = (1 - tau1) / E / tau1 * raw_atm1;

    raw_wind = PR1 / (PR2 * (exp(PB / (IRWTemp + 273.15)) - PF)) - PO;
    raw_wind_attn = (1.0 - IWT) / E / tau1 / IWT * raw_wind;

    raw_refl2 = raw_refl1;
    raw_refl2_attn = WR / E / tau1 / IWT * raw_refl2;

    raw_atm2 = raw_atm1;
    raw_atm2_attn = (1 - tau2) / E / tau1 / IWT / tau2 * raw_atm2;

    for (i = 0; i < h->raw_th_img_sz; i++) {
        if (framew_needs_flippage) {
            raw = htons(src_th->framew[i]);
        } else {
            raw = src_th->framew[i];
        }

        double t_obj = rjpg_calc_glenn(raw);
        d->temp_arr[i] = t_obj;
        if (h->t_min > t_obj) {
            h->t_min = t_obj;
        }
        if (h->t_max < t_obj) {
            h->t_max = t_obj;
        }
        t_acc += raw;
    }
    h->t_avg = rjpg_calc_glenn(t_acc / h->raw_th_img_sz);

#else
    // algorithm expects all temperature variable input in degrees K

    RTemp += RJPG_K;
    ATemp += RJPG_K;

    if (p->flags & OPT_SET_COMP) {
        //if (1) {
        h2o =
            RH * exp(1.5587 + 0.06939 * ATemp - 0.00027816 * pow(ATemp, 2) +
                     0.00000068455 * pow(ATemp, 3));
        tau =
            ATX * exp(-sqrt(OD) * (ATA1 + ATB1 * sqrt(h2o))) + (1 -
                                                                ATX) * exp(-sqrt(OD) * (ATA2 +
                                                                                        ATB2 *
                                                                                        sqrt(h2o)));
        raw_atm = PR1 / (PR2 * (exp(PB / ATemp) - PF)) - PO;
        tau_raw_atm = raw_atm * (1 - tau);
        raw_refl = PR1 / (PR2 * (exp(PB / (RTemp)) - PF)) - PO;
        epsilon_tau_raw_refl = raw_refl * (1 - E) * tau;
        t_acc = 0;

        for (i = 0; i < h->raw_th_img_sz; i++) {
            if (framew_needs_flippage) {
                raw = htons(src_th->framew[i]);
            } else {
                raw = src_th->framew[i];
            }

            double t_obj_dc = rjpg_calc_distcomp(raw);
            d->temp_arr[i] = t_obj_dc;
            if (h->t_min > t_obj_dc) {
                h->t_min = t_obj_dc;
            }
            if (h->t_max < t_obj_dc) {
                h->t_max = t_obj_dc;
            }
            t_acc += raw;
        }
        h->t_avg = rjpg_calc_distcomp(t_acc / h->raw_th_img_sz);
    } else {
        raw_refl = PR1 / (PR2 * (exp(PB / RTemp) - PF)) - PO;
        ep_raw_refl = raw_refl * (1 - E);
        t_acc = 0;

        for (i = 0; i < h->raw_th_img_sz; i++) {
            if (framew_needs_flippage) {
                // data in the exif-png gets here in big endian words
                raw = htons(src_th->framew[i]);
            } else {
                raw = src_th->framew[i];
            }
            double t_obj = rjpg_calc_nodistcomp(raw);
            d->temp_arr[i] = t_obj;
            if (h->t_min > t_obj) {
                h->t_min = t_obj;
            }
            if (h->t_max < t_obj) {
                h->t_max = t_obj;
            }
            t_acc += raw;
        }
        h->t_avg = rjpg_calc_nodistcomp(t_acc / h->raw_th_img_sz);
    }
#endif

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

    //rjpg_temp2csv(d);

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
