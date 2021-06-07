#pragma once

#include <SDL2/SDL.h>

#include "threads.h"

#define OPENGL_MINOR_VER 3
#define OPENGL_MAJOR_VER 4

#define DEFAULT_WINDOW_NAME "openRogue"

#define WINDOW_FILL_COLOR_R 0.2
#define WINDOW_FILL_COLOR_G 0
#define WINDOW_FILL_COLOR_B 0.062

// Size of staticly allocated event queue buffer
#define EVENT_BUFFER_SIZE 16


const char** get_feature_list();

/*
*/
typedef struct {
	SDL_Window* window;
	// SDL_GLContext context; // Now GL context is global for all windows of single thread

	// SDL Window ID
	uint32_t id;

	// Switching queues
	// At given time only one of them should be writable and another - readable
	int current_queue;
	EventQueue* queue0;
	EventQueue* queue1;

	// Updated on start_drawing()
	int width;
	int height;

	uint32_t time_delta;
	uint32_t prev_timestamp;

	// Used for preventing sigegiv from SDL EventWatch thread
	mutex_t lock;
}
WindowHandler;

// TODO Descriptions

/*
*/
key_t init_window(int width, int height, const char* title);

/*
*/
void update_window();

/*
*/
void close_window(key_t);

/*
*/
EventQueue* process_window(key_t);

/*
*/
void resize_window(key_t, int width, int height);

/*
*/
void repos_window(key_t, int x, int y);

/*
*/
void start_drawing(key_t);

/*
*/
void finish_drawing();

/*
	Python code should call to free memory when it's done
*/
// void free_event_queue(EventQueue*);
