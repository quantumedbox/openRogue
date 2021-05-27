#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "window.h"
#include "../../map.h"


int _is_window_subsystem_initialized = false;


// TODO Receiving errors on Python side

// TODO Window map to store active windows in C side too if needed


// ----------------------------------------------------------------- Global objects -- //


// Stores window handlers by their id
Map* window_pool = NULL;


// -------------------------------------------------------------------- Realization -- //


static int _init_window_subsystem()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_AddEventWatch(event_queue_former, NULL);

	SDL_GL_LoadLibrary(NULL);

	window_pool = mapNew();

	_is_window_subsystem_initialized = true;

	return 0;
}


window_id_t init_window(int width, int height, const char* title)
{
	if (!_is_window_subsystem_initialized) {
		if (_init_window_subsystem() == -1)
			return 0;
	}

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, true);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VER);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VER);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if (title == 0) {
		title = DEFAULT_WINDOW_NAME;
	}

	SDL_Window* win = SDL_CreateWindow(
	                      title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                      width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	                  );

	if (!win) {
		printf("Could not create OpenGL window: %s\n", SDL_GetError());
		return 0;
	}

	SDL_GLContext context = SDL_GL_CreateContext(win);

	GLenum error;
	if ((error = glewInit()) != GLEW_OK) {
		printf("Error on GLEW initialization: %s\n", glewGetErrorString(error));
		return 0;
	}

	SDL_GL_SetSwapInterval(1);

	// glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUE_SRC_ALPHA);
	// glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	WindowHandler* win_h = (WindowHandler*)calloc(1, sizeof(WindowHandler));
	win_h->window = win;
	win_h->context = context;
	win_h->id = SDL_GetWindowID(win);

	win_h->queue.events = (Event*)malloc(DISPATCH_BUFFER_SIZE * sizeof(Event));
	win_h->queue.len = 0;

	mapAdd(window_pool, win_h->id, (void*)win_h);

	return win_h->id;
}


// Used for temporally store polled events that do not belong to current processed window
SDL_Event 	event_buffer[EVENT_BUFFER_SIZE];
size_t 		event_buffer_len;

/*__forceinline*/ void _push_buffered_events()
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


void close_window(window_id_t w_id)
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_id);
	mapDel(window_pool, w_id);

	_clear_winodow_events(w->id);
	SDL_GL_DeleteContext(w->context);
	SDL_DestroyWindow(w->window);
	free(w);
}


void resize_window(window_id_t w_id, int width, int height)
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_id);

	SDL_SetWindowSize(w->window, width, height);
}


void repos_window(window_id_t w_id, int x, int y)
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_id);

	SDL_SetWindowPosition(w->window, x, y);
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

void _dispatch_window_close(EventQueue* queue, SDL_Event event)
{
	Event current;

	current.type = CLOSE_EVENT;
	current.timestamp = event.common.timestamp;

	queue->events[queue->len] = current;
	queue->len++;
}


void _dispatch_window_resize(EventQueue* queue, SDL_Event event)
{
	Event current;

	current.type = RESIZE_EVENT;
	current.timestamp = event.common.timestamp;

	current.resize_event.width = event.window.data1;
	current.resize_event.height = event.window.data2;

	queue->events[queue->len] = current;
	queue->len++;
}


void _dispatch_window_repos(EventQueue* queue, SDL_Event event)
{
	Event current;

	current.type = REPOS_EVENT;
	current.timestamp = event.common.timestamp;

	current.repos_event.x = event.window.data1;
	current.repos_event.y = event.window.data2;

	queue->events[queue->len] = current;
	queue->len++;
}


EventQueue process_window(window_id_t w_id)
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_id);

	// Crude way of constant checking
	// const char* err_str = SDL_GetError();
	// if (err_str != NULL) {
	// 	printf("error: %s\n", err_str);
	// }

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

	EventQueue queue = w->queue;

	w->queue.events = (Event*)malloc(DISPATCH_BUFFER_SIZE * sizeof(Event));
	w->queue.len = 0;

	SDL_Event _;
	while (SDL_PollEvent(&_)) {}

	return queue;
}

/*
	Forms event queues to send
*/
int event_queue_former(void* _, SDL_Event* event_ptr)
{
	SDL_Event event = *event_ptr;

	// Helper for getting window ids from event union fields
	#define queue_from_type(_type) &(((WindowHandler*)mapGet(window_pool, event._type.windowID))->queue)

	if (event.type == SDL_MOUSEMOTION) {
		_dispatch_mouse_motion(queue_from_type(motion), event);
	}

	else if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP)) {
		_dispatch_keypress(queue_from_type(window), event);
	}

	else if (event.type == SDL_MOUSEBUTTONUP) {
		_dispatch_mouse_button(queue_from_type(button), event);
	}

	else if (event.type == SDL_WINDOWEVENT) {
		switch (event.window.event) {
		case SDL_WINDOWEVENT_CLOSE:
			_dispatch_window_close(queue_from_type(window), event);
			break;
		case SDL_WINDOWEVENT_RESIZED:
			_dispatch_window_resize(queue_from_type(window), event);
			break;
		case SDL_WINDOWEVENT_MOVED:
			_dispatch_window_repos(queue_from_type(window), event);
			break;
		default:
			break;
		}
	}

	return 0;
}


void _free_event_queue(EventQueue queue)
{
	free(queue.events);
	// free(queue);
}
