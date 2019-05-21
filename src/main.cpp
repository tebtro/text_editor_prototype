/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

// @todo remove vector. use better datastructure 
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

struct Button_State {
    b32 is_pressed;
};

struct Keyboard_Input {
    union {
        Button_State buttons[4];
        struct {
            Button_State up;
            Button_State down;
            Button_State left;
            Button_State right;
        };
    };
};

void render_glyph(SDL_Renderer *renderer, TTF_Font *font,
                  char ch, SDL_Color fg, SDL_Color bg,
                  int x, int y, int glyph_width, int glyph_height) {
    SDL_Texture *texture;
    int width, height;
    auto surface = TTF_RenderGlyph_Shaded(font, ch, fg, bg);
    defer {
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    };
    if (surface == nullptr) {
        SDL_Rect rect = {(int)x * glyph_width, (int)y * glyph_height, glyph_width, glyph_height};
        SDL_RenderFillRect(renderer, &rect);
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    assert(texture);
    width = surface->w;
    height = surface->h;
    SDL_Rect rect = {(int)x * glyph_width, (int)y * glyph_height, width, height};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
}

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

    TTF_Font *font = TTF_OpenFont("../run_tree/data/fonts/SourceCodePro.ttf", 32);
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

    Keyboard_Input input = {};

    char *buffer = "\nHello\nSailor\nThis is a text editor\nJon\n";
    u64 cursor = 0; // offset from start of buffer
    
    SDL_Color fg = {255, 255, 255, 0};
    SDL_Color bg = {0, 0, 0, 0};

    SDL_Color cursor_fg = bg;
    SDL_Color cursor_bg = fg;
    
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
                    case SDLK_ESCAPE: {
                        running = !(event.type == SDL_KEYDOWN);
                    } break;
                    case SDLK_RETURN: {
                        if (event.type != SDL_KEYDOWN) break;
                        /*
                        if (cursor.line + 2 > ch_.size()) {
                            ch_.push_back({});
                            cursor.line++;
                            cursor.offset = 0;
                        }
                        */
                    } break;
                    case SDLK_BACKSPACE: {
                        if (event.type != SDL_KEYDOWN) break;
                        /*
                        if (cursor.line == 0 && cursor.offset == 0) break;
                        if (cursor.offset > 0) {
                            ch_[cursor.line].erase(ch_[cursor.line].begin() + --cursor.offset);
                            break;
                        }
                        if (cursor.offset == 0 && cursor.line > 0) {
                            if (ch_[cursor.line].size() == 0) ch_.erase(ch_.begin() + cursor.line);
                            cursor.line--;
                            cursor.offset = ch_[cursor.line].size();
                            // ch_[cursor.line].erase(ch_[cursor.line].begin() + --cursor.offset);
                        }
                        */
                    } break;
                    case SDLK_UP: {
                        if (event.type != SDL_KEYDOWN) break;
                        u64 line_start = 0;
                        u64 offset = 0;
                        b32 found_offset = false;
                        for (u64 i = cursor; i-- > 0;) {
                            if (buffer[i] == '\n') {
                                if (!found_offset) {
                                    line_start = i;
                                    offset = cursor - i;
                                    found_offset = true;
                                    continue;
                                } else {
                                    cursor = i + offset;
                                    if (cursor >= line_start) cursor = line_start - 1;
                                    break;
                                }
                            }
                            if (found_offset && i == 0) {
                                cursor = offset - 1;
                                if (cursor >= line_start) cursor = line_start - 1;
                            }
                        }
                    } break;
                    case SDLK_DOWN: {
                        if (event.type != SDL_KEYDOWN) break;
                        u64 offset = 0;
                        b32 found_offset = false;
                        for (u64 i = cursor; i-- > 0;) {
                            if (buffer[i] == '\n' || i == 0) {
                                offset = cursor - i - 1;
                                found_offset = true;
                                break;
                            }
                        }
                        if (!found_offset && cursor != 0) break;
                        u64 next_line = 0;
                        for (u64 i = cursor; i < strlen(buffer); i++) {
                            if (buffer[i] == '\n') {
                                next_line = i + 1;
                                break;
                            }
                        }
                        cursor = next_line;
                        for (u64 i = next_line; i < (next_line + offset); i++) {
                            cursor = i + 1;
                            if (buffer[i] == '\n') {
                                cursor--;
                                break;
                            }
                        }
                        if (cursor >= strlen(buffer)) cursor = strlen(buffer) - 1;
                    } break;
                    case SDLK_LEFT: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (cursor > 0) {
                            cursor--;
                            if (buffer[cursor] == '\n' && cursor > 0) cursor--;
                        }
                    } break;
                    case SDLK_RIGHT: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (cursor < strlen(buffer) - 1) {
                            cursor++;
                            if (buffer[cursor] == '\n') cursor++;
                            if (cursor >= strlen(buffer)) cursor -= 2;
                        }
                    } break;
                }
                break;
            }
            if (event.type == SDL_TEXTINPUT) {
                /*
                ch_[cursor.line].push_back(event.text.text[0]);
                cursor.offset++;
                */
                break;
            }
        }

        // Update

        // Render
        b32 cursor_rendered = false;


        SDL_SetRenderTarget(renderer, texture);
        SDL_RenderClear(renderer);
        u64 line = 0;
        u64 offset = 0;
        for (int i = 0; i < strlen(buffer); i++) {
            char c = buffer[i];
            if (c == '\n') { // @todo handle soft line breaks, ...
                if (i == cursor) render_glyph(renderer, font, ' ', bg, fg, offset, line, glyph_width, glyph_height);
                line++;
                offset = 0;
                continue;
            }
            if (i == cursor) {
                render_glyph(renderer, font, c, bg, fg, offset, line, glyph_width, glyph_height);
            } else {
                render_glyph(renderer, font, c, fg, bg, offset, line, glyph_width, glyph_height);
            }
            offset++;
        }
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_Rect r = {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, texture, nullptr, &r);
        SDL_RenderPresent(renderer);
        
        SDL_Delay(32);
    }

    return 0;
}
