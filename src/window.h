#if !defined(WINDOW_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#define WINDOW_H

#include <SDL.h>

#include "gap_buffer.h"
#include "editor.h"

struct Editor;
struct Layout;

struct Buffer {
    Gap_Buffer gap_buffer;
    // file info
    // ...
};

struct Window {
    Layout *parent_layout;
    Buffer *buffer;
    // cursor should maybe be on buffer
    u64 cursor = 0; // offset from start of buffer
    int scroll_line_offset = 0;


    SDL_Rect rect;
    SDL_Surface *surface;
};


void resize_window(Window *window, int width, int height);
Window *make_window(Editor *editor, int width, int height);
void change_active_window(Editor *editor, Window *window);

#endif
