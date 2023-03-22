#ifndef __opengl_helper_h__
#define __opengl_helper_h__

/// selector size in GL units
#define GL_SELSIZE 0.05
#define GL_BOXSIZE 1.0

#define RENDER_W 1000
#define RENDER_H 1000

#ifdef __cplusplus
extern "C" {
#endif

uint8_t load_texture_from_mem(uint8_t * rgba_data, uint32_t * out_texture,
                              const unsigned int image_width, const unsigned int image_height);

uint8_t load_texture_from_file(const char *filename, uint32_t * out_texture,
                               unsigned int *out_width, unsigned int *out_height);

void free_textures(int32_t n, const uint32_t *textures);

#ifdef __cplusplus
}
#endif

#endif
