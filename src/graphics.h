#ifndef __graphics_h__
#define __graphics_h__

#define WHITE 0xffffffff  ///< color in RGBA
#define BLACK 0x000000ff  ///< color in RGBA

struct canvas {
    uint16_t width;
    uint16_t height;
    uint32_t *data; ///< canvas buffer, in RGBA format (4 bytes per pixel)
};
typedef struct canvas canvas_t;

#ifdef __cplusplus
extern "C" {
#endif

void draw_pixel(canvas_t * c, const uint16_t x, const uint16_t y, const uint32_t color);
void draw_vline(canvas_t * c, const uint16_t x, const uint16_t y, const uint16_t h, const uint32_t color);
void draw_hline(canvas_t * c, const uint16_t x, const uint16_t y, const uint16_t w, const uint32_t color);
void draw_fillrect(canvas_t * c, const uint16_t x, const uint16_t y, 
                    const uint16_t w, const uint16_t h,
                    const uint32_t color);
void draw_major_tick(canvas_t * c, const uint16_t y, const uint32_t color);
void draw_minor_tick(canvas_t * c, const uint16_t y, const uint32_t color);
uint32_t highlight_color(const uint32_t color, const uint32_t color_highlight);
void draw_char(canvas_t * c, const uint16_t x, const uint16_t y, 
                const unsigned char ch, const uint32_t color, 
                const uint8_t size);
void draw_text(canvas_t *c, const uint16_t x, const uint16_t y, 
                char *text, const uint32_t color, const uint8_t size);

uint8_t image_zoom(th_rgba_t *dst, th_rgba_t *src, const uint8_t zoom, const uint8_t interp);

#ifdef __cplusplus
}
#endif

#endif
