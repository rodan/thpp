#ifndef __opengl_helper_h__
#define __opengl_helper_h__

#include <GLFW/glfw3.h>

/// selector size in GL units
#define GL_SELSIZE 0.05
#define GL_BOXSIZE 1.0

#define RENDER_W 1000
#define RENDER_H 1000

struct gll_bucket {
    /// glListName for the surrounding box
    GLuint ListCube;
    /// glListName for the axis
    GLuint ListCoord;
    // GLuint ListCloud;
    /// glListName for the cube shaped selector
    GLuint ListSelector;
    /// glListName for the 'L' shaped selector
    GLuint ListSelectorL;
    /// glListName for the selection plane
    GLuint ListPlane;
};

typedef struct {
    struct gll_bucket glb;
} container_t;

#ifdef __cplusplus
extern "C" {
#endif

uint8_t load_texture_from_mem(uint8_t * rgba_data, GLuint * out_texture,
                              const unsigned int image_width, const unsigned int image_height);

uint8_t load_texture_from_file(const char *filename, GLuint * out_texture,
                               unsigned int *out_width, unsigned int *out_height);

void free_textures(GLsizei n, const GLuint *textures);

#ifdef __cplusplus
}
#endif

#endif
