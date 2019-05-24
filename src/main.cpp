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

struct Editor {
    SDL_Window  *window;
    SDL_Surface *screen_surface;

    Theme *theme;

    Array<Buffer *> buffers;
    Array<Window *> windows;
    Array<Layout *> layouts;

    Layout root_layout;

    Window     *current_window;
    Gap_Buffer *current_gap_buffer;
    u64         current_cursor;
};

// @todo absolute path
Buffer *open_file(Editor *editor, char *input_file_path) {
    FILE *file;
    file = fopen(input_file_path, "r"); // test.jai demo.jai
    defer { fclose(file); };

    Buffer *buffer = (Buffer *) calloc(1, sizeof(Buffer));
    buffer->gap_buffer = make_gap_buffer(file);
    editor->buffers.push(buffer);

    return buffer;
}

void resize_window(Window *window, int width, int height) {
    SDL_Rect rect = {0, 0, width, height};
    window->rect = rect;
    SDL_FreeSurface(window->surface);
    window->surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
}

Window *make_window(Editor *editor, int width, int height) {
    Window *window = (Window *) calloc(1, sizeof(Window));
    editor->windows.push(window);
    resize_window(window, width, height);
    return window;
}

Layout *make_layout(Editor *editor, int x, int y, int width, int height, Window *window) {
    Layout *layout = (Layout *) calloc(1, sizeof(Layout));
    editor->layouts.push(layout);
    SDL_Rect rect = {x, y, width, height};
    layout->rect = rect;
    resize_window(window, width, height);
    layout->window = window;
    layout->window->parent_layout = layout;
    return layout;
}

void split_window_horizontally(Editor *editor, Window *window) {
    Layout *layout = window->parent_layout;

    layout->orientation = Layout_Orientation::Horizontal;
    Window *left_window = layout->window;
    layout->window = nullptr;

    Layout *left_layout = make_layout(editor, layout->rect.x, layout->rect.y, layout->rect.w / 2, layout->rect.h, left_window);

    Window *right_window = make_window(editor, layout->rect.w / 2, layout->rect.h);
    right_window->buffer = left_window->buffer;
    Layout *right_layout = make_layout(editor, layout->rect.x + (layout->rect.w / 2), layout->rect.y,
                                       layout->rect.w / 2, layout->rect.h, right_window);

    layout->sub_layout1  = left_layout;
    layout->sub_layout2 = right_layout;


    assert(layout->rect.w == (left_layout->rect.w + right_layout->rect.w));
    // this below is not needed depending on if we use and array for children or just a left and right layout
    // if !layout->window the layout is already split so split and resize all 3 childs
    // @todo if sum of children widths != layout with then just add the difference to one of the childs
}

void split_current_window_horizontally(Editor *editor) {
    split_window_horizontally(editor, editor->current_window);
}

// @copynpaste from split_window_horizontally
void split_window_vertically(Editor *editor, Window *window) {
    Layout *layout = window->parent_layout;

    layout->orientation = Layout_Orientation::Vertical;
    Window *top_window = layout->window;
    layout->window = nullptr;

    Layout *top_layout = make_layout(editor, layout->rect.x, layout->rect.y, layout->rect.w, layout->rect.h / 2, top_window);

    Window *bottom_window = make_window(editor, layout->rect.w, layout->rect.h / 2);
    bottom_window->buffer = top_window->buffer;
    Layout *bottom_layout = make_layout(editor, layout->rect.x, layout->rect.y + (layout->rect.h / 2),
                                        layout->rect.w, layout->rect.h / 2, bottom_window);

    layout->sub_layout1 = top_layout;
    layout->sub_layout2 = bottom_layout;

    
    assert(layout->rect.h == (top_layout->rect.h + bottom_layout->rect.h));
    // this below is not needed depending on if we use and array for children or just a left and right layout
    // if !layout->window the layout is already split so split and resize all 3 childs
    // @todo if sum of children widths != layout with then just add the difference to one of the childs
}

void split_current_window_vertically(Editor *editor) {
    split_window_vertically(editor, editor->current_window);
}

