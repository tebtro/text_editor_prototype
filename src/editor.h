#if !defined(EDITOR_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#define EDITOR_H

#include <SDL.h>

#include "theme.h"
#include "layout.h"

#include "iml_types.h"
#include "iml_array.h"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768


struct Bind_Helper;
struct Buffer;
struct Window;
struct Layout;
struct Gap_Buffer;

struct Editor {
    Bind_Helper *bind_context;

    SDL_Window  *window;
    SDL_Surface *screen_surface;

    Theme *theme;

    Array<Buffer *> buffers;
    Array<Window *> windows;
    Array<Layout *> layouts;

    Layout root_layout;

    Window     *active_window;
    Gap_Buffer *active_gap_buffer;
    u64         active_cursor;
};


Editor *make_editor();
void free_editor(Editor *editor);

Buffer *open_file(Editor *editor, char *input_file_path);

#endif
