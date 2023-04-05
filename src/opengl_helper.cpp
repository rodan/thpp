
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
//#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "proj.h"
#include "lodepng.h"
#include "opengl_helper.h"

uint8_t load_texture_from_mem(uint8_t * rgba_data, GLuint * out_texture,
                              const unsigned int image_width, const unsigned int image_height)
{
    GLfloat max_aniso = 0.0f;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 rgba_data);
    //glGenerateMipmap(GL_TEXTURE_2D);

    //printf("tex %d %dx%d\n", *out_texture, image_width, image_height);

    // Setup filtering parameters for display

    // special treatement for non-power-of-two textures
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
    // set the maximum!
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);

    *out_texture = image_texture;

    return EXIT_SUCCESS;
}

uint8_t load_texture_from_file(const char *filename, GLuint * out_texture, unsigned int *out_width,
                               unsigned int *out_height)
{
    unsigned int image_width = 0;
    unsigned int image_height = 0;
    unsigned char *image_data = NULL;

    //unsigned char *image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    lodepng_decode32_file(&image_data, &image_width, &image_height, filename);
    if (image_data == NULL) {
        return EXIT_FAILURE;
    }

    load_texture_from_mem(image_data, out_texture, image_width, image_height);

    free(image_data);
    *out_width = image_width;
    *out_height = image_height;

    return EXIT_SUCCESS;
}

void free_textures(GLsizei n, const GLuint * textures)
{
    glDeleteTextures(n, textures);
}

