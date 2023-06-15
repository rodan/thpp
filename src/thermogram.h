#ifndef __THERMOGRAM_H__
#define __THERMOGRAM_H__

#include "rjpg_header.h"
#include "dtv_header.h"

#define        TH_UNSET  0x0    ///< default value
#define    TH_IRTIS_DTV  0x1    ///< images generated by an IRTIS 2000 camera
#define    TH_FLIR_RJPG  0x2    ///< radiometric jpeg images generated by a Flir camera

#define     TH_DTV_VER2  0x2
#define     TH_DTV_VER3  0x3

// the following flir cameras are the exception
// they need to have the bytes from the raw thermal data samples flipped
#define       TH_FLIR_THERMACAM_E25  0x1
#define       TH_FLIR_THERMACAM_E65  0x2
#define     TH_FLIR_THERMACAM_EX320  0x3
#define            TH_FLIR_P20_NTSC  0x4
#define            TH_FLIR_S65_NTSC  0x5

#define   ID_FLIR_LITTLE_ENDIAN_MSG  "Little-endian"
#define      ID_FLIR_BIG_ENDIAN_MSG  "Big-endian"
#define       ID_FLIR_LITTLE_ENDIAN  0x1
#define          ID_FLIR_BIG_ENDIAN  0x2
#define          ID_FLIR_UNK_ENDIAN  0x0

#define       ID_FLIR_THERMACAM_E25  "ThermaCAM  E25"
#define       ID_FLIR_THERMACAM_E65  "ThermaCAM E65"
#define     ID_FLIR_THERMACAM_EX320  "ThermaCAM EX320"
#define            ID_FLIR_P20_NTSC  "P20 NTSC"
#define            ID_FLIR_S65_NTSC  "S65 NTSC"

struct tgram {
    uint16_t type;      ///< thermogram type
    uint16_t subtype;   ///< thermography model identifier
    union head_t {
        dtv_header_t *dtv;      ///< pointer to a DTV header
        rjpg_header_t *rjpg;    ///< pointer to a rjpeg header
    } head;
    uint8_t *frame;     ///< raw thermal data (if information is 8bit per pixel)
    uint16_t *framew;   ///< raw thermal data (if information is 16bit per pixel)
};

typedef struct tgram tgram_t;

#endif

