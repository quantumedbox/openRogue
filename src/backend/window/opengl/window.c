#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "window.h"
#include "map.h"
#include "text/text.c"
#include "error.h"


// TODO Put event logic in a separate file

// TODO It is possible to negate the need to allocate new event queues by having 2 event queues for every window that could be switched back and forth on demand


// Helper for getting window ids from event union fields
#define queue_from_event_type(event, type) ((WindowHandler*)mapGet(window_pool, event.type.windowID))->queue


// ----------------------------------------------------------------- Global objects -- //


static int is_window_subsystem_initialized = false;


// Stores window handlers by their id
Map* window_pool = NULL;


// -------------------------------------------------------------------- Realization -- //


static
int
init_window_subsystem ()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error on SDL initialization:\n%s\n", SDL_GetError());
		SIGNAL_ERROR();
		return -1;
	}

	SDL_AddEventWatch(event_queue_former, NULL);

	if (SDL_GL_LoadLibrary(NULL) != 0) {
		fprintf(stderr, "Error on SDL OpenGL extenion initialization:\n%s\n", SDL_GetError());
		SIGNAL_ERROR();
		return -1;
	}

	window_pool = mapNew();

	is_window_subsystem_initialized = true;

	return 0;
}

window_id_t
init_window (int width, int height, const char* title)
{
	if (!is_window_subsystem_initialized) {
		if (init_window_subsystem() == -1) {
			return 0;
		}
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
		fprintf(stderr, "Could not create OpenGL window: %s\n", SDL_GetError());
		SIGNAL_ERROR();
		return 0;
	}

	SDL_GLContext context = SDL_GL_CreateContext(win);

	GLenum error;
	if ((error = glewInit()) != GLEW_OK) {
		fprintf(stderr, "Error on GLEW initialization: %s\n", glewGetErrorString(error));
		SIGNAL_ERROR();
		return 0;
	}

	if (!is_text_subsystem_initialized) {
		if (init_text_subsystem() == -1)
			return 0;
	}

	SDL_GL_SetSwapInterval(1);

	glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUE_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	WindowHandler* win_h = (WindowHandler*)calloc(1, sizeof(WindowHandler));
	win_h->window = win;
	win_h->context = context;
	win_h->id = SDL_GetWindowID(win);

	win_h->queue = (EventQueue*)malloc(sizeof(EventQueue));
	win_h->queue->events = (Event*)malloc(EVENT_BUFFER_SIZE * sizeof(Event));
	win_h->queue->len = 0;

	mapAdd(window_pool, win_h->id, (void*)win_h);

	return win_h->id;
}


void
close_window (window_id_t w_id)
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_id);
	mapDel(window_pool, w_id);

	free(w->queue->events);
	free(w->queue);

	SDL_GL_DeleteContext(w->context);
	SDL_DestroyWindow(w->window);
	free(w);
}


void
resize_window (window_id_t w_id, int width, int height)
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_id);

	SDL_SetWindowSize(w->window, width, height);
}


void
repos_window (window_id_t w_id, int x, int y)
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_id);

	SDL_SetWindowPosition(w->window, x, y);
}


static
void
dispatch_keypress (EventQueue* queue, SDL_Event event)
{
	Event current;

	if (queue->len >= EVENT_BUFFER_SIZE)
		return;

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


static
void
_dispatch_mouse_motion (EventQueue* queue, SDL_Event event)
{
	Event current;

	if (queue->len >= EVENT_BUFFER_SIZE)
		return;

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


static
void
dispatch_mouse_button (EventQueue* queue, SDL_Event event)
{
	Event current;

	if (queue->len >= EVENT_BUFFER_SIZE)
		return;

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

static
void
dispatch_window_close (EventQueue* queue, SDL_Event event)
{
	Event current;

	if (queue->len >= EVENT_BUFFER_SIZE)
		return;

	current.type = CLOSE_EVENT;
	current.timestamp = event.common.timestamp;

	queue->events[queue->len] = current;
	queue->len++;
}


static
void
dispatch_window_resize (EventQueue* queue, SDL_Event event)
{
	Event current;

	if (queue->len >= EVENT_BUFFER_SIZE)
		return;

	current.type = RESIZE_EVENT;
	current.timestamp = event.common.timestamp;

	current.resize_event.width = event.window.data1;
	current.resize_event.height = event.window.data2;

	queue->events[queue->len] = current;
	queue->len++;
}


static
void
dispatch_window_repos (EventQueue* queue, SDL_Event event)
{
	Event current;

	if (queue->len >= EVENT_BUFFER_SIZE)
		return;

	current.type = REPOS_EVENT;
	current.timestamp = event.common.timestamp;

	current.repos_event.x = event.window.data1;
	current.repos_event.y = event.window.data2;

	queue->events[queue->len] = current;
	queue->len++;
}


EventQueue*
process_window (window_id_t w_id)
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_id);

	SDL_GL_MakeCurrent(w->window, w->context);

	// Temp
	glClearColor(
	    WINDOW_FILL_COLOR_R,
	    WINDOW_FILL_COLOR_G,
	    WINDOW_FILL_COLOR_B,
	    1.0
	);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	SDL_GL_SwapWindow(w->window);

	// Process all events and clear the SDL queue
	SDL_Event _;
	while (SDL_PollEvent(&_)) {}

	EventQueue* queue = w->queue;

	w->queue = (EventQueue*)malloc(sizeof(EventQueue));
	w->queue->events = (Event*)malloc(EVENT_BUFFER_SIZE * sizeof(Event));
	w->queue->len = 0;

	return queue;
}

/*
	Forms event queues to send
*/
int
event_queue_former (void* _, SDL_Event* event_ptr)
{
	SDL_Event event = *event_ptr;

	// TODO Check if window is present in window_pool

	if (event.type == SDL_MOUSEMOTION) {
		_dispatch_mouse_motion(queue_from_event_type(event, motion), event);
	}

	else if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP)) {
		dispatch_keypress(queue_from_event_type(event, window), event);
	}

	else if (event.type == SDL_MOUSEBUTTONUP) {
		dispatch_mouse_button(queue_from_event_type(event, button), event);
	}

	else if (event.type == SDL_WINDOWEVENT) {
		switch (event.window.event) {
		case SDL_WINDOWEVENT_CLOSE:
			dispatch_window_close(queue_from_event_type(event, window), event);
			break;
		case SDL_WINDOWEVENT_RESIZED:
			dispatch_window_resize(queue_from_event_type(event, window), event);
			break;
		case SDL_WINDOWEVENT_MOVED:
			dispatch_window_repos(queue_from_event_type(event, window), event);
			break;
		default:
			break;
		}
	}

	return 0;
}


void
free_event_queue (EventQueue* queue)
{
	free(queue->events);
	free(queue);
}
