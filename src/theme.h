#if !defined(THEME_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#define THEME_H

#include <SDL_ttf.h>

struct Theme {
    TTF_Font *font;
    int glyph_width;
    int glyph_height;
    SDL_Color fg;
    SDL_Color bg;
    SDL_Color cursor_fg;
    SDL_Color cursor_bg;
};

Theme *load_theme();

#endif
