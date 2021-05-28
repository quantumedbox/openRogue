CC=gcc
CFLAGS=-x c -std=c11 -Wall -Wextra #-Werror

BACKEND_INCLUDES=-I src/backend

# TODO for now this is stupid but works i guess

default: debug

run:
	$(CC) $(CFLAGS) src/loader/loader.c -o run.exe $(DEBUG)

backend:
	$(CC) $(CFLAGS) src/backend/backend.h -shared -o backends/openRogue_SDL.dll $(DEBUG) $(BACKEND_INCLUDES) -lopengl32 -lglew32 -lSDL2 -fopenmp -lfreetype

debug: DEBUG=-g
release: DEBUG=

debug: run backend
release: run backend
