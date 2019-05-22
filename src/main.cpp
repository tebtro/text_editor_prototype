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

#include "gap_buffer.h"

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

void render_glyph(SDL_Surface *window_surface, TTF_Font *font,
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
        SDL_BlitSurface(NULL, NULL, window_surface, &rect);
    }
    width = surface->w;
    height = surface->h;
    SDL_Rect rect = {(int)x * glyph_width, (int)y * glyph_height, width, height};
    SDL_Rect rect_font_surface = {0,0,width,height};
    SDL_BlitSurface(surface, &rect_font_surface , window_surface, &rect);
}

int main(int argc, char *argv[]) {
    char *input_file_path = nullptr;

    for (int i = 1; i < argc; ++i) {
        printf("argument[%d]: %s\n", i, argv[i]);
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            // @todo print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input-file-path") == 0) {
            input_file_path = argv[++i];
        } else {
            if (!input_file_path) {
                input_file_path = argv[i];
            }
        }
    }
    if (!input_file_path) {
        printf("error: no input file!\n");
        return -1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window *window = SDL_CreateWindow("vis", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    assert(window);
    SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    assert(window_surface);
    SDL_Surface *buffer1_surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
    SDL_Rect buffer1_rect = {0,0,SCREEN_WIDTH / 2,SCREEN_HEIGHT};
    SDL_Surface *buffer2_surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
    SDL_Rect buffer2_rect = {SCREEN_WIDTH / 2,0,SCREEN_WIDTH / 2,SCREEN_HEIGHT};
    defer {
        SDL_FreeSurface(buffer1_surface);
        SDL_FreeSurface(buffer2_surface);
        SDL_DestroyWindow(window);
        SDL_Quit();
        TTF_Quit();
    };

    TTF_Font *font = TTF_OpenFont("../run_tree/data/fonts/SourceCodePro.ttf", 16);
    assert(font);
    int glyph_width;
    {
        int minx, maxx, miny, maxy, advance;
        TTF_GlyphMetrics(font, L'W', &minx, &maxx, &miny, &maxy, &advance);
        glyph_width = advance;
    }
    int glyph_height = TTF_FontLineSkip(font) + 1;
    SDL_Color text_color = {255, 255, 255, 0};
    defer { TTF_CloseFont(font); };

    Keyboard_Input input = {};

    u64 cursor = 0; // offset from start of buffer

    FILE *file;
    file = fopen(input_file_path, "r"); // test.jai demo.jai
    defer { fclose(file); };
    Gap_Buffer gap_buffer = Gap_Buffer::make_gap_buffer(file);
    
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
                        gap_buffer.set_point(cursor);
                        gap_buffer.put_char('\n');
                        cursor = gap_buffer.point_offset();
                    } break;
                    case SDLK_BACKSPACE: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (cursor == 0)   break;
                        gap_buffer.set_point(cursor - 1);
                        gap_buffer.delete_chars(1);
                        cursor = gap_buffer.point_offset();
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
                            gap_buffer.set_point(i);
                            if (gap_buffer.get_char() == '\n') {
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
                            gap_buffer.set_point(cursor);
                        }
                    } break;
                    case SDLK_DOWN: {
                        if (event.type != SDL_KEYDOWN) break;
                        u64 offset = 0;
                        b32 found_offset = false;
                        for (u64 i = cursor; i-- > 0;) {
                            gap_buffer.set_point(i);
                            if (gap_buffer.get_char() == '\n' || i == 0) {
                                offset = cursor - i - 1;
                                found_offset = true;
                                break;
                            }
                        }
                        if (!found_offset && cursor != 0) break;
                        u64 next_line = 0;
                        for (u64 i = cursor; i < gap_buffer.sizeof_buffer(); i++) {
                            gap_buffer.set_point(i);
                            if (gap_buffer.get_char() == '\n') {
                                next_line = i + 1;
                                break;
                            }
                        }
                        cursor = next_line;
                        for (u64 i = next_line; i < (next_line + offset); i++) {
                            cursor = i + 1;
                            gap_buffer.set_point(i);
                            if (gap_buffer.get_char() == '\n') {
                                cursor--;
                                break;
                            }
                        }
                        if (cursor >= gap_buffer.sizeof_buffer())   cursor = gap_buffer.sizeof_buffer() - 1;
                        gap_buffer.set_point(cursor);
                    } break;
                    case SDLK_LEFT: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (cursor > 0) {
                            // gap_buffer.set_point(cursor);
                            gap_buffer.previous_char();
                            cursor = gap_buffer.point_offset();
                            if (gap_buffer.get_char() == '\n' && cursor > 0)   gap_buffer.previous_char();
                            cursor = gap_buffer.point_offset();
                        }
                    } break;
                    case SDLK_RIGHT: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (cursor < gap_buffer.sizeof_buffer() - 1) {
                            // gap_buffer.set_point(cursor);
                            gap_buffer.next_char();
                            if (gap_buffer.get_char() == '\n')   gap_buffer.next_char();
                            cursor = gap_buffer.point_offset();
                            /*
                            if (cursor >= gap_buffer.sizeof_buffer())   cursor -= 2;
                            gap_buffer.set_point(cursor);
                            */
                        }
                    } break;
                }
                break;
            }
            if (event.type == SDL_TEXTINPUT) {
                gap_buffer.set_point(cursor);
                gap_buffer.put_char(event.text.text[0]);
                cursor = gap_buffer.point_offset();
                break;
            }
        }

        // Update

        // Render
        b32 cursor_rendered = false;

        SDL_FillRect(buffer1_surface, NULL, 0x000000);
        SDL_FillRect(buffer2_surface, NULL, 0x072627);

        u64 line = 0;
        u64 offset = 0;
        char *temp = gap_buffer.buffer;
        for (int i = 0; temp < gap_buffer.buffer_end; i++) {
            if ((temp >= gap_buffer.gap_start) && (temp < gap_buffer.gap_end)) {
                temp++;
                continue;
            }
            char c = *(temp++);
            if (c == '\n') { // @todo handle soft line breaks (\r), ...
                line++;
                offset = 0;
                continue;
            }
            if (temp - 1 == gap_buffer.point) {
                render_glyph(buffer1_surface, font, c, cursor_fg, cursor_bg, offset, line, glyph_width, glyph_height);
            } else {
                render_glyph(buffer1_surface, font, c, fg, bg, offset, line, glyph_width, glyph_height);
            }
            offset++;
        }

        SDL_Rect rect_full = {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
        SDL_BlitSurface(buffer1_surface, &rect_full , window_surface, &buffer1_rect);
        SDL_BlitSurface(buffer2_surface, &rect_full , window_surface, &buffer2_rect);
        SDL_UpdateWindowSurface(window);
        
        SDL_Delay(32);
    }

#if 1
    gap_buffer.print_buffer();
#endif
#if 1
    FILE *out;
    out = fopen("../tests/test_out.jai", "wb");
    defer { fclose(out); };
    gap_buffer.set_point(0);
    gap_buffer.save_buffer_to_file(out);
#endif

    return 0;
}
