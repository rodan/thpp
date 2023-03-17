
#include <stdint.h>
#include "graphics.h"

void draw_pixel(canvas_t * c, uint16_t x, uint16_t y, const uint32_t color)
{
    uint16_t t;

    if (!c->data) {
        return;
    }

    if ((x >= c->width) || (y >= c->height)) {
        return;
    }

    switch (c->rotation) {
    case 1:
        t = x;
        x = c->width - 1 - y;
        y = t;
        break;
    case 2:
        x = c->width - 1 - x;
        y = c->height - 1 - y;
        break;
    case 3:
        t = x;
        x = y;
        y = c->height - 1 - t;
        break;
    }

    c->data[y * c->width + x] = color;
}

/**************************************************************************/
/*!
   @brief    Speed optimized vertical line drawing into the raw canvas buffer
   @param    x   Line horizontal start point
   @param    y   Line vertical start point
   @param    h   length of vertical line to be drawn, including first point
   @param    color   Binary (on or off) color to fill with
*/
/**************************************************************************/
void draw_raw_vline(canvas_t * c, uint16_t x, uint16_t y, uint16_t h, const uint32_t color)
{
    uint16_t i;
    uint32_t *ptr;

    ptr = c->data + y * c->width + x;
    
    for (i = 0; i<h; i++) {
        *ptr = color;
        ptr += c->width;
    }
}

/**************************************************************************/
/*!
   @brief    Speed optimized horizontal line drawing into the raw canvas buffer
   @param    x   Line horizontal start point
   @param    y   Line vertical start point
   @param    w   length of horizontal line to be drawn, including first point
   @param    color   Binary (on or off) color to fill with
*/
/**************************************************************************/
void draw_raw_hline(canvas_t * c, uint16_t x, uint16_t y, uint16_t w, const uint32_t color)
{
    uint16_t i;
    uint16_t start = y * c->width + x;

    for (i = 0; i<w; i++) {
        c->data[start + i] = color;
    }
}

/**************************************************************************/
/*!
   @brief  Speed optimized vertical line drawing
   @param  x      Line horizontal start point
   @param  y      Line vertical start point
   @param  h      Length of vertical line to be drawn, including first point
   @param  color  Color to fill with
*/
/**************************************************************************/
void draw_vline(canvas_t * c, uint16_t x, uint16_t y, uint16_t h, const uint32_t color)
{
    uint16_t t;

    if (!c->data) {
        return;
    }

    // Edge rejection (no-draw if totally off canvas)
    if ((x >= c->width) || (y >= c->height) || (y + h >= c->height)) {
        return;
    }

    switch (c->rotation) {
    case 0:
        draw_raw_vline(c, x, y, h, color);
        break;
    case 1:
        t = x;
        x = c->width - 1 - y;
        y = t;
        x -= h - 1;
        draw_raw_hline(c, x, y, h, color);
        break;
    case 2:
        x = c->width - 1 - x;
        y = c->height - 1 - y;
        y -= h - 1;
        draw_raw_vline(c, x, y, h, color);
        break;
    case 3:
        t = x;
        x = y;
        y = c->height - 1 - t;
        draw_raw_hline(c, x, y, h, color);
        break;
    }
}

/**************************************************************************/
/*!
   @brief  Speed optimized horizontal line drawing
   @param  x      Line horizontal start point
   @param  y      Line vertical start point
   @param  w      Length of horizontal line to be drawn, including first point
   @param  color  Color to fill with
*/
/**************************************************************************/
void draw_hline(canvas_t * c, uint16_t x, uint16_t y, uint16_t w, const uint32_t color)
{
    uint16_t t;

    if (!c->data) {
        return;
    }

    // Edge rejection (no-draw if totally off canvas)
    if ((x >= c->width) || (y >= c->height) || (x + w >= c->width)) {
        return;
    }

    switch (c->rotation) {
    case 0:
        draw_raw_hline(c, x, y, w, color);
        break;
    case 1:
        t = x;
        x = c->width - 1 - y;
        y = t;
        draw_raw_vline(c, x, y, w, color);
        break;
    case 2:
        x = c->width - 1 - x;
        y = c->height - 1 - y;
        x -= w - 1;
        draw_raw_hline(c, x, y, w, color);
        break;
    case 3:
        t = x;
        x = y;
        y = c->height - 1 - t;
        y -= w - 1;
        draw_raw_vline(c, x, y, w, color);
        break;
    }
}


