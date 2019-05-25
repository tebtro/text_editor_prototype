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

    int left_w = layout->rect.w / 2;
    Layout *left_layout = make_layout(editor, layout->rect.x, layout->rect.y, left_w, layout->rect.h, left_window);

    int right_w = left_w;
    if (layout->rect.w != left_w * 2)   right_w = layout->rect.w - left_w;
    Window *right_window = make_window(editor, right_w, layout->rect.h);
    right_window->buffer = left_window->buffer;
    Layout *right_layout = make_layout(editor, layout->rect.x + left_w, layout->rect.y,
                                       right_w, layout->rect.h, right_window);

    layout->sub_layout1  = left_layout;
    layout->sub_layout2 = right_layout;
}

// @copynpaste from split_window_horizontally
void split_window_vertically(Editor *editor, Window *window) {
    Layout *layout = window->parent_layout;

    layout->orientation = Layout_Orientation::Vertical;
    Window *top_window = layout->window;
    layout->window = nullptr;

    int top_height = layout->rect.h / 2;
    Layout *top_layout = make_layout(editor, layout->rect.x, layout->rect.y, layout->rect.w, top_height, top_window);

    int bottom_height = top_height;
    if (layout->rect.h != top_height * 2)   bottom_height = layout->rect.h - top_height;
    Window *bottom_window = make_window(editor, layout->rect.w, bottom_height);
    bottom_window->buffer = top_window->buffer;
    Layout *bottom_layout = make_layout(editor, layout->rect.x, layout->rect.y + top_height,
                                        layout->rect.w, bottom_height, bottom_window);

    layout->sub_layout1 = top_layout;
    layout->sub_layout2 = bottom_layout;
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
