
#ifndef __RJPG_H__
#define __RJPG_H__

#ifdef __cplusplus
extern "C" {
#endif

uint8_t rjpg_new(tgram_t **thermo);
uint8_t rjpg_open(tgram_t *thermo, char *src_file);
void rjpg_close(tgram_t *thermo);
uint8_t rjpg_transfer(const tgram_t *thermo, uint8_t *image, const uint8_t pal, const uint8_t zoom);
uint8_t rjpg_rescale(tgram_t *dst_thermo, const tgram_t *src_thermo, double new_min, double new_max);

#ifdef __cplusplus
}
#endif

#endif
