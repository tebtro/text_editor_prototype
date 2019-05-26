/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include <assert.h>
#include "base_commands.h"

COMMAND_SIG(newline) {
    editor->active_gap_buffer->set_point(editor->active_cursor);
    editor->active_gap_buffer->put_char('\n');
    editor->active_cursor = editor->active_gap_buffer->point_offset();
}

COMMAND_SIG(backspace_char) {
    if (editor->active_cursor == 0)   return;
    editor->active_gap_buffer->set_point(editor->active_cursor - 1);
    editor->active_gap_buffer->delete_chars(1);
    editor->active_cursor = editor->active_gap_buffer->point_offset();
    /*
      if (editor->active_cursor.line == 0 && editor->active_cursor.offset == 0) break;
      if (editor->active_cursor.offset > 0) {
      ch_[editor->active_cursor.line].erase(ch_[editor->active_cursor.line].begin() + --editor->active_cursor.offset);
      break;
      }
      if (editor->active_cursor.offset == 0 && editor->active_cursor.line > 0) {
      if (ch_[editor->active_cursor.line].size() == 0) ch_.erase(ch_.begin() + editor->active_cursor.line);
      editor->active_cursor.line--;
      editor->active_cursor.offset = ch_[editor->active_cursor.line].size();
      // ch_[editor->active_cursor.line].erase(ch_[editor->active_cursor.line].begin() + --editor->active_cursor.offset);
      }
    */
}

COMMAND_SIG(move_up) {
    u64 line_start = 0;
    u64 offset = 0;
    b32 found_offset = false;
    for (u64 i = editor->active_cursor; i-- > 0;) {
        editor->active_gap_buffer->set_point(i);
        if (editor->active_gap_buffer->get_char() == '\n') {
            if (!found_offset) {
                line_start = i;
                offset = editor->active_cursor - i;
                found_offset = true;
                continue;
            } else {
                editor->active_cursor = i + offset;
                if (editor->active_cursor >= line_start) editor->active_cursor = line_start;
                break;
            }
        }
        if (found_offset && i == 0) {
            editor->active_cursor = offset - 1;
            if (editor->active_cursor >= line_start) editor->active_cursor = line_start;
        }
        editor->active_gap_buffer->set_point(editor->active_cursor);
    }
}

COMMAND_SIG(move_down) {
    u64 offset = 0;
    b32 found_offset = false;
    for (u64 i = editor->active_cursor; i-- > 0;) {
        editor->active_gap_buffer->set_point(i);
        if (editor->active_gap_buffer->get_char() == '\n' || i == 0) {
            offset = editor->active_cursor - i - 1;
            found_offset = true;
            break;
        }
    }
    if (!found_offset && editor->active_cursor != 0)   return;
    u64 next_line = 0;
    for (u64 i = editor->active_cursor; i < editor->active_gap_buffer->sizeof_buffer(); i++) {
        editor->active_gap_buffer->set_point(i);
        if (editor->active_gap_buffer->get_char() == '\n') {
            next_line = i + 1;
            break;
        }
    }
    editor->active_cursor = next_line;
    for (u64 i = next_line; i < (next_line + offset); i++) {
        editor->active_cursor = i + 1;
        editor->active_gap_buffer->set_point(i);
        if (editor->active_gap_buffer->get_char() == '\n') {
            editor->active_cursor--;
            break;
        }
    }
    if (editor->active_cursor >= editor->active_gap_buffer->sizeof_buffer())   editor->active_cursor = editor->active_gap_buffer->sizeof_buffer() - 1;
    editor->active_gap_buffer->set_point(editor->active_cursor);
}

COMMAND_SIG(move_left) {
    if (editor->active_cursor > 0) {
        editor->active_gap_buffer->set_point(editor->active_cursor);
        editor->active_gap_buffer->previous_char();
        editor->active_cursor = editor->active_gap_buffer->point_offset();
        if (editor->active_gap_buffer->get_char() == '\n' && editor->active_cursor > 0)   editor->active_gap_buffer->previous_char();
        editor->active_cursor = editor->active_gap_buffer->point_offset();
    }
}

