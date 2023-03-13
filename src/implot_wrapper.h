#ifndef __IMPLOT_WRAPPER_H__
#define __IMPLOT_WRAPPER_H__

struct linedef {
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
};

typedef linedef linedef_t;

#ifdef __cplusplus
//extern "C" {
#endif

void implot_wrapper(th_db_t *db, linedef_t *line);

#ifdef __cplusplus
//}
#endif

#endif

