/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include "base_commands.h"

COMMAND_SIG(newline) {
    editor->current_gap_buffer->set_point(editor->current_cursor);
    editor->current_gap_buffer->put_char('\n');
    editor->current_cursor = editor->current_gap_buffer->point_offset();
}

COMMAND_SIG(backspace_char) {
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

COMMAND_SIG(move_up) {
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

COMMAND_SIG(move_down) {
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

COMMAND_SIG(move_left) {
    if (editor->current_cursor > 0) {
        editor->current_gap_buffer->set_point(editor->current_cursor);
        editor->current_gap_buffer->previous_char();
        editor->current_cursor = editor->current_gap_buffer->point_offset();
        if (editor->current_gap_buffer->get_char() == '\n' && editor->current_cursor > 0)   editor->current_gap_buffer->previous_char();
        editor->current_cursor = editor->current_gap_buffer->point_offset();
    }
}

COMMAND_SIG(move_right) {
    if (editor->current_cursor < editor->current_gap_buffer->sizeof_buffer() - 1) {
        editor->current_gap_buffer->set_point(editor->current_cursor);
        editor->current_gap_buffer->next_char();
        if (editor->current_gap_buffer->get_char() == '\n')   editor->current_gap_buffer->next_char();
        editor->current_cursor = editor->current_gap_buffer->point_offset();
        if (editor->current_cursor >= editor->current_gap_buffer->sizeof_buffer())   editor->current_cursor -= 2;
        editor->current_gap_buffer->set_point(editor->current_cursor);
    }
}


COMMAND_SIG(split_current_window_horizontally) {
    split_window_horizontally(editor, editor->current_window);
}

COMMAND_SIG(split_current_window_vertically) {
    split_window_vertically(editor, editor->current_window);
}
