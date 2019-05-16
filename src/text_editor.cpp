/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include <assert.h>
#include <iostream>

#include <SDL.h>
#include <SDL_ttf.h>

#include "iml_types.h"
#include "iml_general.h"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

struct Rectangle {
    int x;
    int y;
    int width;
    int height;
};

internal void fill_rect(Rectangle rect, u32 pixel_color, u32 *screen_pixels) {
    assert(screen_pixels);
    for (int row = 0; row < rect.height; ++row) {
        for (int col = 0; col < rect.width; ++col) {
            screen_pixels[(row + rect.y) * SCREEN_WIDTH + col + rect.x] = pixel_color;
        }
    }
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window *window = SDL_CreateWindow("vis", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    assert(window);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_SOFTWARE);
    assert(renderer);

    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB888);

    SDL_Texture *screen = SDL_CreateTexture(renderer, format->format, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    assert(screen);

    u32 *screen_pixels = (u32*) calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(u32));
    assert(screen_pixels);

    Rectangle square = {0, 0, 30, 30};
    square.x = (SCREEN_WIDTH  - square.width)  / 2;
    square.y = (SCREEN_HEIGHT - square.height) / 2;
    u32 pixel_color = SDL_MapRGB(format, 0, 0, 255);
    fill_rect(square, pixel_color, screen_pixels);
    
    defer { 
        SDL_DestroyWindow(window);
        SDL_Quit();
        TTF_Quit();
    };

    b32 pressed_up    = false;
    b32 pressed_down  = false;
    b32 pressed_left  = false;
    b32 pressed_right = false;
    
    b32 running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
                break;
            }
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = !(event.type == SDL_KEYDOWN);
                    break;
                case SDLK_UP:
                    pressed_up = (event.type == SDL_KEYDOWN);
                    break;
                case SDLK_DOWN:
                    pressed_down = (event.type == SDL_KEYDOWN);
                    break;
                case SDLK_LEFT:
                    pressed_left = (event.type == SDL_KEYDOWN);
                    break;
                case SDLK_RIGHT:
                    pressed_right = (event.type == SDL_KEYDOWN);
                    break;
            }
        }

        if (pressed_up)    square.y -= 1;
        if (pressed_down)  square.y += 1;
        if (pressed_left)  square.x -= 1;
        if (pressed_right) square.x += 1;

        memset(screen_pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u32));
        fill_rect(square, 255, screen_pixels);

        SDL_UpdateTexture(screen, NULL, screen_pixels, SCREEN_WIDTH * sizeof(u32));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(14);
    }

    return 0;
}
