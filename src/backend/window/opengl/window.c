#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>
// #include <cglm/cglm.h>

#include "backend.h"
#include "window.h"
#include "map.h"
#include "text/text.h"
#include "error.h"


// TODO Positioning of new windows depending on existing ones. Maybe require the caller to specify positions and rely on ui positioning?

// ??? Is it possible that event_queue_former could be called from sdl at the same time api caller swaps queues ???

// TODO start_drawing(key_t) 	function that prepares window and its context for future drawing commands
// TODO end_drawing(key_t) 	function that signals that drawing for a given window is finished and the contents of buffer could be showed

// TODO Put event logic in a separate file

// TODO It is possible to negate the need to allocate new event queues by having 2 event queues for every window that could be switched back and forth on demand


// Helper for getting window ids from event union fields
// #define queue_from_event_type(event, type) ((WindowHandler*)mapGet(window_pool, event.type.windowID))->queue


// ----------------------------------------------------------------- Global objects -- //


static SDL_GLContext opengl_context;

// Global index counter for giving unique keys for each window
static key_t new_winodw_key;

// Stores window handlers by their id
Map* window_pool = NULL;

// Active render window that is bound
key_t current_drawing_window;


// --------------------------------------------------------------------------- Misc -- //


static int event_queue_former(void*, SDL_Event*);


// TODO
// Idea is that APIs could give the information about their functionalities
// The fact that it is done via strings gives it the ability to have non-standard features without modifying headers or engines
const char* FEATURE_LIST[] = {
	"shaders",
	"...",
};

// TODO It is kinda bad to return char**, maybe we should do something else?
const char** get_feature_list ()
{
	return FEATURE_LIST;
}


// -------------------------------------------------------------------- Realization -- //


static
key_t
get_new_window_key()
{
	return ++new_winodw_key;
}


static
int
init_window_subsystem()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error on SDL initialization:\n%s\n", SDL_GetError());
		// SIGNAL_ERROR();
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, true);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VER);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VER);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
	// SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if (SDL_GL_LoadLibrary(NULL) != 0) {
		fprintf(stderr, "Error on SDL OpenGL extenion initialization:\n%s\n", SDL_GetError());
		// SIGNAL_ERROR();
		return -1;
	}

	window_pool = mapNew();

	return 0;
}

/*
	Create new window
	This function is entry for an API and thus, by calling it first time initializes everything
	On failure it should reverse all states to back they were
	Failure itself is signaled by 0 returned value
*/
EXPORT_SYMBOL
key_t
init_window( int width, int height, const char* title )
{
	key_t win_key = get_new_window_key();

	if (win_key == 1) {
		if (init_window_subsystem() == -1) {
			new_winodw_key = 0;
			return 0;
		}
	}

	if (title == 0) {
		title = DEFAULT_WINDOW_NAME;
	}

	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, window_pool->len ? true : false);
	SDL_Window* win = SDL_CreateWindow(
	                      title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                      width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	                  );

	if (!win) {
		fprintf(stderr, "Could not create OpenGL window: %s\n", SDL_GetError());
		// SIGNAL_ERROR();
		if (win_key == 1) {
			SDL_Quit();
			new_winodw_key = 0;
		}
		return 0;
	}

	// Make sure that only one OpenGL context is created
	if (win_key == 1)
	{
		opengl_context = SDL_GL_CreateContext(win);

		GLenum error;
		if ((error = glewInit()) != GLEW_OK) {
			fprintf(stderr, "Error on GLEW initialization: %s\n", glewGetErrorString(error));
			// SIGNAL_ERROR();
			SDL_GL_DeleteContext(opengl_context);
			SDL_Quit();
			new_winodw_key = 0;
			return 0;
		}

		SDL_GL_MakeCurrent(win, opengl_context);

		glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);

		// -1 for adaptive vsync, if isn't supported - use regular one
		if(SDL_GL_SetSwapInterval(-1) == -1) {
			SDL_GL_SetSwapInterval(1);
		}

		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
		glEnable(GL_MULTISAMPLE);

		if (init_text_subsystem() == -1) {
			SDL_GL_DeleteContext(opengl_context);
			SDL_Quit();
			new_winodw_key = 0;
			return 0;
		}
	}

	WindowHandler* handler = (WindowHandler*)calloc(1, sizeof(WindowHandler));
	handler->window = win;
	// handler->context = context;
	handler->id = SDL_GetWindowID(win);

	handler->queue1 = (EventQueue*)malloc(sizeof(EventQueue));
	handler->queue1->events = (Event*)malloc(EVENT_BUFFER_SIZE * sizeof(Event));
	handler->queue1->len = 0;

	handler->queue0 = (EventQueue*)malloc(sizeof(EventQueue));
	handler->queue0->events = (Event*)malloc(EVENT_BUFFER_SIZE * sizeof(Event));
	handler->queue0->len = 0;

	handler->current_queue = 0;

	mutex_init(handler->lock);

	mapAdd(window_pool, win_key, handler);

	if (win_key == 1) {
		SDL_AddEventWatch(event_queue_former, NULL);
	}

	return win_key;
}

// ??? Should we exit all systems when all windows are closed ?
// It should be a valid concern if we allow swapping APIs at runtime
EXPORT_SYMBOL
void
close_window( key_t w_key )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return;

	mutex_lock(w->lock);

	mapDel(window_pool, w_key);

	free(w->queue0->events);
	free(w->queue0);

	free(w->queue1->events);
	free(w->queue1);

	// SDL_GL_DeleteContext(w->context);
	SDL_DestroyWindow(w->window);

	mutex_unlock(w->lock);

	// There's possibility of other mutexes waiting for window
	// Should somehow secure them by second map checking or smt
	mutex_destroy(w->lock);

	free(w);
}


