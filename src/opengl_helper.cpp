
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "proj.h"
#include "lodepng.h"
#include "gl_utils.h"
#include "opengl_helper.h"

uint32_t fbo;
uint32_t fb_texture;
uint32_t rbo;

extern GLFWwindow *application_window;

uint8_t opengl_init(void)
{
    uint32_t err = 0;

    restart_gl_log();
    err += start_gl(application_window);

    return err;
}

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
    glGenerateMipmap( GL_TEXTURE_2D );

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);        // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);        // Same
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

    glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso );
    // set the maximum!
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso );

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

void fb_create(const float width, const float height)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &fb_texture);
    glBindTexture(GL_TEXTURE_2D, fb_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);        // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);        // Same
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_texture, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void fb_free(void)
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fb_texture);
    glDeleteRenderbuffers(1, &rbo);
}

unsigned int *fb_get_texture_ptr(void)
{
    return &fb_texture;
}

unsigned int fb_get_texture(void)
{
    return fb_texture;
}

void fb_rescale(const float width, const float height)
{
    glBindTexture(GL_TEXTURE_2D, fb_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_texture, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
}

void fb_bind(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void fb_unbind(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fb_init(void)
{
    glewInit();
    fb_create(RENDER_W, RENDER_H);
}

