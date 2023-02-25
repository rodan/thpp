
#ifndef __DTV_H__
#define __DTV_H__

#ifdef __cplusplus
extern "C" {
#endif

uint8_t dtv_open(tgram_t *thermo, char *dtv_file);
void dtv_close(tgram_t *thermo);
uint8_t dtv_transfer(const tgram_t *thermo, uint8_t *image, const uint8_t pal, const uint8_t zoom);
uint8_t dtv_rescale(tgram_t *dst_thermo, const tgram_t *src_thermo, const float new_min, const float new_max);

#ifdef __cplusplus
}
#endif

#endif