EXPORT_SYMBOL
void
resize_window( key_t w_key, int width, int height )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return;

	// mutex_lock(w->lock);

	SDL_SetWindowSize(w->window, width, height);

	// mutex_unlock(w->lock);
}


EXPORT_SYMBOL
void
repos_window( key_t w_key, int x, int y )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return;

	// mutex_lock(w->lock);

	SDL_SetWindowPosition(w->window, x, y);

	// mutex_unlock(w->lock);
}


static
void
dispatch_keypress( EventQueue* queue, SDL_Event event )
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
dispatch_mouse_motion( EventQueue* queue, SDL_Event event )
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
dispatch_mouse_button( EventQueue* queue, SDL_Event event )
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
dispatch_window_close( EventQueue* queue, SDL_Event event )
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
dispatch_window_resize( EventQueue* queue, SDL_Event event )
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
dispatch_window_repos( EventQueue* queue, SDL_Event event )
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


// TODO Rename to something more fitting
/*
	@brief	Main way of processing window events
			It returns one of the window queues

	@warn 	You cannot process both queues,
			make sure that only one of them are in python space

	@return Reference to EventQueue struct or NULL if window key is not valid
*/
EXPORT_SYMBOL
EventQueue*
process_window( key_t w_key )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return NULL;

	// Process all events and clear the SDL queue
	SDL_Event _;
	while (SDL_PollEvent(&_)) {}

	mutex_lock(w->lock);

	if (w->current_queue == 0) {
		w->current_queue = 1;
		w->queue1->len = 0;
		mutex_unlock(w->lock);
		return w->queue0;
	} else {
		w->current_queue = 0;
		w->queue0->len = 0;
		mutex_unlock(w->lock);
		return w->queue1;
	}

	// EventQueue* queue = w->queue;
	// w->queue = (EventQueue*)malloc(sizeof(EventQueue));
	// w->queue->events = (Event*)malloc(EVENT_BUFFER_SIZE * sizeof(Event));
	// w->queue->len = 0;
}

/*
	@brief 	SDL event watcher that is used for queuing multiple windows at a time
*/
static
int
event_queue_former( void* _, SDL_Event* event_ptr )
{
	SDL_Event event = *event_ptr;

	if (event.type == SDL_MOUSEMOTION) {
		WindowHandler* w = (WindowHandler*)mapGet(window_pool, event.motion.windowID);
		if (!w) return 0;
		mutex_lock(w->lock);
		dispatch_mouse_motion(w->current_queue ? w->queue1 : w->queue0, event);
		mutex_unlock(w->lock);
	}

	else if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP)) {
		WindowHandler* w = (WindowHandler*)mapGet(window_pool, event.motion.windowID);
		if (!w) return 0;
		mutex_lock(w->lock);
		dispatch_keypress(w->current_queue ? w->queue1 : w->queue0, event);
		mutex_unlock(w->lock);
	}

	else if (event.type == SDL_MOUSEBUTTONUP) {
		WindowHandler* w = (WindowHandler*)mapGet(window_pool, event.button.windowID);
		if (!w) return 0;
		mutex_lock(w->lock);
		dispatch_mouse_button(w->current_queue ? w->queue1 : w->queue0, event);
		mutex_unlock(w->lock);
	}

	else if (event.type == SDL_WINDOWEVENT) {
		WindowHandler* w = (WindowHandler*)mapGet(window_pool, event.window.windowID);
		if (!w) return 0;
		mutex_lock(w->lock);

		switch (event.window.event) {
		case SDL_WINDOWEVENT_CLOSE:
			dispatch_window_close(w->current_queue ? w->queue1 : w->queue0, event);
			break;
		case SDL_WINDOWEVENT_RESIZED:
			dispatch_window_resize(w->current_queue ? w->queue1 : w->queue0, event);
			break;
		case SDL_WINDOWEVENT_MOVED:
			dispatch_window_repos(w->current_queue ? w->queue1 : w->queue0, event);
			break;
		default:
			break;
		}

		mutex_unlock(w->lock);
	}

	return 0;
}

/*
	@brief 	Prepares the window for drawing
*/
EXPORT_SYMBOL
void
start_drawing( key_t w_key )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (w == NULL)
		return;

	SDL_GL_MakeCurrent(w->window, opengl_context);

	current_drawing_window = w_key;

	SDL_GetWindowSize(w->window, &w->width, &w->height);

	glViewport(0, 0, w->width, w->height);

	// float aspect = (float)width / (float)height;

	// Setup the projection
	// glm_ortho(-(float)width / 2, (float)width / 2,
	//           -(float)height / 2 * aspect, (float)height / 2 * aspect,
	//           0.1f, 100.0f,
	//           current_window_projection);

	// glm_ortho(0.0, (float)width,
	//           0.0, (float)height,
	//           -100.0f, 100.0f,
	//           current_window_projection);

	// glOrtho(-0.5, (float)(width - 1) + 0.5, (float)(height - 1) + 0.5, -0.5, 0.0, 1.0);
}

/*
	@brief 	Update the current drawing window with what is in context
			Actual clearing of the buffer happens by this function and not by start_drawing()
*/
EXPORT_SYMBOL
void
finish_drawing()
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, current_drawing_window);
	if (w == NULL)
		return;

	glFlush();
	SDL_GL_SwapWindow(w->window);

	glClearColor(
	    WINDOW_FILL_COLOR_R,
	    WINDOW_FILL_COLOR_G,
	    WINDOW_FILL_COLOR_B,
	    1.0
	);
	glClear(GL_COLOR_BUFFER_BIT);

	current_drawing_window = 0;
}
