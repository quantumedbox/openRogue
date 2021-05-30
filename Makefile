CC=gcc
CFLAGS=-x c -std=c11 -Wall -Wextra #-Werror

BACKEND_INCLUDES=-I src/backend
BACKEND_DEFINES=

# TODO for now this is stupid but works i guess

default: debug

run:
	$(CC) $(CFLAGS) src/loader/loader.c -o run.exe

backend:
	$(CC) $(CFLAGS) src/backend/backend.h -shared -o backends/openRogue_SDL.dll $(BACKEND_INCLUDES) -lopengl32 -lglew32 -lSDL2 -fopenmp -lfreetype

debug: CFLAGS += -g -DDEBUG=1
release: CFLAGS += -O3

debug: run backend
release: run backend
