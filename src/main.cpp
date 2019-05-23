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
#include "iml_array.h"

#include "gap_buffer.h"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

#define MS_PER_FRAME (u64)(1000 / 30)

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

struct Theme {
    TTF_Font *font;
    int glyph_width;
    int glyph_height;
    SDL_Color fg;
    SDL_Color bg;
    SDL_Color cursor_fg;
    SDL_Color cursor_bg;
};

struct Buffer {
    Gap_Buffer gap_buffer;
    // file info
    // ...
};

struct Layout;

struct Window {
    Layout *parent_layout;
    Buffer *buffer;
    // cursor should maybe be on buffer
    u64 cursor = 0; // offset from start of buffer


    SDL_Rect rect;
    SDL_Surface *surface;
};

namespace Layout_Orientation {
    enum {
        Horizontal,
        Vertical,
    };
}

struct Layout {
    enum32(Layout_Orientation) orientation;
    Layout *sub_layout1;
    Layout *sub_layout2;
    // @todo change children this to something like just left and right Layout
    // maybe one or the other has benefits. idk

    Layout *parent_layout = nullptr;
    Window *window = nullptr;

    SDL_Rect rect;
};

void render_glyph(SDL_Surface *window_surface, TTF_Font *font,
                  char ch, SDL_Color fg, SDL_Color bg,
                  int x, int y, int glyph_width, int glyph_height) {
    int width, height;
    auto surface = TTF_RenderGlyph_Shaded(font, ch, fg, bg);
    defer {
        SDL_FreeSurface(surface);
    };
    if (!surface) {
        SDL_Rect rect = {(int)x * glyph_width, (int)y * glyph_height, glyph_width, glyph_height};
        SDL_BlitSurface(NULL, NULL, window_surface, &rect);
    }
    width = surface->w;
    height = surface->h;
    SDL_Rect rect = {(int)x * glyph_width, (int)y * glyph_height, width, height};
    SDL_Rect rect_font_surface = {0,0,width,height};
    SDL_BlitSurface(surface, &rect_font_surface , window_surface, &rect);
}

// @todo absolute path
Buffer *open_file(Array<Buffer *> *buffers, char *input_file_path) {
    FILE *file;
    file = fopen(input_file_path, "r"); // test.jai demo.jai
    defer { fclose(file); };

    Buffer *buffer = (Buffer *) calloc(1, sizeof(Buffer));
    buffer->gap_buffer = make_gap_buffer(file);
    buffers->push(buffer);

    return buffer;
}

void resize_window(Window *window, int width, int height) {
    SDL_Rect rect = {0, 0, width, height};
    window->rect = rect;
    SDL_FreeSurface(window->surface);
    window->surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
}

Window *make_window(Array<Window *> *windows, int width, int height) {
    Window *window = (Window *) calloc(1, sizeof(Window));
    windows->push(window);
    resize_window(window, width, height);
    return window;
}

Layout *make_layout(Array<Layout *> *layouts, int x, int y, int width, int height, Window *window) {
    Layout *layout = (Layout *) calloc(1, sizeof(Layout));
    layouts->push(layout);
    SDL_Rect rect = {x, y, width, height};
    layout->rect = rect;
    resize_window(window, width, height);
    layout->window = window;
    layout->window->parent_layout = layout;
    return layout;
}

void split_window_horizontally(Array<Layout *> *layouts, Array<Window *> *windows, Window *current_window) {
    Layout *layout = current_window->parent_layout;

    layout->orientation = Layout_Orientation::Horizontal;
    Window *left_window = layout->window;
    layout->window = nullptr;

    Layout *left_layout = make_layout(layouts, layout->rect.x, layout->rect.y, layout->rect.w / 2, layout->rect.h, left_window);

    Window *right_window = make_window(windows, layout->rect.w / 2, layout->rect.h);
    right_window->buffer = left_window->buffer;
    Layout *right_layout = make_layout(layouts, layout->rect.x + (layout->rect.w / 2), layout->rect.y,
                                       layout->rect.w / 2, layout->rect.h, right_window);

    layout->sub_layout1  = left_layout;
    layout->sub_layout2 = right_layout;

    // this below is not needed depending on if we use and array for children or just a left and right layout
    // if !layout->window the layout is already split so split and resize all 3 childs
    // @todo if sum of children widths != layout with then just add the difference to one of the childs
}

