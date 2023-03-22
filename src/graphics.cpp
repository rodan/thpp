
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "proj.h"
#include "graphics.h"
#include "font_int.h"

style_t style;

void draw_pixel(canvas_t * c, const uint16_t x, const uint16_t y, const uint32_t color)
{
    if (!c->data) {
        return;
    }

    if ((x >= c->width) || (y >= c->height)) {
        return;
    }

    c->data[y * c->width + x] = color;
}

/**************************************************************************/
/*!
   @brief  vertical line drawing
   @param  x      Line horizontal start point
   @param  y      Line vertical start point
   @param  h      Length of vertical line to be drawn, including first point
   @param  color  Color to fill with
*/
/**************************************************************************/
void draw_vline(canvas_t * c, const uint16_t x, const uint16_t y, const uint16_t h,
                const uint32_t color)
{
    uint16_t i;
    uint32_t *ptr;

    if (!c->data) {
        return;
    }
    // Edge rejection (no-draw if at all off canvas)
    if ((x >= c->width) || (y >= c->height) || (y + h >= c->height)) {
        return;
    }

    ptr = c->data + y * c->width + x;

    for (i = 0; i < h; i++) {
        *ptr = color;
        ptr += c->width;
    }
}

/**************************************************************************/
/*!
   @brief  horizontal line drawing
   @param  x      Line horizontal start point
   @param  y      Line vertical start point
   @param  w      Length of horizontal line to be drawn, including first point
   @param  color  Color to fill with
*/
/**************************************************************************/
void draw_hline(canvas_t * c, const uint16_t x, const uint16_t y, const uint16_t w,
                const uint32_t color)
{
    uint32_t i;
    uint32_t start = y * c->width + x;

    if (!c->data) {
        return;
    }
    // Edge rejection (no-draw if at all off canvas)
    if ((x >= c->width) || (y >= c->height) || (x + w >= c->width)) {
        return;
    }

    for (i = 0; i < w; i++) {
        c->data[start + i] = color;
    }
}

void draw_fillrect(canvas_t * c, const uint16_t x, const uint16_t y, 
                    const uint16_t w, const uint16_t h,
                    const uint32_t color) 
{
    uint16_t i;

    for (i = x; i < x + w; i++) {
        draw_vline(c, i, y, h, color);
    }
}


void draw_major_tick(canvas_t * c, const uint16_t y)
{
    draw_hline(c, 16, y + 2, 100, style.ovl_highlight_color);
    draw_hline(c, 16, y - 2, 100, style.ovl_highlight_color);
    draw_vline(c, 116, y - 2, 4, style.ovl_highlight_color);
    draw_vline(c, 16, y - 2, 4, style.ovl_highlight_color);
}

void draw_minor_tick(canvas_t * c, const uint16_t y)
{
    draw_hline(c, 16, y, 40, style.ovl_highlight_color);
}

// Draw a character
/**************************************************************************/
/*!
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    c   The 8-bit font-indexed character (likely ascii)
    @param    color 32-bit RGBA color to draw chraracter with
    @param    bg 32-bit RGBA color to fill background with (if same as color,
   no background)
    @param    size  Font magnification level in both axes, 1 is 'original' size
*/
/**************************************************************************/
void draw_char(canvas_t * c, const uint16_t x, const uint16_t y, 
                const unsigned char ch, 
                const uint32_t color, const uint8_t size)
{
    uint8_t i, j;
    uint8_t line;

    if (1) {

        if ((x >= c->width) ||
            (y >= c->height) ||
            ((x + 6 * size - 1) < 0) ||
            ((y + 8 * size - 1) < 0)) {
            return;
        }

        for (i = 0; i < 5; i++) {        // Char bitmap = 5 columns
            line = font[ch * 5 + i];
            for (j = 0; j < 8; j++, line >>= 1) {
                if (line & 1) {
                  if (size == 1)
                    draw_pixel(c, x + i, y + j, color);
                  else
                    draw_fillrect(c, x + i * size, y + j * size, size, size, color);
                }
            }
        }
    }
}

void draw_text(canvas_t *c, const uint16_t x, const uint16_t y, char *text, const uint32_t color, const uint8_t size)
{
    uint16_t cursor_x = x;
    uint8_t i;

    for (i=0; i<strlen(text); i++) {
        draw_char(c, cursor_x, y, text[i], color, size);
        cursor_x += 6 * size;
    }
}

uint32_t highlight_color(const uint32_t color, const uint32_t color_highlight)
{
    uint8_t alpha;
    if (localhost_is_le()) {
        alpha = (color & 0xff000000) >> 24;
        if (alpha < 200) {
            return color_highlight;
        }
        return (255 << 24) |
            ((255 - ((color & 0xff0000) >> 16)) << 16) |
            ((255 - ((color & 0xff00) >> 8)) << 8) | (255 - (color & 0xff));
    } else {
        alpha = color & 0xff;
        if (alpha < 200) {
            return color_highlight;
        }
        return ((255 - ((color & 0xff000000) >> 24)) << 24) |
            ((255 - ((color & 0xff0000) >> 16)) << 16) |
            ((255 - ((color & 0xff00) >> 8)) << 8) | 255;
    }
}

void style_init(void)
{
    memset(&style, 0, sizeof(style_t));
}

void style_set(uint8_t theme)
{
    switch (theme) {
        case STYLE_DARK:
            style.theme = theme;
            style.ovl_text_color = 0xccccccff;
            style.ovl_highlight_color = 0xddddddff;
            style.plot_line_color = 0xffff00ff;
            break;
        case STYLE_LIGHT:
            style.theme = theme;
            style.ovl_text_color = 0x333333ff;
            style.ovl_highlight_color = 0x222222ff;
            style.plot_line_color = 0x111100ff;
            break;
    } 
}

style_t *get_style_ptr(void)
{
    return &style;
}

