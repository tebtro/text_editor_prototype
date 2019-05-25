/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include <assert.h>
#include "window.h"

#include "layout.h"
#include "editor.h"


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

void make_base_layout(Editor *editor) {
    SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    editor->root_layout.rect = rect;
    Window *layout_window = make_window(editor, SCREEN_WIDTH, SCREEN_HEIGHT);
    editor->root_layout.window = layout_window;
    editor->root_layout.window->parent_layout = &editor->root_layout;

    editor->current_window = editor->windows.array[0];
    editor->current_cursor = 0; // offset from start of buffer
}
