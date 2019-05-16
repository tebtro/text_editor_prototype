/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Trebor Rentro $
   $Notice: (C) Copyright 2018 by Rentro, Inc. All Rights Reserved. $
   ======================================================================== */

#include <iostream>

#include <SDL.h>

#include "iml_types.h"
#include "iml_general.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

int main(int argc, char *argv[]) {
    SDL_Window *window = NULL;
    SDL_Surface *screen = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Konnte SDL nicht initialisieren! Fehler: " << SDL_GetError() << std::endl;
        return -1;
    }
    window = SDL_CreateWindow("Text Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Konnte SDL-Window nicht initialisieren! Fehler: " << SDL_GetError() << std::endl;
        return -1;
    }

    screen = SDL_GetWindowSurface(window);
    if (!screen) {
        std::cerr << "Konnte SDL-Fenster nicht erzeugen! Fehler: " << SDL_GetError() << std::endl;
        return -1;
    }
    defer { 
        SDL_DestroyWindow(window);
        SDL_Quit();
    };

    u32 r = 0xFF;
    u32 g = 0xFF;
    u32 b = 0xFF;
    
    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        r = 0xAA;
                        break;
                    case SDLK_DOWN:
                        g = 0xAA;
                        break;
                    case SDLK_LEFT:
                        r = 0x0;
                        break;
                    case SDLK_RIGHT:
                        b = 0xAA;
                        break;
                }
            }

        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, r, g, b));
        SDL_UpdateWindowSurface(window);
    }

    return 0;
}
