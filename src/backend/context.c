#include <stdio.h>

PDCEX SDL_Window* pdc_window;
PDCEX SDL_Surface* pdc_screen;
PDCEX TTF_Font* pdc_ttffont;
PDCEX int pdc_font_size;


STATUS initSDL() {
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		PUSH_ERRORM("Cannot initialize SDL");
		return ERR_STATUS;
	}
	return OK_STATUS;
}


SDL_Window* test(const char* t) {
	pdc_window = SDL_CreateWindow("PDCurses", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	pdc_screen = SDL_GetWindowSurface(pdc_window);

	initscr();
	start_color();
	scrollok(stdscr, true);

	PDC_set_title("PDCurses");

	init_pair(1, COLOR_WHITE + 8, COLOR_BLUE);
    bkgd(COLOR_PAIR(1));

    addstr("This is a demo of ");

    return pdc_window;
}


void makeCurrent(SDL_Window* w) {
	pdc_window = w;
	pdc_screen = SDL_GetWindowSurface(w);
	PDC_update_rects();
}


void writeStr(SDL_Window* w, const char* s) {
	makeCurrent(w);
	addstr(s);
}


void setFont(const char* font_path, int size) {
	pdc_ttffont = TTF_OpenFont(font_path, size);
	pdc_font_size = size;
}


STATUS processPDWindow(SDL_Window* w) {
	makeCurrent(w);

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		SDL_PumpEvents();
	}
	refresh();
	SDL_UpdateWindowSurface(pdc_window);
	check_sdl_err();

	return OK_STATUS;
}
