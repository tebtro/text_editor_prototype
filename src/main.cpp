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

#include "theme.h"
#include "window.h"
#include "layout.h"
#include "renderer.h"

#include "editor.h"

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

typedef void Custom_Command_Function(struct Editor *editor);
#define CUSTOM_COMMAND_SIG(function_name) void function_name(struct Editor *editor)

CUSTOM_COMMAND_SIG(newline) {
    editor->current_gap_buffer->set_point(editor->current_cursor);
    editor->current_gap_buffer->put_char('\n');
    editor->current_cursor = editor->current_gap_buffer->point_offset();
}

CUSTOM_COMMAND_SIG(backspace_char) {
    if (editor->current_cursor == 0)   return;
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
}

CUSTOM_COMMAND_SIG(move_up) {
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
}

CUSTOM_COMMAND_SIG(move_down) {
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
    if (!found_offset && editor->current_cursor != 0)   return;
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
}

CUSTOM_COMMAND_SIG(move_left) {
    if (editor->current_cursor > 0) {
        editor->current_gap_buffer->set_point(editor->current_cursor);
        editor->current_gap_buffer->previous_char();
        editor->current_cursor = editor->current_gap_buffer->point_offset();
        if (editor->current_gap_buffer->get_char() == '\n' && editor->current_cursor > 0)   editor->current_gap_buffer->previous_char();
        editor->current_cursor = editor->current_gap_buffer->point_offset();
    }
}

CUSTOM_COMMAND_SIG(move_right) {
    if (editor->current_cursor < editor->current_gap_buffer->sizeof_buffer() - 1) {
        editor->current_gap_buffer->set_point(editor->current_cursor);
        editor->current_gap_buffer->next_char();
        if (editor->current_gap_buffer->get_char() == '\n')   editor->current_gap_buffer->next_char();
        editor->current_cursor = editor->current_gap_buffer->point_offset();
        if (editor->current_cursor >= editor->current_gap_buffer->sizeof_buffer())   editor->current_cursor -= 2;
        editor->current_gap_buffer->set_point(editor->current_cursor);
    }
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

    
    Editor *editor = make_editor();
    defer { free_editor(editor); };


    if (input_file_path) {
        Buffer *buffer = open_file(editor, input_file_path);
        editor->root_layout.window->buffer = buffer;
        editor->current_gap_buffer = &editor->current_window->buffer->gap_buffer;
    }

#if 1
    Buffer *buffer2 = open_file(editor, "../tests/test.jai");
    split_current_window_horizontally(editor);
    // @todo refactor to change buffer in current window change_buffer(window, buffer2);
    editor->windows.array[1]->buffer = buffer2; // left_window->buffer;

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
#endif
    
    
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
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: {
                        running = !(event.type == SDL_KEYDOWN);
                    } break;
                    case SDLK_RETURN: {
                        newline(editor);
                    } break;
                    case SDLK_BACKSPACE: {
                        backspace_char(editor);
                    } break;
                    case SDLK_UP: {
                        move_up(editor);
                    } break;
                    case SDLK_DOWN: {
                        move_down(editor);
                    } break;
                    case SDLK_LEFT: {
                        move_left(editor);
                    } break;
                    case SDLK_RIGHT: {
                        move_right(editor);
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
