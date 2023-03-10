#ifndef __THERMOGRAM_H__
#define __THERMOGRAM_H__

#include "rjpg_header.h"
#include "dtv_header.h"

#define        TH_UNSET  0x0    ///< default value
#define    TH_IRTIS_DTV  0x1    ///< images generated by an IRTIS 2000 camera
#define    TH_FLIR_RJPG  0x2    ///< radiometric jpeg images generated by a Flir camera

struct tgram {
    uint16_t type;      ///< thermogram type
    union head_t {
        dtv_header_t *dtv;      ///< pointer to a DTV header
        rjpg_header_t *rjpg;    ///< pointer to a rjpeg header
    } head;
    uint8_t *frame;     ///< raw thermal data
};

typedef struct tgram tgram_t;

#endif

