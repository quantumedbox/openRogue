#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "rogue_definitions.h"
#include "rogue_events.h"
#include "window.h"
#include "map.h"
#include "render.h"

#define OPENGL_MINOR_VER 3
#define OPENGL_MAJOR_VER 4

#define DEFAULT_WINDOW_NAME "openRogue"

#define WINDOW_FILL_COLOR_R 0.2F
#define WINDOW_FILL_COLOR_G 0.0F
#define WINDOW_FILL_COLOR_B 0.062F

// Size of staticly allocated event queue buffer
#define EVENT_BUFFER_SIZE 16


// TODO Positioning of new windows depending on existing ones. Maybe require the caller to specify positions and rely on ui positioning?

// TODO Put event logic in a separate file

// ----------------------------------------------------------------- Global objects -- //


static SDL_GLContext opengl_context;

// Global index counter for giving unique keys for each window
static key_t new_winodw_key;

// Active render window that is bound, should be returned by get_current_drawing_window()
static key_t current_drawing_window;

// Stores window handlers by their id
Map* window_pool = NULL;


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
ROGUE_EXPORT
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

/*
	Used for getting currently bound window for drawing
	It is in function form for preventing direct access from other submodules
*/
key_t
get_current_drawing_window()
{
	return current_drawing_window;
}


static
int
init_window_subsystem()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error on SDL initialization:\n%s\n", SDL_GetError());
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
		return -1;
	}

	window_pool = mapNew();

	return 0;
}


ROGUE_EXPORT
key_t
init_window( int width, int height, const char* title )
{
	key_t w_key = get_new_window_key();

	if (w_key == 1) {
		if (init_window_subsystem() == -1) {
			new_winodw_key = 0;
			return NONE_KEY;
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
		if (w_key == 1) {
			SDL_Quit();
			new_winodw_key = 0;
		}
		return NONE_KEY;
	}

	// Make sure that only one OpenGL context is created
	if (w_key == 1)
	{
		opengl_context = SDL_GL_CreateContext(win);

		GLenum error;
		if ((error = glewInit()) != GLEW_OK) {
			fprintf(stderr, "Error on GLEW initialization: %s\n", glewGetErrorString(error));
			SDL_GL_DeleteContext(opengl_context);
			SDL_Quit();
			new_winodw_key = 0;
			return NONE_KEY;
		}

		SDL_GL_MakeCurrent(win, opengl_context);

		glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);

		// -1 for adaptive vsync, if isn't supported - use regular one
		if (SDL_GL_SetSwapInterval(-1) == -1) {
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
			return NONE_KEY;
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

	rogue_mutex_init(handler->lock);

	mapAdd(window_pool, w_key, handler);

	if (w_key == 1) {
		SDL_AddEventWatch(event_queue_former, NULL);
	}

	return w_key;
}

// ??? Should we exit all systems when all windows are closed ?
// It should be a valid concern if we allow swapping APIs at runtime
ROGUE_EXPORT
void
close_window( key_t w_key )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return;

	rogue_mutex_lock(w->lock);

	mapDel(window_pool, w_key);

	free(w->queue0->events);
	free(w->queue0);

	free(w->queue1->events);
	free(w->queue1);

	// SDL_GL_DeleteContext(w->context);
	SDL_DestroyWindow(w->window);

	rogue_mutex_unlock(w->lock);

	// There's possibility of other mutexes waiting for window
	// Should somehow secure them by second map checking or smt
	rogue_mutex_destroy(w->lock);

	free(w);
}


ROGUE_EXPORT
void
resize_window( key_t w_key, int width, int height )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return;

	// rogue_mutex_lock(w->lock);

	SDL_SetWindowSize(w->window, width, height);

	// rogue_mutex_unlock(w->lock);
}


ROGUE_EXPORT
void
repos_window( key_t w_key, int x, int y )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return;

	// rogue_mutex_lock(w->lock);

	SDL_SetWindowPosition(w->window, x, y);

	// rogue_mutex_unlock(w->lock);
}


