
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

#include "lodepng.h"
#include "tlpi_hdr.h"
#include "proj.h"
#include "graphics.h"
#include "font_int.h"

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
    style_t *style = style_get_ptr();
    if (style) {
        draw_hline(c, 16, y + 2, 100, style->ovl_highlight_color);
        draw_hline(c, 16, y - 2, 100, style->ovl_highlight_color);
        draw_vline(c, 116, y - 2, 4, style->ovl_highlight_color);
        draw_vline(c, 16, y - 2, 4, style->ovl_highlight_color);
    }
}

void draw_minor_tick(canvas_t * c, const uint16_t y)
{
    style_t *style = style_get_ptr();
    if (style) {
        draw_hline(c, 16, y, 40, style->ovl_highlight_color);
    }
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

uint8_t image_zoom_realsr(th_rgba_t *dst, th_rgba_t *src, const uint8_t zoom)
{
    int err = 0;
    pid_t pid;
    int status;
    unsigned w, h;

    if (zoom > 4) {
        fprintf(stderr, "realsr upscaling method only goes up to 4x\n");
        return EXIT_FAILURE;
    }
    
    char tmp_src[] = "/tmp/thpp_upscale_src_XXXXXX";
    char tmp_dst[] = "/tmp/thpp_upscale_dst_XXXXXX";

    umask(077);

    err = mkstemp(tmp_src);
    if (err == -1) {
        errMsg("during mkstemp");
        return EXIT_FAILURE;
    }

    err = mkstemp(tmp_dst);
    if (err == -1) {
        errMsg("during mkstemp");
        return EXIT_FAILURE;
    }

    char *tmp_dst_ext = (char *) calloc(strlen(tmp_dst) + 5, sizeof(char));

    snprintf(tmp_dst_ext, strlen(tmp_dst) + 5, "%s.png", tmp_dst);

    err = lodepng_encode32_file(tmp_src, src->data, src->width, src->height);
    if (err) {
        fprintf(stderr, "encoder error %d: %s\n", err, lodepng_error_text(err));
    }

    switch (fork()) {
    case -1:
        errExit("fork");

    case 0:
        execlp("realesrgan-ncnn-vulkan", "realesrgan-ncnn-vulkan", "-i", tmp_src, "-o", tmp_dst_ext, "-s", "4", "-f", "png", (char *)NULL);
        exit(EXIT_SUCCESS);
    default:
        for (;;) {
            pid = waitpid(-1, &status, WUNTRACED);
            if (pid == -1) {
                errExit("during waitpid");
            }

            if (status != 0) {
                fprintf(stderr, "realesrgan-ncnn-vulkan exited in error\n");
                free(tmp_dst_ext);
                return EXIT_FAILURE;
            }

            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                err = lodepng_decode32_file(&dst->data, &w, &h, tmp_dst_ext);
                if (err) {
                    fprintf(stderr, "decoder error %d: %s\n", err, lodepng_error_text(err));
                }
                unlink(tmp_dst);
                unlink(tmp_src);
                unlink(tmp_dst_ext);
                free(tmp_dst_ext);
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_SUCCESS;
}

uint8_t image_zoom_nearest(th_rgba_t *dst, th_rgba_t *src, const uint8_t zoom)
{
    uint16_t w = src->width;
    uint16_t h = src->height;
    uint8_t zc;
    uint8_t color[4];
    uint16_t i = 0;
    uint16_t row = 0;

    for (row = 0; row < h; row++) {
        for (i = 0; i < w; i++) {
            memcpy(color, src->data + ((row * w + i) * 4), 4);
            for (zc = 0; zc < zoom; zc++) {
                // multiply each pixel zoom times
                memcpy(dst->data + ((row * w * zoom * zoom + i * zoom + zc) * 4), color, 4);
            }
        }
        for (zc = 1; zc < zoom; zc++) {
            // copy last row zoom times
            memmove(dst->data + ((row * w * zoom * zoom + zc * zoom * w) * 4),
                    dst->data + ((row * w * zoom * zoom) * 4), w * zoom * 4);
        }
    }

    return EXIT_SUCCESS;
}

uint8_t image_zoom(th_rgba_t *dst, th_rgba_t *src, const uint8_t zoom_level, const uint8_t interpolation)
{
    uint8_t ret;
    uint16_t w = src->width;
    uint16_t h = src->height;

    //printf("zoom %d %d\n", zoom_level, interpolation);
    switch (interpolation) {
        case ZOOM_INTERP_NEAREST:
        case ZOOM_INTERP_REALSR:
            if (dst->data) {
                free(dst->data);
            }
            dst->data = (uint8_t *) calloc(w * h * zoom_level * zoom_level * 4, sizeof(uint8_t));
            if (dst->data == NULL) {
                errExit("allocating buffer");
            }
            dst->width = w * zoom_level;
            dst->height = h * zoom_level;
            break;
        default:
            return EXIT_FAILURE;
            break;
    }

    if (zoom_level == 1) {
        ret = image_zoom_nearest(dst, src, zoom_level);
        return ret;
    }

    switch (interpolation) {
        case ZOOM_INTERP_NEAREST:
            ret = image_zoom_nearest(dst, src, zoom_level);
            break;
        case ZOOM_INTERP_REALSR:
            ret = image_zoom_realsr(dst, src, zoom_level);
            break;
        default:
            break;
    }
    return ret;
}

