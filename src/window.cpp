/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include "window.h"

void resize_window(Window *window, int width, int height) {
    SDL_Rect rect = {0, 0, width, height};
    window->rect = rect;
    SDL_FreeSurface(window->surface);
    window->surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
}

Window *make_window(Editor *editor, int width, int height) {
    Window *window = (Window *) calloc(1, sizeof(Window));
    editor->windows.push(window);
    resize_window(window, width, height);
    return window;
}
