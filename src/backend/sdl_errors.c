#include <stdio.h>

void check_sdl_err() {
	const char *error = SDL_GetError();
	if (*error != '\0') {
		fprintf(stderr, "SDL error: %s\n", error);
		SDL_ClearError();
	}
}
