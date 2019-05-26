/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include "window.h"

#include "renderer.h"
#include "iml_general.h"


void render_glyph(SDL_Surface *window_surface, Theme *theme,
                  char ch, int x, int y, b32 render_cursor) {
    int glyph_width  = theme->glyph_width;
    int glyph_height = theme->glyph_height;
    int width, height;
    SDL_Color fg = theme->fg;
    SDL_Color bg = theme->bg;
    if (render_cursor) {
        fg = theme->bg;
        bg = theme->fg;
    }
    auto surface = TTF_RenderGlyph_Shaded(theme->font, ch, fg, bg);
    defer {
        SDL_FreeSurface(surface);
    };
    if (!surface) {
        SDL_Rect rect = {(int)x * glyph_width, (int)y * glyph_height, glyph_width, glyph_height};
        SDL_BlitSurface(NULL, NULL, window_surface, &rect);
    }
    width = surface->w;
    height = surface->h;
    SDL_Rect rect = {(int)x * glyph_width, (int)y * glyph_height, width, height};
    SDL_Rect rect_font_surface = {0,0,width,height};
    SDL_BlitSurface(surface, &rect_font_surface , window_surface, &rect);
}

void render_window(Editor *editor, Layout *layout) {
    SDL_Surface *screen_surface = editor->screen_surface;
    Theme *theme = editor->theme;

    Gap_Buffer *gap_buffer = &layout->window->buffer->gap_buffer;
    if (!gap_buffer)   return;
    gap_buffer->set_point(layout->window->cursor);

    SDL_FillRect(layout->window->surface, NULL, 0x000000);
    u64 line = 0;
    u64 cursor_line = 0;
    u64 offset = 0;
    char *temp = gap_buffer->buffer;

    // eat scroll offset lines
    if (layout->window->scroll_line_offset != 0) {
        int newline_count = 0;
        while (temp < gap_buffer->buffer_end) {
            if ((temp >= gap_buffer->gap_start) && (temp < gap_buffer->gap_end)) {
                temp++;
                continue;
            }
            char c = *(temp++);
            if (c == '\n') {
                newline_count++;
            }
            if (temp - 1 == gap_buffer->point) { // :cursor_scroll
                layout->window->scroll_line_offset--;
                editor->active_scroll_line_offset--;
            }
            if (newline_count == layout->window->scroll_line_offset)   break;
        }
    }
    for (int i = 0; temp < gap_buffer->buffer_end; i++) {
        if ((temp >= gap_buffer->gap_start) && (temp < gap_buffer->gap_end)) {
            temp++;
            continue;
        }
        char c = *(temp++);
        if (c == '\n') { // @todo handle soft line breaks (\r), ...
            if (temp - 1 == gap_buffer->point) {
                render_glyph(layout->window->surface, theme, ' ', offset, line, true);
                cursor_line = line;
            }
            line++;
            offset = 0;
            continue;
        }
        if (temp - 1 == gap_buffer->point) {
            render_glyph(layout->window->surface, theme, c, offset, line, true);
            cursor_line = line;
        } else {
            render_glyph(layout->window->surface, theme, c, offset, line);
        }
        offset++;
    }
    if ((cursor_line * theme->glyph_height) > layout->window->rect.h) { // :cursor_scroll // @todo not consisten set widnow x y
        layout->window->scroll_line_offset++;
        editor->active_scroll_line_offset++;
    }
    SDL_Rect rect = {0,0,layout->window->rect.w,layout->window->rect.h};
    SDL_BlitSurface(layout->window->surface, &rect,
                    screen_surface, &layout->rect);
}

void render_layout(Editor *editor, Layout *layout) {
    if (layout->window) {
        render_window(editor, layout);
        return;
    }

    render_layout(editor, layout->sub_layout1);
    render_layout(editor, layout->sub_layout2);
}