// @copynpaste from split_window_horizontally
void split_window_vertically(Array<Layout *> *layouts, Array<Window *> *windows, Window *current_window) {
    Layout *layout = current_window->parent_layout;

    layout->orientation = Layout_Orientation::Vertical;
    Window *top_window = layout->window;
    layout->window = nullptr;

    Layout *top_layout = make_layout(layouts, layout->rect.x, layout->rect.y, layout->rect.w, layout->rect.h / 2, top_window);

    Window *bottom_window = make_window(windows, layout->rect.w, layout->rect.h / 2);
    bottom_window->buffer = top_window->buffer;
    Layout *bottom_layout = make_layout(layouts, layout->rect.x, layout->rect.y + (layout->rect.h / 2),
                                       layout->rect.w, layout->rect.h / 2, bottom_window);

    layout->sub_layout1 = top_layout;
    layout->sub_layout2 = bottom_layout;

    // this below is not needed depending on if we use and array for children or just a left and right layout
    // if !layout->window the layout is already split so split and resize all 3 childs
    // @todo if sum of children widths != layout with then just add the difference to one of the childs
}

void render_layout(Layout *layout, SDL_Surface *screen_surface, Theme *theme) {
    if (layout->window) {
        Gap_Buffer *gap_buffer = &layout->window->buffer->gap_buffer;
        gap_buffer->set_point(layout->window->cursor);

        SDL_FillRect(layout->window->surface, NULL, 0x000000);
        u64 line = 0;
        u64 offset = 0;
        char *temp = gap_buffer->buffer;
        for (int i = 0; temp < gap_buffer->buffer_end; i++) {
            if ((temp >= gap_buffer->gap_start) && (temp < gap_buffer->gap_end)) {
                temp++;
                continue;
            }
            char c = *(temp++);
            if (c == '\n') { // @todo handle soft line breaks (\r), ...
                if (temp - 1 == gap_buffer->point) {
                    render_glyph(layout->window->surface, theme->font, ' ', theme->cursor_fg, theme->cursor_bg,
                                 offset, line, theme->glyph_width, theme->glyph_height);
                }
                line++;
                offset = 0;
                continue;
            }
            if (temp - 1 == gap_buffer->point) {
                render_glyph(layout->window->surface, theme->font, c, theme->cursor_fg, theme->cursor_bg,
                             offset, line, theme->glyph_width, theme->glyph_height);
            } else {
                render_glyph(layout->window->surface, theme->font, c, theme->fg, theme->bg,
                             offset, line, theme->glyph_width, theme->glyph_height);
            }
            offset++;
        }
        SDL_Rect rect = {0,0,layout->window->rect.w,layout->window->rect.h};
        SDL_BlitSurface(layout->window->surface, &rect,
                        screen_surface, &layout->rect);
        return;
    }


    render_layout(layout->sub_layout1, screen_surface, theme);
    render_layout(layout->sub_layout2, screen_surface, theme);
}

void resize_layout(Layout *layout, SDL_Rect rect) {
    if (layout->window) {
        layout->rect = rect;
        layout->window->rect = rect;
        resize_window(layout->window, rect.w, rect.h);
        return;
    }

    layout->rect = rect;
    if (layout->orientation == Layout_Orientation::Horizontal) {
        rect.w = rect.w / 2;
        resize_layout(layout->sub_layout1, rect);
        rect.x += rect.w;
        resize_layout(layout->sub_layout2, rect);
    }
    if (layout->orientation == Layout_Orientation::Vertical) {
        rect.h = rect.h / 2;
        resize_layout(layout->sub_layout1, rect);
        rect.y += rect.h;
        resize_layout(layout->sub_layout2, rect);
    }
}