COMMAND_SIG(move_right) {
    if (editor->active_cursor < editor->active_gap_buffer->sizeof_buffer() - 1) {
        editor->active_gap_buffer->set_point(editor->active_cursor);
        editor->active_gap_buffer->next_char();
        if (editor->active_gap_buffer->get_char() == '\n')   editor->active_gap_buffer->next_char();
        editor->active_cursor = editor->active_gap_buffer->point_offset();
        if (editor->active_cursor >= editor->active_gap_buffer->sizeof_buffer())   editor->active_cursor -= 2;
        editor->active_gap_buffer->set_point(editor->active_cursor);
    }
}

#include "layout.h"

COMMAND_SIG(split_active_window_horizontally) {
    split_window(editor, editor->active_window, Layout_Orientation::Horizontal);
}

COMMAND_SIG(split_active_window_vertically) {
    split_window(editor, editor->active_window, Layout_Orientation::Vertical);
}

#include "window.h"

// @todo x and y should also be stored on the window, and not only on the parent layout
COMMAND_SIG(change_active_window_from_mouse_click) {
    int x;
    int y;
    SDL_GetMouseState(&x, &y);

    // printf("mouse click: {x: %d, y: %d}\n", x, y);

    Window *window_under_cursor;
    for (int i = 0; i < editor->windows.count; ++i) {
        Layout *layout = editor->windows.array[i]->parent_layout;
        if (layout->rect.x < x && x < (layout->rect.x + layout->rect.w) &&
            layout->rect.y < y && y < (layout->rect.y + layout->rect.h)) {
            // printf("window_under_cursor: {x: %d, y: %d, w: %d, h: %d}\n", layout->rect.x, layout->rect.y, layout->rect.w, layout->rect.h);
            window_under_cursor = editor->windows.array[i];
        }
    }
    if (!window_under_cursor)   return;

    change_active_window(editor, window_under_cursor);
}

COMMAND_SIG(delete_active_window) {
    if (editor->windows.count == 1)   return;

    Layout *layout = editor->active_window->parent_layout->parent_layout;

    if (layout->sub_layout1->window && layout->sub_layout2->window) {
        if (layout->sub_layout1->window == editor->active_window) {
            layout->window = layout->sub_layout2->window;
        }
        if (layout->sub_layout2->window == editor->active_window) {
            layout->window = layout->sub_layout1->window;
        }
        layout->window->parent_layout = layout;

        {
            int index = editor->windows.get_index_for_element(editor->active_window);
            free(editor->windows.array[index]);
            editor->windows.remove(index);
        }

        {
            int index = editor->layouts.get_index_for_element(layout->sub_layout1);
            free(editor->layouts.array[index]);
            editor->layouts.remove(index);
            layout->sub_layout1 = nullptr;
        }
        {
            int index = editor->layouts.get_index_for_element(layout->sub_layout2);
            free(editor->layouts.array[index]);
            editor->layouts.remove(index);
            layout->sub_layout2 = nullptr;
        }
    } else {
        Layout *sub_layout;
        Window *delete_window;
        Layout *delete_layout;
        if (layout->sub_layout1->window) {
            sub_layout = layout->sub_layout2;
            delete_window = layout->sub_layout1->window;
            delete_layout = layout->sub_layout1;
        }
        if (layout->sub_layout2->window) {
            sub_layout = layout->sub_layout1;
            delete_window = layout->sub_layout2->window;
            delete_layout = layout->sub_layout2;
        }
        layout->orientation = sub_layout->orientation;
        layout->sub_layout1 = sub_layout->sub_layout1;
        layout->sub_layout1->parent_layout = layout;
        layout->sub_layout2 = sub_layout->sub_layout2;
        layout->sub_layout2->parent_layout = layout;


        {
            int index = editor->windows.get_index_for_element(delete_window);
            free(editor->windows.array[index]);
            editor->windows.remove(index);
        }
        {
            int index = editor->layouts.get_index_for_element(delete_layout);
            free(editor->layouts.array[index]);
            editor->layouts.remove(index);
        }
    }

    resize_layout(&editor->root_layout, editor->root_layout.rect);
    change_active_window(editor, layout->window);
}
