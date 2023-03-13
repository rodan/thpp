
#ifndef __DTV_H__
#define __DTV_H__

#ifdef __cplusplus
extern "C" {
#endif

uint8_t dtv_new(tgram_t ** thermo);
uint8_t dtv_open(tgram_t *thermo, char *dtv_file);
void dtv_close(tgram_t *thermo);
uint8_t dtv_transfer(const tgram_t *thermo, uint8_t *image, const uint8_t pal, const uint8_t zoom);
uint8_t dtv_rescale(th_db_t *db);
uint8_t dtv_populate_temp_arr(th_db_t * d);

#ifdef __cplusplus
}
#endif

#endif
