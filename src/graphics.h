#ifndef __graphics_h__
#define __graphics_h__

#define WHITE 0xffffffff  ///< color in RGBA

struct canvas {
    uint16_t width; ///< canvas width
    uint16_t height; ///< canvas height
    uint8_t rotation;
    uint32_t *data; ///< canvas buffer, in RGBA format (4 bytes per pixel)
};

typedef struct canvas canvas_t;

#ifdef __cplusplus
extern "C" {
#endif

void draw_pixel(canvas_t * c, uint16_t x, uint16_t y, const uint32_t color);
void draw_vline(canvas_t * c, uint16_t x, uint16_t y, uint16_t h, const uint32_t color);
void draw_hline(canvas_t * c, uint16_t x, uint16_t y, uint16_t w, const uint32_t color);


#ifdef __cplusplus
}
#endif

#endif
