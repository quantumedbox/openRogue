#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "window.h"


int _SDL_INITIALIZED = 0;


// TODO Receiving errors on Python side


WindowHandler* init_window(int width, int height, const char* title)
{
	if (!_SDL_INITIALIZED) {
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_GL_LoadLibrary(NULL);
		_SDL_INITIALIZED = 1;
	}

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, true);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VER);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VER);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if (title == NULL) {
		title = DEFAULT_WINDOW_NAME;
	}

	SDL_Window* win = SDL_CreateWindow(
		title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (!win) {
		printf("Could not create OpenGL window: %s\n", SDL_GetError());
		return NULL;
	}

	SDL_GLContext context = SDL_GL_CreateContext(win);
	
	GLenum error;
	if ((error = glewInit()) != GLEW_OK) {
		printf("Error on GLEW initialization: %s\n", glewGetErrorString(error));
		return NULL;
	}

	SDL_GL_SetSwapInterval(1);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	WindowHandler* win_h = (WindowHandler*)calloc(1, sizeof(WindowHandler));
	win_h->window = win;
	win_h->context = context;
	win_h->id = SDL_GetWindowID(win);

	return win_h;
}


// Used for temporally store polled events that do not belong to current processed window
SDL_Event 	event_buffer[EVENT_BUFFER_SIZE];
size_t 		event_buffer_len;

__forceinline void _push_buffered_events()
{
	uint32_t cur_tick = SDL_GetTicks();
	// Does order of addition matter? I believe it might
	for (register int i = 0; i < event_buffer_len; i++ ) {
		if (event_buffer[i].common.timestamp - cur_tick >= EVENT_TIMEOUT)
			continue;
		SDL_PushEvent(&event_buffer[i]);
	}
	event_buffer_len = 0;
}


// If events are not processed for a long time they will be erased from buffer to prevent overflowing
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


void close_window(WindowHandler* w)
{
	_clear_winodow_events(w->id);
	SDL_GL_DeleteContext(w->context);
	SDL_DestroyWindow(w->window);
	free(w);
}


__forceinline void _dispatch_keypress(EventQueue* queue, SDL_Event event)
{
	Event current;

	current.type = INPUT_EVENT;
	current.timestamp = event.common.timestamp;

	current.input_event.is_key_pressed = event.key.state == SDL_PRESSED ? true : false;

	current.input_event.is_key_repeat = event.key.repeat;
	current.input_event.keycode = event.key.keysym.sym;

	switch (event.key.keysym.mod) {
	case KMOD_LSHIFT:
		current.input_event.keymod = KEYMOD_LEFT_SHIFT;
		break;
	case KMOD_RSHIFT:
		current.input_event.keymod = KEYMOD_RIGHT_SHIFT;
		break;
	case KMOD_LCTRL:
		current.input_event.keymod = KEYMOD_LEFT_CTRL;
		break;
	case KMOD_RCTRL:
		current.input_event.keymod = KEYMOD_RIGHT_CTRL;
		break;
	case KMOD_LALT:
		current.input_event.keymod = KEYMOD_LEFT_ALT;
		break;
	case KMOD_RALT:
		current.input_event.keymod = KEYMOD_RIGHT_ALT;
		break;
	default:
		current.input_event.keymod = KEYMOD_NONE;
	}

	queue->events[queue->len] = current;
	queue->len++;
}


void _dispatch_mouse_motion(EventQueue* queue, SDL_Event event)
{
	Event current;

	current.type = POINTER_EVENT;
	current.timestamp = event.common.timestamp;

	current.pointer_event.mouse_action 	= MOUSE_MOTION;
	current.pointer_event.is_pressed 	= 0;
	current.pointer_event.x 			= event.motion.x;
	current.pointer_event.y 			= event.motion.y;
	current.pointer_event.x_motion 		= event.motion.xrel;
	current.pointer_event.y_motion 		= event.motion.yrel;

	queue->events[queue->len] = current;
	queue->len++;
}


void _dispatch_mouse_button(EventQueue* queue, SDL_Event event)
{
	Event current;

	current.type = POINTER_EVENT;
	current.timestamp = event.common.timestamp;

	switch (event.button.button) {
	case SDL_BUTTON_LEFT:
		current.pointer_event.mouse_action = MOUSE_BUTTON_LEFT;
		break;
	case SDL_BUTTON_RIGHT:
		current.pointer_event.mouse_action = MOUSE_BUTTON_RIGHT;
		break;
	case SDL_BUTTON_MIDDLE:
		current.pointer_event.mouse_action = MOUSE_BUTTON_MIDDLE;
		break;
	}

	current.pointer_event.is_pressed 	= event.button.state == SDL_PRESSED ? true : false;
	current.pointer_event.x 			= event.button.x;
	current.pointer_event.y 			= event.button.y;
	current.pointer_event.x_motion 		= 0;
	current.pointer_event.y_motion 		= 0;

	queue->events[queue->len] = current;
	queue->len++;
}


EventQueue* process_window(WindowHandler* w)
{
	// Temp
	SDL_GL_MakeCurrent(w->window, w->context);

	glClearColor(
		WINDOW_FILL_COLOR_R,
		WINDOW_FILL_COLOR_G,
		WINDOW_FILL_COLOR_B,
		1.0
	);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	SDL_GL_SwapWindow(w->window);

	EventQueue* queue = (EventQueue*)malloc(sizeof(EventQueue));
	queue->events = (Event*)malloc(DISPATCH_BUFFER_SIZE * sizeof(Event));
	queue->len = 0;
	queue->window_signals = WINDOW_SIGNAL_CLEAR;

	#define check_window_id_of_type(_type) 			\
													\
			if (event._type.windowID != w->id)		\
					{								\
					_add_buffered_event(event);		\
					continue; 						\
				}


	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP)) {
			check_window_id_of_type(window);
			_dispatch_keypress(queue, event);
		}

		else if (event.type == SDL_MOUSEMOTION) {
			check_window_id_of_type(motion);
			_dispatch_mouse_motion(queue, event);
		}

		else if (event.type == SDL_MOUSEBUTTONUP) {
			check_window_id_of_type(button);
			_dispatch_mouse_button(queue, event);
		}

		else if (event.type == SDL_WINDOWEVENT) {
			check_window_id_of_type(window);
			switch (event.window.type) {
			case SDL_WINDOWEVENT_CLOSE:
				MASK_SET_BIT(queue->window_signals, WINDOW_SIGNAL_CLOSED);
				break;
			default:
				break;
			}
		}

		// Halt dispatch if buffer is full
		if (queue->len >= DISPATCH_BUFFER_SIZE) {
			break;
		}
	}
	_push_buffered_events();

	return queue;
}


void _free_event_queue(EventQueue* queue)
{
	free(queue->events);
	free(queue);
}
