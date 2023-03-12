#ifndef __opengl_helper_h__
#define __opengl_helper_h__

#ifdef __cplusplus
extern "C" {
#endif

uint8_t load_texture_from_mem(uint8_t * rgba_data, GLuint * out_texture,
                              const unsigned int image_width, const unsigned int image_height);

uint8_t load_texture_from_file(const char *filename, GLuint * out_texture,
                               unsigned int *out_width, unsigned int *out_height);

void fb_create(const float width, const float height);

void fb_free(void);

unsigned int fb_getFrameTexture(void);

void fb_rescale(const float width, const float height);

void fb_bind(void);

void fb_unbind(void);

#ifdef __cplusplus
}
#endif

#endif
