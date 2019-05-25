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

void split_window(Editor *editor, Window *window, enum32(Layout_Orientation) orientation) {
    Layout *layout = window->parent_layout;

    layout->orientation = orientation;
    Window *first_window = layout->window;
    layout->window = nullptr;

    SDL_Rect first_rect = layout->rect;
    if (orientation == Layout_Orientation::Horizontal)   first_rect.w = first_rect.w / 2;
    if (orientation == Layout_Orientation::Vertical)     first_rect.h = first_rect.h / 2;

    Layout *first_layout = make_layout(editor, first_rect.x, first_rect.y, first_rect.w, first_rect.h, first_window);

    SDL_Rect second_rect = layout->rect;
    if (orientation == Layout_Orientation::Horizontal) {
        second_rect.x += first_rect.w;
        if (layout->rect.w != first_rect.w * 2)   second_rect.w = layout->rect.w - first_rect.w;
    }
    if (orientation == Layout_Orientation::Vertical) {
        second_rect.y += first_rect.h;
        if (layout->rect.h != first_rect.h * 2)   second_rect.h = layout->rect.h - first_rect.h;
    }
    Window *second_window = make_window(editor, second_rect.w, second_rect.h);
    second_window->buffer = first_window->buffer;
    Layout *second_layout = make_layout(editor, second_rect.x, second_rect.y,
                                        second_rect.w, second_rect.h, second_window);

    layout->sub_layout1 = first_layout;
    layout->sub_layout2 = second_layout;
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
