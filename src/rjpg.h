
#ifndef __RJPG_H__
#define __RJPG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define          RJPG_SET_NEW_MIN  0x1
#define          RJPG_SET_NEW_MAX  0x2
#define     RJPG_SET_NEW_DISTANCE  0x4
#define   RJPG_SET_NEW_EMISSIVITY  0x8


uint8_t rjpg_new(tgram_t **thermo);
uint8_t rjpg_open(tgram_t *thermo, char *src_file);
void rjpg_close(tgram_t *thermo);
uint8_t rjpg_transfer(const tgram_t *thermo, uint8_t *image, const uint8_t pal, const uint8_t zoom);
uint8_t rjpg_rescale(tgram_t *dst_thermo, const tgram_t *src_thermo, const th_custom_param_t *param);

#ifdef __cplusplus
}
#endif

#endif
