#if !defined(RENDERER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#define RENDERER_H

#include "iml_types.h"

#include "SDL.h"
#include "theme.h"
#include "editor.h"

void render_glyph(SDL_Surface *window_surface, Theme *theme,
                  char ch, int x, int y, b32 render_cursor = false);

void render_layout(Editor *editor, Layout *layout);

#endif
