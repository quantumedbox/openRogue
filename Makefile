CC=gcc
CFLAGS=-x c -std=c11 -Wall -Werror


default: debug

run:
	$(CC) $(CFLAGS) src/loader/loader.c -o run.exe $(DEBUG)

backend:
	$(CC) $(CFLAGS) src/backend/backend.h -shared -o openRogue.dll $(DEBUG) -lopengl32 -lglew32 -l:SDL2.dll

debug: DEBUG=-g
release: DEBUG=

debug: run backend
release: run backend
