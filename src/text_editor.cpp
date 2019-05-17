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

struct Game_Button_State {
    b32 is_pressed;
};

struct Game_Controller_Input {
    union {
        Game_Button_State buttons[4];
        struct {
            Game_Button_State up;
            Game_Button_State down;
            Game_Button_State left;
            Game_Button_State right;
        };
    };
};

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
    SDL_Rect text_rect = {(SCREEN_WIDTH - text_surface->w) / 2, (SCREEN_HEIGHT - (int)(text_surface->h * 1.5f)) / 2, text_surface->w, text_surface->h};
    defer {
        TTF_CloseFont(font);
        SDL_DestroyTexture(text_texture);
    };

    Game_Controller_Input input = {};
    
    b32 running = true;
    SDL_StartTextInput();
    defer {SDL_StopTextInput();};
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_TEXTINPUT) printf("%s\n", event.text.text);
            if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
                break;
            }
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = !(event.type == SDL_KEYDOWN);
                    break;
                case SDLK_UP:
                    input.up.is_pressed = (event.type == SDL_KEYDOWN);
                    break;
                case SDLK_DOWN:
                    input.down.is_pressed = (event.type == SDL_KEYDOWN);
                    break;
                case SDLK_LEFT:
                    input.left.is_pressed = (event.type == SDL_KEYDOWN);
                    break;
                case SDLK_RIGHT:
                    input.right.is_pressed = (event.type == SDL_KEYDOWN);
                    break;
            }
        }

        // Update
        if (input.up.is_pressed)    text_rect.y -= 1;
        if (input.down.is_pressed)  text_rect.y += 1;
        if (input.left.is_pressed)  text_rect.x -= 1;
        if (input.right.is_pressed) text_rect.x += 1;
        
        // Render
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
        SDL_Delay(32);
    }

    return 0;
}
