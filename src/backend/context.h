#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define PDC_WIDE
#define PDC_UTF8
#define PDC_DLL_BUILD
#include <curses.h>
#include <pdcsdl.h>

#include "sdl_errors.c"

#define DEFALUT_FONT "C:/Windows/Fonts/consola.ttf"
#define DEFALUT_FONT_SIZE 20
#include "context.c"
