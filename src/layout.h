#if !defined(LAYOUT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#define LAYOUT_H

#include <SDL.h>

#include "iml_types.h"

struct Window;
struct Editor;

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

Layout *make_layout(Editor *editor, int x, int y, int width, int height, Window *window);
void split_window(Editor *editor, Window *window, enum32(Layout_Orientation) orientation);

void resize_layout(Layout *layout, SDL_Rect rect);
void resize_screen(Layout *layout, int new_width, int new_height);

void make_base_layout(Editor *editor);

#endif
