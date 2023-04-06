
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
#include "ExifTool.h"
#include "tinytiffreader.h"
#include "lodepng.h"
#include "tlpi_hdr.h"
#include "thermogram.h"
#include "palette.h"
#include "proj.h"
#include "rjpg.h"

// this compile-type define is provided to pick one of the open-source formulas to calculate the temperature from the raw data
// enable USE_GLENN_ALGO in order to use the algorithm described in https://github.com/gtatters/Thermimage
// otherwise the simplified https://github.com/kentavv/flir-batch-process algo will be applied
#define  USE_GLENN_ALGO

#define    RJPG_EXIFTOOL_BASE64_PREFIX  7       ///< number of bytes that need to be skipped during base64_decode

const uint8_t magic_number_png[8] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
const uint8_t magic_number_tiff_1[4] = { 0x49, 0x49, 0x2a, 0x00 };
const uint8_t magic_number_tiff_2[4] = { 0x4d, 0x4d, 0x00, 0x2a };

struct raw_conv {
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
};
typedef struct raw_conv raw_conv_t;

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

uint8_t rjpg_check_th_validity(tgram_t * th) {
    uint8_t ret = EXIT_SUCCESS;

    if (th == NULL) {
        return EXIT_FAILURE;
    }

    rjpg_header_t *h = th->head.rjpg;

    if ((!h->raw_th_img_sz) || (!h->raw_th_img_height) || (!h->raw_th_img_width)) {
        return EXIT_FAILURE;
    }

    return ret;
}

