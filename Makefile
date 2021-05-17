CC=gcc
CFLAGS=-x c -std=c11 -Wall

default: debug

main:
	$(CC) $(CFLAGS) src/loader/loader.c -o run.exe $(DEBUG)

debug: DEBUG=-g
release: DEBUG=

debug: main
release: main
