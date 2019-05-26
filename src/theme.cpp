/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include <assert.h>

#include "theme.h"

Theme *load_theme() { 
    Theme *theme = (Theme *) calloc(1, sizeof(Theme));
    theme->font = TTF_OpenFont("../run_tree/data/fonts/SourceCodePro.ttf", 16);
    assert(theme->font);
    {
        int minx, maxx, miny, maxy, advance;
        TTF_GlyphMetrics(theme->font, L'W', &minx, &maxx, &miny, &maxy, &advance);
        theme->glyph_width = advance;
        theme->glyph_height = TTF_FontLineSkip(theme->font) + 1;
    }
    theme->fg = {189, 184, 164, 0}; // {255, 255, 255, 0};
    theme->bg = {7, 38, 39, 0}; // {0, 0, 0, 0};
    theme->cursor_fg = theme->bg;
    theme->cursor_bg = {143, 238, 142, 0}; // theme->fg;

    return theme;
}
