/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

// @todo remove vector. use gap-buffer or rope... later
// @todo to utf 16 https://github.com/mika314/texteditor

#include <assert.h>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <list>

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
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    assert(renderer);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    defer {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        TTF_Quit();
    };

    TTF_Font *font = TTF_OpenFont("../assets/SourceCodePro.ttf", 32);
    assert(font);
    int glyph_width;
    {
        int minx, maxx, miny, maxy, advance;
        TTF_GlyphMetrics(font, L'W', &minx, &maxx, &miny, &maxy, &advance);
        glyph_width = advance;
    }
    int glyph_height = TTF_FontLineSkip(font) + 1;
    SDL_Color text_color = {255, 255, 255, 0};
    /*
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, "Hello Sailor!", text_color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect text_rect = {(SCREEN_WIDTH - text_surface->w) / 2, (SCREEN_HEIGHT - (int)(text_surface->h * 1.5f)) / 2, text_surface->w, text_surface->h};
    */
    defer {
        TTF_CloseFont(font);
        // SDL_DestroyTexture(text_texture);
    };

    Game_Controller_Input input = {};

    std::vector<std::vector<char>> ch_ = {{'H', 'e', 'l', 'l', 'o'}};
    ch_.push_back({});
    ch_[1].push_back('S');
    ch_[1].push_back('a');
    ch_[1].push_back('i');
    ch_[1].push_back('l');
    ch_[1].push_back('o');
    ch_[1].push_back('r');
    ch_[1].push_back('!');

    b32 running = true;
    SDL_StartTextInput();
    defer {SDL_StopTextInput();};
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) { // @todo poll or wait
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
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
                break;
            }
            if (event.type == SDL_TEXTINPUT) {
                ch_[0].push_back(event.text.text[0]);
                break;
            }
        }

        // Update
        /*
        if (input.up.is_pressed)    text_rect.y -= 1;
        if (input.down.is_pressed)  text_rect.y += 1;
        if (input.left.is_pressed)  text_rect.x -= 1;
        if (input.right.is_pressed) text_rect.x += 1;
        */

        // Render
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_Rect r = {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, texture, nullptr, &r);

        SDL_SetRenderTarget(renderer, texture);
        for (size_t y = 0; y < ch_.size(); ++y) {
            for (size_t x = 0; x < ch_[y].size(); ++x) {
                const auto &ch = ch_[y][x];
                SDL_Color fg = {255, 255, 255, 0};
                SDL_Color bg = {0, 0, 0, 0};

                SDL_Texture *texture;
                int width, height;
                auto surface = TTF_RenderGlyph_Shaded(font, ch, fg, bg);
                if (surface == nullptr) {
                    SDL_Rect rect = {(int)x * glyph_width, (int)y * glyph_height, glyph_width, glyph_height};
                    SDL_RenderFillRect(renderer, &rect);
                }
                texture = SDL_CreateTextureFromSurface(renderer, surface);
                assert(texture);
                width = surface->w;
                height = surface->h;
                SDL_FreeSurface(surface);
                SDL_Rect rect = {(int)x * glyph_width, (int)y * glyph_height, width, height};
                SDL_RenderCopy(renderer, texture, nullptr, &rect);
            }
        }
        SDL_RenderPresent(renderer);

        /*
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        SDL_RenderPresent(renderer);
        */
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
