/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include <assert.h>
#include <iostream>

#include "editor.h"
#include "iml_general.h"

#include "window.h"
#include "key_binding.h"

Editor *make_editor() {
    // new because of std::map not getting initialized or something.   ugh ... 
    Editor *editor = new Editor(); // (Editor *) calloc(1, sizeof(Editor));

    // Bind_Helper init
    editor->bind_context = new Bind_Helper(); // (Bind_Helper *) calloc(1, sizeof(Bind_Helper));

    // SDL init
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    editor->window = SDL_CreateWindow("vis", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
    assert(editor->window);
    editor->screen_surface = SDL_GetWindowSurface(editor->window);
    assert(editor->screen_surface);

    editor->theme = load_theme();

    editor->buffers = {};
    editor->windows = {};
    editor->layouts = {};

    make_base_layout(editor);

    return editor;
}

void free_editor(Editor *editor) {
    free(editor->bind_context);

    // free arrays
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

    // free SDL
    TTF_CloseFont(editor->theme->font);
    
    SDL_FreeSurface(editor->screen_surface);
    SDL_DestroyWindow(editor->window);
    SDL_Quit();
    TTF_Quit();

    // free editor, also the last thing
    defer { free(editor); };
}

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