uint8_t rjpg_open(tgram_t * th, char *in_file)
{
    uint8_t ret = EXIT_FAILURE;
    uint32_t rti_len;
    uint8_t *rti_content = NULL;
    TagInfo *i;
    unsigned x, y;
    unsigned err = 0;
    int fd_rti;
    char fn_rti[] = "/tmp/thpp_rti_XXXXXX";

    rjpg_header_t *h = th->head.rjpg;

    // create our ExifTool object
    ExifTool *et = new ExifTool();
    // read metadata from the image
    TagInfo *info = et->ImageInfo(in_file,"-b\n-RawThermalImageWidth\n-RawThermalImageHeight\n-RawThermalImage\n-Emissivity\n-ObjectDistance\n-RelativeHumidity\n-AtmosphericTransAlpha1\n-AtmosphericTransAlpha2\n-AtmosphericTransBeta1\n-AtmosphericTransBeta2\n-PlanckR1\n-PlanckR2\n-PlanckB\n-PlanckF\n-PlanckO\n-AtmosphericTransX\n-AtmosphericTemperature\n-ReflectedApparentTemperature\n-IRWindowTransmission\n-IRWindowTemperature\n-Make\n-Model\n-CreateDate",2);

    if (!info) {
        if (et->LastComplete() <= 0) {
            fprintf(stderr, "error executing exiftool!\n");
        }
        goto cleanup;
    }

    // expected notation for floating point numbers
    setlocale(LC_ALL | ~LC_NUMERIC, "");

    for (i=info; i; i=i->next) {
        if (strstr(i->name, "RawThermalImageHeight") != NULL) {
            h->raw_th_img_height = strtol(i->value, NULL, 10);
        } else if (strstr(i->name, "RawThermalImageWidth") != NULL) {
            h->raw_th_img_width = strtol(i->value, NULL, 10);
        } else if (strstr(i->name, "Emissivity") != NULL) {
            h->emissivity = strtod(i->value, NULL);
        } else if (strstr(i->name, "ObjectDistance") != NULL) {
            h->distance = strtod(i->value, NULL);
        } else if (strstr(i->name, "RelativeHumidity") != NULL) {
            h->rh = strtod(i->value, NULL) / 100.0;
        } else if (strstr(i->name, "AtmosphericTransAlpha1") != NULL) {
            h->alpha1 = strtod(i->value, NULL);
        } else if (strstr(i->name, "AtmosphericTransAlpha2") != NULL) {
            h->alpha2 = strtod(i->value, NULL);
        } else if (strstr(i->name, "AtmosphericTransBeta1") != NULL) {
            h->beta1 = strtod(i->value, NULL);
        } else if (strstr(i->name, "AtmosphericTransBeta2") != NULL) {
            h->beta2 = strtod(i->value, NULL);
        } else if (strstr(i->name, "PlanckR1") != NULL) {
            h->planckR1 = strtod(i->value, NULL);
        } else if (strstr(i->name, "PlanckR2") != NULL) {
            h->planckR2 = strtod(i->value, NULL);
        } else if (strstr(i->name, "PlanckB") != NULL) {
            h->planckB = strtod(i->value, NULL);
        } else if (strstr(i->name, "PlanckF") != NULL) {
            h->planckF = strtod(i->value, NULL);
        } else if (strstr(i->name, "PlanckO") != NULL) {
            h->planckO = strtod(i->value, NULL);
        } else if (strstr(i->name, "AtmosphericTransX") != NULL) {
            h->atm_trans_X = strtod(i->value, NULL);
        } else if (strstr(i->name, "AtmosphericTemperature") != NULL) {
            h->air_temp = strtod(i->value, NULL);
        } else if (strstr(i->name, "ReflectedApparentTemperature") != NULL) {
            h->refl_temp = strtod(i->value, NULL);
        } else if (strstr(i->name, "IRWindowTransmission") != NULL) {
            h->iwt = strtod(i->value, NULL);
        } else if (strstr(i->name, "IRWindowTemperature") != NULL) {
            h->iwtemp = strtod(i->value, NULL);
        } else if (strstr(i->name, "Make") != NULL) {
            snprintf(h->camera_make, TAG_SZ_MAX, "%s", i->value);
        } else if (strstr(i->name, "Model") != NULL) {
            snprintf(h->camera_model, TAG_SZ_MAX, "%s", i->value);
        } else if (strstr(i->name, "CreateDate") != NULL) {
            snprintf(h->create_ts, TAG_SZ_MAX, "%s", i->value);
        } else if (strstr(i->name, "RawThermalImage") != NULL) {
            rti_len = i->valueLen;
            rti_content = (uint8_t *) calloc(rti_len, sizeof(uint8_t));
            if (rti_content == NULL) {
                errMsg("allocating rti_content");
                goto cleanup;
            }
            memcpy(rti_content, i->value, rti_len);

            if (memcmp(rti_content, magic_number_png, 8) == 0) {
                err =
                    lodepng_decode_memory((uint8_t **) & (th->framew), &x, &y, rti_content, rti_len,
                                          LCT_GREY, 16);
                if (err) {
                    fprintf(stderr, "decoder error %u: %s\n", err, lodepng_error_text(err));
                    goto cleanup;
                }
            } else if ((memcmp(rti_content, magic_number_tiff_1, 4) == 0) ||
                    (memcmp(rti_content, magic_number_tiff_2, 4) == 0)) {
                // we got a tiff image and the lib cannot directly read from mem
                umask(077);
                fd_rti = mkstemp(fn_rti);
                if (fd_rti < 0) {
                    errMsg("during mkstemp(%s)", fn_rti);
                    goto cleanup;
                }

                if ((fd_rti = open(fn_rti, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
                    errMsg("opening file %s", fn_rti);
                    goto cleanup;
                }

                if (write(fd_rti, rti_content, rti_len) < rti_len) {
                    errMsg("writing");
                    close(fd_rti);
                    goto cleanup;
                }

                close(fd_rti);

                TinyTIFFReaderFile *tiffr = NULL;
                tiffr = TinyTIFFReader_open(fn_rti);
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
            } // end tiff
        } // end of strstr()
    } // for (i = info ...)

    h->wr = 0;
    h->raw_th_img_sz = h->raw_th_img_width * h->raw_th_img_height;

    err = rjpg_check_th_validity(th);
    if (err) {
        goto cleanup;
    }

    if (strlen(h->camera_model) > 2) {
        if (memcmp
            (h->camera_model, ID_FLIR_THERMACAM_E25,
             min(strlen(h->camera_model), strlen(ID_FLIR_THERMACAM_E25))) == 0) {
            th->subtype = TH_FLIR_THERMACAM_E25;
        } else if (memcmp
                   (h->camera_model, ID_FLIR_THERMACAM_E65,
                    min(strlen(h->camera_model), strlen(ID_FLIR_THERMACAM_E65))) == 0) {
            th->subtype = TH_FLIR_THERMACAM_E65;
        } else if (memcmp
                   (h->camera_model, ID_FLIR_THERMACAM_EX320,
                    min(strlen(h->camera_model), strlen(ID_FLIR_THERMACAM_EX320))) == 0) {
            th->subtype = TH_FLIR_THERMACAM_EX320;
        } else if (memcmp
                   (h->camera_model, ID_FLIR_P20_NTSC,
                    min(strlen(h->camera_model), strlen(ID_FLIR_P20_NTSC))) == 0) {
            th->subtype = TH_FLIR_P20_NTSC;
        } else if (memcmp
                   (h->camera_model, ID_FLIR_S65_NTSC,
                    min(strlen(h->camera_model), strlen(ID_FLIR_S65_NTSC))) == 0) {
            th->subtype = TH_FLIR_S65_NTSC;
        }
    }

    ret = EXIT_SUCCESS;

cleanup:

    if (rti_content) {
        free(rti_content);
    }

    if (info) {
        delete info;
    }

    char *serr = et->GetError();
    if (serr) {
        fprintf(stderr, "%s\n", serr);
    }
    delete et;
    return ret;
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

double rjpg_calc_glenn(raw_conv_t *r, uint16_t raw)
{
    double raw_obj;
    double ret;

    raw_obj =
        (1.0 * raw / r->E / r->tau1 / r->IWT / r->tau2 - r->raw_atm1_attn - r->raw_atm2_attn - r->raw_wind_attn -
         r->raw_refl1_attn - r->raw_refl2_attn);
    ret = r->PB / log(r->PR1 / (r->PR2 * (raw_obj + r->PO)) + r->PF) - 273.15;

    return ret;
}

#else
double rjpg_calc_distcomp(raw_conv_t *r, uint16_t raw)
{
    double raw_obj;
    double ret;

    raw_obj = (1.0 * raw - r->tau_raw_atm - r->epsilon_tau_raw_refl) / r->E / r->tau;
    ret = r->PB / log(r->PR1 / (r->PR2 * (raw_obj + r->PO)) + r->PF) - RJPG_K;

    return ret;
}

double rjpg_calc_nodistcomp(raw_conv_t *r, uint16_t raw)
{
    double raw_obj;
    double ret;

    raw_obj = (1.0 * raw - r->ep_raw_refl) / r->E;
    ret = r->PB / log(r->PR1 / (r->PR2 * (raw_obj + r->PO)) + r->PF) - RJPG_K;

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
    double t_obj;
    raw_conv_t *r = NULL;
    uint8_t ret = EXIT_FAILURE;
    rjpg_header_t *h = dst_th->head.rjpg;

    // populate dst thermo header
    memcpy((uint8_t *) dst_th->head.rjpg, (uint8_t *) src_th->head.rjpg, sizeof(rjpg_header_t));

    // alloc frame mem
    dst_th->framew = (uint16_t *) calloc(h->raw_th_img_sz, sizeof(uint16_t));
    if (dst_th->framew == NULL) {
        errMsg("allocating buffer");
        goto cleanup;
    }

    if (d->temp_arr != NULL) {
        free(d->temp_arr);
    }

    // alloc float calculation buffer
    d->temp_arr = (double *)calloc(h->raw_th_img_sz, sizeof(double));
    if (d->temp_arr == NULL) {
        errMsg("allocating buffer");
        goto cleanup;
    }

    h->t_min = 32000.0;
    h->t_max = -32000.0;

    // alloc conversion structure
    r = (raw_conv_t *) calloc(1, sizeof(raw_conv_t));
    if (r == NULL) {
        errMsg("allocating buffer");
        goto cleanup;
    }

    if (p->flags & OPT_SET_NEW_DISTANCE) {
        r->OD = p->distance;
    } else {
        r->OD = h->distance;
    }

    if (p->flags & OPT_SET_NEW_EMISSIVITY) {
        r->E = p->emissivity;
    } else {
        r->E = h->emissivity;
    }

    if (p->flags & OPT_SET_NEW_AT) {
        r->ATemp = p->atm_temp;
    } else {
        r->ATemp = h->air_temp;
    }

    if (p->flags & OPT_SET_NEW_RT) {
        r->RTemp = p->refl_temp;
    } else {
        r->RTemp = h->refl_temp;
    }

    if (p->flags & OPT_SET_NEW_RH) {
        r->RH = p->rh;
    } else {
        r->RH = h->rh;
    }

    if (p->flags & OPT_SET_NEW_IWT) {
        r->IWT = p->iwt;
    } else {
        r->IWT = h->iwt;
    }

    if (p->flags & OPT_SET_NEW_IWTEMP) {
        r->IRWTemp = p->iwtemp;
    } else {
        r->IRWTemp = h->iwtemp;
    }

    if (p->flags & OPT_SET_NEW_WR) {
        r->WR = p->wr;
    } else {
        r->WR = h->wr;
    }

    r->RTemp = h->refl_temp;
    r->PR1 = h->planckR1;
    r->PB = h->planckB;
    r->PF = h->planckF;
    r->PO = h->planckO;
    r->PR2 = h->planckR2;
    r->ATA1 = h->alpha1;
    r->ATA2 = h->alpha2;
    r->ATB1 = h->beta1;
    r->ATB2 = h->beta2;
    r->ATX = h->atm_trans_X;

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

    r->h2o =
        r->RH * exp(1.5587 + 0.06939 * r->ATemp - 0.00027816 * pow(r->ATemp, 2) +
                 0.00000068455 * pow(r->ATemp, 3));
    r->tau1 =
        r->ATX * exp(-sqrt(r->OD / 2) * (r->ATA1 + r->ATB1 * sqrt(r->h2o))) + (1 -
            r->ATX) * exp(-sqrt(r->OD / 2) * (r->ATA2 + r->ATB2 * sqrt(r->h2o)));
    r->tau2 = r->tau1;

    r->raw_refl1 = r->PR1 / (r->PR2 * (exp(r->PB / (r->RTemp + 273.15)) - r->PF)) - r->PO;
    r->raw_refl1_attn = (1 - r->E) / r->E * r->raw_refl1;

    r->raw_atm1 = r->PR1 / (r->PR2 * (exp(r->PB / (r->ATemp + 273.15)) - r->PF)) - r->PO;
    r->raw_atm1_attn = (1 - r->tau1) / r->E / r->tau1 * r->raw_atm1;

    r->raw_wind = r->PR1 / (r->PR2 * (exp(r->PB / (r->IRWTemp + 273.15)) - r->PF)) - r->PO;
    r->raw_wind_attn = (1.0 - r->IWT) / r->E / r->tau1 / r->IWT * r->raw_wind;

    r->raw_refl2 = r->raw_refl1;
    r->raw_refl2_attn = r->WR / r->E / r->tau1 / r->IWT * r->raw_refl2;

    r->raw_atm2 = r->raw_atm1;
    r->raw_atm2_attn = (1 - r->tau2) / r->E / r->tau1 / r->IWT / r->tau2 * r->raw_atm2;

    for (i = 0; i < h->raw_th_img_sz; i++) {
        if (framew_needs_flippage) {
            raw = htons(src_th->framew[i]);
        } else {
            raw = src_th->framew[i];
        }

        t_obj = rjpg_calc_glenn(r, raw);
        d->temp_arr[i] = t_obj;
        if (h->t_min > t_obj) {
            h->t_min = t_obj;
        }
        if (h->t_max < t_obj) {
            h->t_max = t_obj;
        }
        t_acc += raw;
    }

    h->t_avg = rjpg_calc_glenn(r, t_acc / h->raw_th_img_sz);

#else
    // algorithm expects all temperature variable input in degrees K

    r->RTemp += RJPG_K;
    r->ATemp += RJPG_K;

    if (p->flags & OPT_SET_COMP) {
        //if (1) {
        r->h2o =
            r->RH * exp(1.5587 + 0.06939 * r->ATemp - 0.00027816 * pow(r->ATemp, 2) +
                     0.00000068455 * pow(r->ATemp, 3));
        r->tau =
            r->ATX * exp(-sqrt(r->OD) * (r->ATA1 + r->ATB1 * sqrt(r->h2o))) + 
               (1 - r->ATX) * exp(-sqrt(r->OD) * (r->ATA2 + r->ATB2 * sqrt(r->h2o)));
        r->raw_atm = r->PR1 / (r->PR2 * (exp(r->PB / r->ATemp) - r->PF)) - r->PO;
        r->tau_raw_atm = r->raw_atm * (1 - r->tau);
        r->raw_refl = r->PR1 / (r->PR2 * (exp(r->PB / (r->RTemp)) - r->PF)) - r->PO;
        r->epsilon_tau_raw_refl = r->raw_refl * (1 - r->E) * r->tau;
        t_acc = 0;

        for (i = 0; i < h->raw_th_img_sz; i++) {
            if (framew_needs_flippage) {
                raw = htons(src_th->framew[i]);
            } else {
                raw = src_th->framew[i];
            }

            t_obj = rjpg_calc_distcomp(r, raw);
            d->temp_arr[i] = t_obj;
            if (h->t_min > t_obj) {
                h->t_min = t_obj;
            }
            if (h->t_max < t_obj) {
                h->t_max = t_obj;
            }
            t_acc += raw;
        }
        h->t_avg = rjpg_calc_distcomp(r, t_acc / h->raw_th_img_sz);
    } else {
        r->raw_refl = r->PR1 / (r->PR2 * (exp(r->PB / r->RTemp) - r->PF)) - r->PO;
        r->ep_raw_refl = r->raw_refl * (1 - r->E);
        t_acc = 0;

        for (i = 0; i < h->raw_th_img_sz; i++) {
            if (framew_needs_flippage) {
                // data in the exif-png gets here in big endian words
                raw = htons(src_th->framew[i]);
            } else {
                raw = src_th->framew[i];
            }
            t_obj = rjpg_calc_nodistcomp(r, raw);
            d->temp_arr[i] = t_obj;
            if (h->t_min > t_obj) {
                h->t_min = t_obj;
            }
            if (h->t_max < t_obj) {
                h->t_max = t_obj;
            }
            t_acc += raw;
        }
        h->t_avg = rjpg_calc_nodistcomp(r, t_acc / h->raw_th_img_sz);
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
        goto cleanup;
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

    ret = EXIT_SUCCESS;

    //rjpg_temp2csv(d);

cleanup:
    if (r) {
        free(r);
    }

    return ret;
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
