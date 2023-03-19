#ifndef __graphics_h__
#define __graphics_h__

#define WHITE 0xffffffff  ///< color in RGBA
#define BLACK 0x000000ff  ///< color in RGBA

#define   STYLE_DARK  0
#define  STYLE_LIGHT  1

struct canvas {
    uint16_t width;
    uint16_t height;
    uint32_t *data; ///< canvas buffer, in RGBA format (4 bytes per pixel)
};
typedef struct canvas canvas_t;

struct style {
    uint8_t theme; // 0 - classic dark, 1 - light
    uint32_t ovl_text_color;
    uint32_t ovl_highlight_color;
    uint32_t plot_line_color;
    //GFXfont font;
};
typedef style style_t;

#ifdef __cplusplus
extern "C" {
#endif

void draw_pixel(canvas_t * c, const uint16_t x, const uint16_t y, const uint32_t color);
void draw_vline(canvas_t * c, const uint16_t x, const uint16_t y, const uint16_t h, const uint32_t color);
void draw_hline(canvas_t * c, const uint16_t x, const uint16_t y, const uint16_t w, const uint32_t color);
void draw_fillrect(canvas_t * c, const uint16_t x, const uint16_t y, 
                    const uint16_t w, const uint16_t h,
                    const uint32_t color);
void draw_major_tick(canvas_t * c, const uint16_t y);
void draw_minor_tick(canvas_t * c, const uint16_t y);
uint32_t highlight_color(const uint32_t color, const uint32_t color_highlight);
void draw_char(canvas_t * c, const uint16_t x, const uint16_t y, 
                const unsigned char ch, const uint32_t color, 
                const uint8_t size);
void draw_text(canvas_t *c, const uint16_t x, const uint16_t y, 
                char *text, const uint32_t color, const uint8_t size);

style_t *get_style_ptr(void);
void set_style(uint8_t theme);
void init_style(void);

#ifdef __cplusplus
}
#endif

#endif
