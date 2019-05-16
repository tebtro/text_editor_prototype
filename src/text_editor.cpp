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

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window *window = SDL_CreateWindow("vis", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    assert(window);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    assert(renderer);
    SDL_Surface *screen = SDL_GetWindowSurface(window);
    assert(screen);
    defer { 
        SDL_DestroyWindow(window);
        SDL_Quit();
        TTF_Quit();
    };

    TTF_Font *font = TTF_OpenFont("../assets/SourceCodePro.ttf", 32);
    assert(font);
    SDL_Color text_color = {255, 255, 255, 0};
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, "Hello Sailor!", text_color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect text_rect = {(SCREEN_WIDTH - text_surface->w) / 2, (SCREEN_HEIGHT - 30) / 2, text_surface->w, text_surface->h};
    defer {
        TTF_CloseFont(font);
        SDL_DestroyTexture(text_texture);
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

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        SDL_RenderPresent(renderer);
        /*
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
        SDL_UpdateWindowSurface(window);
        */
        /*
        SDL_UpdateTexture(screen, NULL, screen_pixels, SCREEN_WIDTH * sizeof(u32));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen, NULL, NULL);
        SDL_RenderPresent(renderer);
        */
        SDL_Delay(14);
    }

    return 0;
}