void render_glyph(SDL_Surface *window_surface, Theme *theme,
                  char ch, int x, int y, b32 render_cursor = false) {
    int glyph_width  = theme->glyph_width;
    int glyph_height = theme->glyph_height;
    int width, height;
    SDL_Color fg = theme->fg;
    SDL_Color bg = theme->bg;
    if (render_cursor) {
        fg = theme->bg;
        bg = theme->fg;
    }
    auto surface = TTF_RenderGlyph_Shaded(theme->font, ch, fg, bg);
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

void render_layout(Editor *editor, Layout *layout) {
    SDL_Surface *screen_surface = editor->screen_surface;
    Theme *theme = editor->theme;
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
                    render_glyph(layout->window->surface, theme, ' ', offset, line, true);
                }
                line++;
                offset = 0;
                continue;
            }
            if (temp - 1 == gap_buffer->point) {
                render_glyph(layout->window->surface, theme, c, offset, line, true);
            } else {
                render_glyph(layout->window->surface, theme, c, offset, line);
            }
            offset++;
        }
        SDL_Rect rect = {0,0,layout->window->rect.w,layout->window->rect.h};
        SDL_BlitSurface(layout->window->surface, &rect,
                        screen_surface, &layout->rect);
        return;
    }


    render_layout(editor, layout->sub_layout1);
    render_layout(editor, layout->sub_layout2);
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

    Editor *editor = (Editor *) calloc(1, sizeof(Editor));
    defer { free(editor); };
    // @todo create a method to free everything
    
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    editor->window = SDL_CreateWindow("vis", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
    assert(editor->window);
    editor->screen_surface = SDL_GetWindowSurface(editor->window);
    assert(editor->screen_surface);
    defer {
        SDL_FreeSurface(editor->screen_surface);
        SDL_DestroyWindow(editor->window);
        SDL_Quit();
        TTF_Quit();
    };


    { 
        Theme *theme = (Theme *) calloc(1, sizeof(Theme));
        editor->theme = theme;
        theme->font = TTF_OpenFont("../run_tree/data/fonts/SourceCodePro.ttf", 16);
        assert(theme->font);
        {
            int minx, maxx, miny, maxy, advance;
            TTF_GlyphMetrics(theme->font, L'W', &minx, &maxx, &miny, &maxy, &advance);
            theme->glyph_width = advance;
            theme->glyph_height = TTF_FontLineSkip(theme->font) + 1;
        }
        theme->fg = {255, 255, 255, 0};
        theme->bg = {0, 0, 0, 0};
        theme->cursor_fg = theme->bg;
        theme->cursor_bg = theme->fg;
    }
    defer { TTF_CloseFont(editor->theme->font); };

    
    
    editor->buffers = {};
    editor->windows = {};
    editor->layouts = {};
    defer {
        for (int i = 0; i < editor->buffers.count; ++i) {
            free(editor->buffers.array[i]->gap_buffer.buffer);
            free(editor->buffers.array[i]);
        }
        for (int i = 0; i < editor->windows.count; ++i) {
            SDL_FreeSurface(editor->windows.array[i]->surface);
            free(editor->windows.array[i]);
        }
        for (int i = 0; i < editor->layouts.count; ++i) {
            free(editor->layouts.array[i]);
        }
    };

    editor->root_layout = {};
    SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    editor->root_layout.rect = rect;
    Window *layout_window = make_window(editor, SCREEN_WIDTH, SCREEN_HEIGHT);
    editor->root_layout.window = layout_window;
    editor->root_layout.window->parent_layout = &editor->root_layout;


    Buffer *buffer = open_file(editor, input_file_path);
    editor->root_layout.window->buffer = buffer;
    
    Buffer *buffer2 = open_file(editor, "../tests/test.jai");

    editor->current_window = editor->windows.array[0];
    editor->current_gap_buffer = &editor->current_window->buffer->gap_buffer;

#if 1
    split_current_window_horizontally(editor);
    // @todo refactor to change buffer in current window change_buffer(window, buffer2);
    editor->windows.array[1]->buffer = buffer2; // left_window->buffer;
#endif
#if 1
    split_current_window_vertically(editor);
#endif
#if 1
    // @todo functions to change window
    split_window_horizontally(editor, editor->windows.array[1]);
#endif
#if 0
    // @todo delete current window
    // then change current window
#endif

    
    editor->current_cursor = 0; // offset from start of buffer

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
                        SDL_FreeSurface(editor->screen_surface);
                        editor->screen_surface = SDL_GetWindowSurface(editor->window);
                        resize_screen(&editor->root_layout, event.window.data1, event.window.data2);
                        SDL_UpdateWindowSurface(editor->window);
                    } break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        SDL_FreeSurface(editor->screen_surface);
                        editor->screen_surface = SDL_GetWindowSurface(editor->window);
                        resize_screen(&editor->root_layout, event.window.data1, event.window.data2);
                        SDL_UpdateWindowSurface(editor->window);
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
                        editor->current_gap_buffer->set_point(editor->current_cursor);
                        editor->current_gap_buffer->put_char('\n');
                        editor->current_cursor = editor->current_gap_buffer->point_offset();
                    } break;
                    case SDLK_BACKSPACE: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (editor->current_cursor == 0)   break;
                        editor->current_gap_buffer->set_point(editor->current_cursor - 1);
                        editor->current_gap_buffer->delete_chars(1);
                        editor->current_cursor = editor->current_gap_buffer->point_offset();
                        /*
                        if (editor->current_cursor.line == 0 && editor->current_cursor.offset == 0) break;
                        if (editor->current_cursor.offset > 0) {
                            ch_[editor->current_cursor.line].erase(ch_[editor->current_cursor.line].begin() + --editor->current_cursor.offset);
                            break;
                        }
                        if (editor->current_cursor.offset == 0 && editor->current_cursor.line > 0) {
                            if (ch_[editor->current_cursor.line].size() == 0) ch_.erase(ch_.begin() + editor->current_cursor.line);
                            editor->current_cursor.line--;
                            editor->current_cursor.offset = ch_[editor->current_cursor.line].size();
                            // ch_[editor->current_cursor.line].erase(ch_[editor->current_cursor.line].begin() + --editor->current_cursor.offset);
                        }
                        */
                    } break;
                    case SDLK_UP: {
                        if (event.type != SDL_KEYDOWN) break;
                        u64 line_start = 0;
                        u64 offset = 0;
                        b32 found_offset = false;
                        for (u64 i = editor->current_cursor; i-- > 0;) {
                            editor->current_gap_buffer->set_point(i);
                            if (editor->current_gap_buffer->get_char() == '\n') {
                                if (!found_offset) {
                                    line_start = i;
                                    offset = editor->current_cursor - i;
                                    found_offset = true;
                                    continue;
                                } else {
                                    editor->current_cursor = i + offset;
                                    if (editor->current_cursor >= line_start) editor->current_cursor = line_start;
                                    break;
                                }
                            }
                            if (found_offset && i == 0) {
                                editor->current_cursor = offset - 1;
                                if (editor->current_cursor >= line_start) editor->current_cursor = line_start;
                            }
                            editor->current_gap_buffer->set_point(editor->current_cursor);
                        }
                    } break;
                    case SDLK_DOWN: {
                        if (event.type != SDL_KEYDOWN) break;
                        u64 offset = 0;
                        b32 found_offset = false;
                        for (u64 i = editor->current_cursor; i-- > 0;) {
                            editor->current_gap_buffer->set_point(i);
                            if (editor->current_gap_buffer->get_char() == '\n' || i == 0) {
                                offset = editor->current_cursor - i - 1;
                                found_offset = true;
                                break;
                            }
                        }
                        if (!found_offset && editor->current_cursor != 0) break;
                        u64 next_line = 0;
                        for (u64 i = editor->current_cursor; i < editor->current_gap_buffer->sizeof_buffer(); i++) {
                            editor->current_gap_buffer->set_point(i);
                            if (editor->current_gap_buffer->get_char() == '\n') {
                                next_line = i + 1;
                                break;
                            }
                        }
                        editor->current_cursor = next_line;
                        for (u64 i = next_line; i < (next_line + offset); i++) {
                            editor->current_cursor = i + 1;
                            editor->current_gap_buffer->set_point(i);
                            if (editor->current_gap_buffer->get_char() == '\n') {
                                editor->current_cursor--;
                                break;
                            }
                        }
                        if (editor->current_cursor >= editor->current_gap_buffer->sizeof_buffer())   editor->current_cursor = editor->current_gap_buffer->sizeof_buffer() - 1;
                        editor->current_gap_buffer->set_point(editor->current_cursor);
                    } break;
                    case SDLK_LEFT: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (editor->current_cursor > 0) {
                            editor->current_gap_buffer->set_point(editor->current_cursor);
                            editor->current_gap_buffer->previous_char();
                            editor->current_cursor = editor->current_gap_buffer->point_offset();
                            if (editor->current_gap_buffer->get_char() == '\n' && editor->current_cursor > 0)   editor->current_gap_buffer->previous_char();
                            editor->current_cursor = editor->current_gap_buffer->point_offset();
                        }
                    } break;
                    case SDLK_RIGHT: {
                        if (event.type != SDL_KEYDOWN) break;
                        if (editor->current_cursor < editor->current_gap_buffer->sizeof_buffer() - 1) {
                            editor->current_gap_buffer->set_point(editor->current_cursor);
                            editor->current_gap_buffer->next_char();
                            if (editor->current_gap_buffer->get_char() == '\n')   editor->current_gap_buffer->next_char();
                            editor->current_cursor = editor->current_gap_buffer->point_offset();
                            if (editor->current_cursor >= editor->current_gap_buffer->sizeof_buffer())   editor->current_cursor -= 2;
                            editor->current_gap_buffer->set_point(editor->current_cursor);
                        }
                    } break;
                }
                continue;
            }
            if (event.type == SDL_TEXTINPUT) {
                editor->current_gap_buffer->set_point(editor->current_cursor);
                editor->current_gap_buffer->put_char(event.text.text[0]);
                editor->current_cursor = editor->current_gap_buffer->point_offset();
                continue;
            }
        }

        
        // Update
        editor->current_window->cursor = editor->current_cursor;

        // Render
        render_layout(editor, &editor->root_layout);
        SDL_UpdateWindowSurface(editor->window);


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
    editor->current_gap_buffer->print_buffer();
#endif
#if 1
    FILE *out;
    out = fopen("../tests/test_out.jai", "wb");
    defer { fclose(out); };
    editor->current_gap_buffer->set_point(0);
    editor->current_gap_buffer->save_buffer_to_file(out);
#endif

    return 0;
}
