#ifndef __DTV_HEADER_H__
#define __DTV_HEADER_H__

#define DTV_HEADER_SZ  110

struct dtv_header {
    uint8_t disz;               ///< 0 header size?
    uint8_t diver;              ///< 1 header version?
    uint16_t shtp;              ///< 2
    int8_t fltp;                ///< 4
    int8_t tc;                  ///< 5
    int8_t cftime[4];           ///< 6
    uint16_t nst;               ///< 10 horizontal resolution
    uint16_t nstv;              ///< 12 vertical resolution
    uint16_t shtph;             ///< 14
    int16_t frn;                ///< 16 number of frames
    uint16_t nstl;              ///< 18
    uint8_t bgx;                ///< 20
    uint8_t bgy;                ///< 21
    float hszk;                 ///< 22
    float vszk;                 ///< 26
    float frtk;                 ///< 30
    float tsc[3];               ///< 34 temperature resolution, min, max
    char inform[64];            ///< 46
    float ttmin;                ///< 100
    float ttmax;                ///< 104
    char unknown;               ///< 108
} __attribute__((packed));

typedef struct dtv_header dtv_header_t;

#endif
