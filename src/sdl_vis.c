
#ifdef CONFIG_SDL

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#ifdef CONFIG_SDL_OPENGL
#include <GL/glew.h>
#endif

#include "proj.h"
#include "sdl_vis.h"

uint8_t sdl_vis_file(const char *file_name, const uint16_t width, const uint16_t height)
{
    uint8_t quit = 0;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    SDL_Window *window = NULL;
    SDL_Event event;

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

    SDL_Rect rect;
    rect.x = 0; // x position of thermal image in the window
    rect.y = 0; // y position of thermal image in the window
    rect.w = width;
    rect.h = height;

#ifdef CONFIG_SDL_OPENGL
    SDL_GLContext gl_context;

    window = SDL_CreateWindow(
        __FILE__, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width + 100, height, SDL_WINDOW_OPENGL
    );
    renderer = SDL_CreateRenderer(window, 0, 0);
    gl_context = SDL_GL_CreateContext(window);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
#else
    //SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);
    window = SDL_CreateWindow(
        __FILE__, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width + 100, height, SDL_WINDOW_ALLOW_HIGHDPI
    );
    renderer = SDL_CreateRenderer(window, 0, 0);
#endif

    IMG_Init(IMG_INIT_PNG);
    texture = IMG_LoadTexture(renderer, file_name);
    SDL_SetWindowTitle(window, "output visualisation");
    while (!quit) {
        SDL_RenderCopy(renderer, texture, NULL, &rect);
#ifdef CONFIG_SDL_OPENGL
        SDL_GL_SwapWindow(window);
#else
        SDL_RenderPresent(renderer);
#endif
        usleep(16000);
        while (SDL_PollEvent(&event) == 1) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = 1;
                    break;
                default:
                    break;
                }
            } else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = 1;
                default:
                    break;
                }
            }
        }

    }
#ifdef CONFIG_SDL_OPENGL
    SDL_GL_DeleteContext(gl_context);
#endif
    SDL_DestroyTexture(texture);
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}

#endif