void resize_screen(Layout *layout, int new_width, int new_height) {
    SDL_Rect rect = {0, 0, new_width, new_height};
    resize_layout(layout, rect);
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
    SDL_Window *window = SDL_CreateWindow("vis", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
    assert(window);
    SDL_Surface *screen_surface = SDL_GetWindowSurface(window);
    assert(screen_surface);
    defer {
        SDL_FreeSurface(screen_surface);
        SDL_DestroyWindow(window);
        SDL_Quit();
        TTF_Quit();
    };


    
    Theme theme = {};
    theme.font = TTF_OpenFont("../run_tree/data/fonts/SourceCodePro.ttf", 16);
    assert(theme.font);
    defer { TTF_CloseFont(theme.font); };
    {
        int minx, maxx, miny, maxy, advance;
        TTF_GlyphMetrics(theme.font, L'W', &minx, &maxx, &miny, &maxy, &advance);
        theme.glyph_width = advance;
        theme.glyph_height = TTF_FontLineSkip(theme.font) + 1;
    }
    theme.fg = {255, 255, 255, 0};
    theme.bg = {0, 0, 0, 0};
    theme.cursor_fg = theme.bg;
    theme.cursor_bg = theme.fg;

    
    
    Array<Buffer *> buffers = {};
    Array<Window *> windows = {};
    Array<Layout *> layouts = {};
    defer {
        for (int i = 0; i < buffers.count; ++i) {
            free(buffers.array[i]->gap_buffer.buffer);
            free(buffers.array[i]);
        }
        for (int i = 0; i < windows.count; ++i) {
            SDL_FreeSurface(windows.array[i]->surface);
            free(windows.array[i]);
        }
        for (int i = 0; i < layouts.count; ++i) {
            free(layouts.array[i]);
        }
    };

    Layout layout = {};
    SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    layout.rect = rect;
    Window *layout_window = make_window(&windows, SCREEN_WIDTH, SCREEN_HEIGHT);
    layout.window = layout_window;
    layout.window->parent_layout = &layout;


    Buffer *buffer = open_file(&buffers, input_file_path);
    layout.window->buffer = buffer;
    
    Buffer *buffer2 = open_file(&buffers, "../tests/test.jai");

    Window *current_window = windows.array[0];
    Gap_Buffer *current_gap_buffer = &current_window->buffer->gap_buffer;

#if 1
    split_window_horizontally(&layouts, &windows, current_window);
    // @todo refactor to change buffer in current window change_buffer(window, buffer2);
    windows.array[1]->buffer = buffer2; // left_window->buffer;
#endif
#if 1
    split_window_vertically(&layouts, &windows, current_window);
#endif
#if 1
    // @todo functions to change window
    split_window_horizontally(&layouts, &windows, windows.array[1]);
#endif
#if 0
    // @todo delete current window
    // then change current window
#endif

    
    u64 cursor = 0; // offset from start of buffer

    b32 running = true;
    SDL_StartTextInput();
    defer { SDL_StopTextInput(); };
    while (running) {
        u64 start_tick = SDL_GetTicks();
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_WINDOWEVENT) {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        SDL_FreeSurface(screen_surface);
                        screen_surface = SDL_GetWindowSurface(window);
                        resize_screen(&layout, event.window.data1, event.window.data2);
                        SDL_UpdateWindowSurface(window);
                    } break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        SDL_FreeSurface(screen_surface);
                        screen_surface = SDL_GetWindowSurface(window);
                        resize_screen(&layout, event.window.data1, event.window.data2);
                        SDL_UpdateWindowSurface(window);
                    } break;
                    case SDL_WINDOWEVENT_MINIMIZED: {
                        // @todo pause editor
                    } break;
                }
                break;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: {
                        running = !(event.type == SDL_KEYDOWN);
                    } break;
                    case SDLK_RETURN: {
                        if (event.type != SDL_KEYDOWN) break;
                        current_gap_buffer->set_point(cursor);
                        current_gap_buffer->put_char('\n');
                        cursor = current_gap_buffer->point_offset();
                    } break;
                    case SDLK_BACKSPACE: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (cursor == 0)   break;
                        current_gap_buffer->set_point(cursor - 1);
                        current_gap_buffer->delete_chars(1);
                        cursor = current_gap_buffer->point_offset();
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
                            current_gap_buffer->set_point(i);
                            if (current_gap_buffer->get_char() == '\n') {
                                if (!found_offset) {
                                    line_start = i;
                                    offset = cursor - i;
                                    found_offset = true;
                                    continue;
                                } else {
                                    cursor = i + offset;
                                    if (cursor >= line_start) cursor = line_start;
                                    break;
                                }
                            }
                            if (found_offset && i == 0) {
                                cursor = offset - 1;
                                if (cursor >= line_start) cursor = line_start;
                            }
                            current_gap_buffer->set_point(cursor);
                        }
                    } break;
                    case SDLK_DOWN: {
                        if (event.type != SDL_KEYDOWN) break;
                        u64 offset = 0;
                        b32 found_offset = false;
                        for (u64 i = cursor; i-- > 0;) {
                            current_gap_buffer->set_point(i);
                            if (current_gap_buffer->get_char() == '\n' || i == 0) {
                                offset = cursor - i - 1;
                                found_offset = true;
                                break;
                            }
                        }
                        if (!found_offset && cursor != 0) break;
                        u64 next_line = 0;
                        for (u64 i = cursor; i < current_gap_buffer->sizeof_buffer(); i++) {
                            current_gap_buffer->set_point(i);
                            if (current_gap_buffer->get_char() == '\n') {
                                next_line = i + 1;
                                break;
                            }
                        }
                        cursor = next_line;
                        for (u64 i = next_line; i < (next_line + offset); i++) {
                            cursor = i + 1;
                            current_gap_buffer->set_point(i);
                            if (current_gap_buffer->get_char() == '\n') {
                                cursor--;
                                break;
                            }
                        }
                        if (cursor >= current_gap_buffer->sizeof_buffer())   cursor = current_gap_buffer->sizeof_buffer() - 1;
                        current_gap_buffer->set_point(cursor);
                    } break;
                    case SDLK_LEFT: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (cursor > 0) {
                            current_gap_buffer->set_point(cursor);
                            current_gap_buffer->previous_char();
                            cursor = current_gap_buffer->point_offset();
                            if (current_gap_buffer->get_char() == '\n' && cursor > 0)   current_gap_buffer->previous_char();
                            cursor = current_gap_buffer->point_offset();
                        }
                    } break;
                    case SDLK_RIGHT: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (cursor < current_gap_buffer->sizeof_buffer() - 1) {
                            current_gap_buffer->set_point(cursor);
                            current_gap_buffer->next_char();
                            if (current_gap_buffer->get_char() == '\n')   current_gap_buffer->next_char();
                            cursor = current_gap_buffer->point_offset();
                            if (cursor >= current_gap_buffer->sizeof_buffer())   cursor -= 2;
                            current_gap_buffer->set_point(cursor);
                        }
                    } break;
                }
                continue;
            }
            if (event.type == SDL_TEXTINPUT) {
                current_gap_buffer->set_point(cursor);
                current_gap_buffer->put_char(event.text.text[0]);
                cursor = current_gap_buffer->point_offset();
                continue;
            }
        }

        
        // Update
        current_window->cursor = cursor;

        // Render
        render_layout(&layout, screen_surface, &theme);
        SDL_UpdateWindowSurface(window);


        // sleep (or not)
        SDL_Event *events = {};
        int event_count = SDL_PeepEvents(events,
                                         1, // idk
                                         SDL_PEEKEVENT,
                                         SDL_FIRSTEVENT,
                                         SDL_LASTEVENT);
        // printf("event count: %d\n", event_count);
        u64 frame_duration_ticks = SDL_GetTicks() - start_tick;
        if (frame_duration_ticks > MS_PER_FRAME)   frame_duration_ticks = MS_PER_FRAME;
        u64 sleep_time = MS_PER_FRAME - frame_duration_ticks;
        // printf("duration ticks: %llu, sleep_time: %llu\n", frame_duration_ticks, sleep_time);
        if (event_count == 0)   SDL_Delay(sleep_time);
    }

#if 1
    current_gap_buffer->print_buffer();
#endif
#if 1
    FILE *out;
    out = fopen("../tests/test_out.jai", "wb");
    defer { fclose(out); };
    current_gap_buffer->set_point(0);
    current_gap_buffer->save_buffer_to_file(out);
#endif

    return 0;
}