static
void
dispatch_keypress( EventQueue* queue, SDL_Event event )
{
	Event current;

	if (queue->len >= EVENT_BUFFER_SIZE)
		return;

	current.type = INPUT_EVENT;
	// current.timestamp = event.common.timestamp;

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
	// current.timestamp = event.common.timestamp;

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
	// current.timestamp = event.common.timestamp;

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
	// current.timestamp = event.common.timestamp;

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
	// current.timestamp = event.common.timestamp;

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
	// current.timestamp = event.common.timestamp;

	current.repos_event.x = event.window.data1;
	current.repos_event.y = event.window.data2;

	queue->events[queue->len] = current;
	queue->len++;
}


// TODO Rename to something more fitting
ROGUE_EXPORT
EventQueue*
get_window_events( key_t w_key )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return NULL;

	// Process all events and clear the SDL queue
	SDL_Event _;
	while (SDL_PollEvent(&_)) {}

	rogue_mutex_lock(w->lock);

	if (w->current_queue == 0) {
		w->current_queue = 1;
		w->queue1->len = 0;
		rogue_mutex_unlock(w->lock);
		return w->queue0;
	} else {
		w->current_queue = 0;
		w->queue0->len = 0;
		rogue_mutex_unlock(w->lock);
		return w->queue1;
	}
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
		rogue_mutex_lock(w->lock);
		dispatch_mouse_motion(w->current_queue ? w->queue1 : w->queue0, event);
		rogue_mutex_unlock(w->lock);
	}

	else if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP)) {
		WindowHandler* w = (WindowHandler*)mapGet(window_pool, event.motion.windowID);
		if (!w) return 0;
		rogue_mutex_lock(w->lock);
		dispatch_keypress(w->current_queue ? w->queue1 : w->queue0, event);
		rogue_mutex_unlock(w->lock);
	}

	else if (event.type == SDL_MOUSEBUTTONUP) {
		WindowHandler* w = (WindowHandler*)mapGet(window_pool, event.button.windowID);
		if (!w) return 0;
		rogue_mutex_lock(w->lock);
		dispatch_mouse_button(w->current_queue ? w->queue1 : w->queue0, event);
		rogue_mutex_unlock(w->lock);
	}

	else if (event.type == SDL_WINDOWEVENT) {
		WindowHandler* w = (WindowHandler*)mapGet(window_pool, event.window.windowID);
		if (!w) return 0;
		rogue_mutex_lock(w->lock);

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

		rogue_mutex_unlock(w->lock);
	}

	return 0;
}


ROGUE_EXPORT
void
start_drawing( key_t w_key )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return;

	SDL_GL_MakeCurrent(w->window, opengl_context);

	current_drawing_window = w_key;

	SDL_GetWindowSize(w->window, &w->width, &w->height);

	// Align the viewport size to be even for the consistent pixel alignment
	if (w->width % 2 != 0)
		w->width--;

	if (w->height % 2 != 0)
		w->height--;

	glViewport(0, 0, w->width, w->height);

	// float aspect = (float)width / (float)height;

	// glm_ortho(0.0, (float)width,
	//           0.0, (float)height,
	//           0.0f, 100.0f,
	//           current_window_projection);

	// glOrtho(-0.5, (float)(w->width - 1) + 0.5, (float)(w->height - 1) + 0.5, -0.5, 0.0, 1.0);
}


ROGUE_EXPORT
void
finish_drawing()
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, current_drawing_window);
	if (!w) return;

	glFlush();
	SDL_GL_SwapWindow(w->window);

	glClearColor(
	    WINDOW_FILL_COLOR_R,
	    WINDOW_FILL_COLOR_G,
	    WINDOW_FILL_COLOR_B,
	    1.0
	);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	current_drawing_window = 0;
}


ROGUE_EXPORT
int
set_window_icon_from_file( key_t w_key,
                           const char* path )
{
	WindowHandler* w = (WindowHandler*)mapGet(window_pool, w_key);
	if (!w) return -1;

	int width, height, n_channels;
	uint8_t *data = stbi_load(path, &width, &height, &n_channels, 4);

	if (!data) {
		fprintf(stderr, "Cannot load the image\n");
		return -1;
	}

	SDL_Surface* icon = SDL_CreateRGBSurfaceFrom(data, width, height,
	                    8 * n_channels, width * n_channels,
	                    0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

	if (!icon) {
		fprintf(stderr, "Cannot create surface from image data, error: %s\n", SDL_GetError());
		return -1;
	}

	SDL_SetWindowIcon(w->window, icon);

	SDL_FreeSurface(icon);

	stbi_image_free(data);

	return 0;
}
