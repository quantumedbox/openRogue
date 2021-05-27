CC=gcc
CFLAGS=-x c -std=c11 -Wall -Werror

# TODO for now this is stupid but works i guess

default: debug

run:
	$(CC) $(CFLAGS) src/loader/loader.c -o run.exe $(DEBUG)

backend:
	$(CC) $(CFLAGS) src/backend/backend.h -shared -fPIC -o backends/openRogue_SDL.dll $(DEBUG) -lopengl32 -lglew32 -l:SDL2.dll -fopenmp

debug: DEBUG=-g
release: DEBUG=

debug: run backend
release: run backend
