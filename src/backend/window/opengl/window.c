#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "window.h"


int _SDL_INITIALIZED = 0;


SDL_Window* init_window(int width, int height, const char* title)
{
	if (!_SDL_INITIALIZED) {
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_GL_LoadLibrary(NULL);
		_SDL_INITIALIZED = 1;
	}

	SDL_Window* window;
	// SDL_GLContext context;

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, true);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VER);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VER);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if (title == NULL) {
		title = DEFAULT_WINDOW_NAME;
	}

	window = SDL_CreateWindow(
		title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (!window) {
		printf("Could not create OpenGL window: %s\n", SDL_GetError());
		return 0;
	}

	/*context = */SDL_GL_CreateContext(window);
	glewInit();

	SDL_GL_SetSwapInterval(1);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// printf("%p\n", window);
	return window;
}


// Used for temporally store polled events that do not belong to current processed window
SDL_Event 	event_buffer[EVENT_BUFFER_SIZE];
size_t 		event_buffer_len;

__forceinline void _push_buffered_events()
{
	for (register int i = event_buffer_len; i--; ) {
		SDL_PushEvent(&event_buffer[i]);
	}
	event_buffer_len = 0;
}


// If events are not processed for a long time they will be erased from buffer
__forceinline void _add_buffered_event(SDL_Event event)
{
	if (event_buffer_len >= EVENT_BUFFER_SIZE)
		return;
	event_buffer[event_buffer_len] = event;
	event_buffer_len++;
}


// When window is no longer valid we should clear all leftover events in event stack
void _clear_winodow_events(int window_id)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type != SDL_WINDOWEVENT || event.window.windowID != window_id)
			_add_buffered_event(event);
	}
	_push_buffered_events();
}

// Switch-case
WINDOW_SIGNAL process_window(SDL_Window* w)
{
	bitmask mask = 0;

	int window_id = SDL_GetWindowID(w);

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		printf("event type: %d\n", event.type);
		if (event.type == SDL_QUIT)
		{
			MASK_SET_BIT(mask, WINDOW_SIGNAL_CLOSED);
			SDL_Quit();
			break;
		}
		else if (event.type == SDL_WINDOWEVENT && event.window.windowID == window_id)
		{
			printf("win: %d, cur: %d, event: %d\n", event.window.windowID, window_id, event.window.event);
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				MASK_SET_BIT(mask, WINDOW_SIGNAL_CLOSED);
				SDL_DestroyWindow(w);
				_clear_winodow_events(window_id);
				break;
			}
		} else {
			_add_buffered_event(event);
		}
	}
	_push_buffered_events();

	return mask;
}
