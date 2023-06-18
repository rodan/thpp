
#ifndef __RJPG_H__
#define __RJPG_H__

#define RJPG_RET_FLIP_BYTE_ORDER  0x3

#ifdef __cplusplus
extern "C" {
#endif

uint8_t rjpg_new(tgram_t **thermo);
uint8_t rjpg_open(tgram_t *thermo, char *src_file);
void rjpg_close(tgram_t *thermo);
uint8_t rjpg_transfer(const tgram_t *thermo, uint8_t *image, const uint8_t pal);
uint8_t rjpg_rescale(th_db_t *db, uint8_t *flip_byte_order);

#ifdef __cplusplus
}
#endif

#endif
