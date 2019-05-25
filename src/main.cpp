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

#include "key_binding.h"

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
    load_default_bindings(editor);


    if (input_file_path) {
        Buffer *buffer = open_file(editor, input_file_path);
        editor->root_layout.window->buffer = buffer;
        editor->active_gap_buffer = &editor->active_window->buffer->gap_buffer;
    }

#if 0
    Buffer *buffer2 = open_file(editor, "../tests/test.jai");
    split_active_window_horizontally(editor);
    // @todo refactor to change buffer in active window change_buffer(window, buffer2);
    editor->windows.array[1]->buffer = buffer2; // left_window->buffer;

#if 1
    split_active_window_vertically(editor);
#endif
#if 1
    // @todo functions to change window
    split_window_horizontally(editor, editor->windows.array[1]);
#endif
#if 0
    // @todo delete active window
    // then change active window
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
                if (event.key.keysym.sym == SDLK_ESCAPE)   running = !(event.type == SDL_KEYDOWN);
                handle_command(editor, event.key.keysym.sym, event.key.keysym.mod);
                continue;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                handle_command(editor, event.button.button);
            }
            if (event.type == SDL_TEXTINPUT) {
                editor->active_gap_buffer->set_point(editor->active_cursor);
                editor->active_gap_buffer->put_char(event.text.text[0]);
                editor->active_cursor = editor->active_gap_buffer->point_offset();
                continue;
            }
        }

        
        // Update
        editor->active_window->cursor = editor->active_cursor;

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
    editor->active_gap_buffer->print_buffer();
#endif
#if 1
    FILE *out;
    out = fopen("../tests/test_out.jai", "wb");
    defer { fclose(out); };
    editor->active_gap_buffer->set_point(0);
    editor->active_gap_buffer->save_buffer_to_file(out);
#endif

    return 0;
}
