#pragma once

#include <SDL2/SDL_video.h>

#include "rogue_events.h"
#include "threads.h"

// -------------------------------------------------------------------- Definitions -- //


typedef struct {
	SDL_Window* window;
	// SDL_GLContext context; // Now GL context is global for all windows of single thread

	// SDL Window ID
	uint32_t id;

	// Switching queues
	// At given time only one of them should be writable and another - readable
	int8_t current_queue;
	EventQueue* queue0;
	EventQueue* queue1;

	// Updated on start_drawing()
	int width;
	int height;

	// ??? Should they be here ? or it's better to have global counter
	uint32_t time_delta;
	uint32_t prev_timestamp;

	// Used for preventing sigegiv from SDL EventWatch thread
	rogue_mutex_t lock;
}
WindowHandler;


// ----------------------------------------------------------------------- Internal -- //

/*
	@brief 	Returns current window key that is bound for drawing
*/
key_t get_current_drawing_window();


// ----------------------------------------------------------------------- Exported -- //


// ??? Problem: if we decide to use this backend with other languages - we will have to
//				find ways to evaluate these strings which is not convenient
/*
	@brief	Way of getting API specifications

	@return wchar string that contains data that could be evaluated to valid Python
*/
ROGUE_EXPORT wchar_t* get_spec( const char* spec );

/*
	@brief 	Create new window
			This function is entry for an API and thus, by calling it for the first time it initializes everything
			On failure it should reverse all states to back they were

	@return Hash key of newly created window or 0 on failure
*/
ROGUE_EXPORT key_t init_window( int width, int height, const char* title );

/*
*/
ROGUE_EXPORT void close_window( key_t );

/*
	@brief	Main way of getting and processing window events
			It returns one of the window queues

	@warn 	You cannot process both queues,
			make sure that only one of them are in python space

	@return Reference to EventQueue struct or NULL if window key is not valid
*/
ROGUE_EXPORT EventQueue* get_window_events( key_t );

/*
*/
ROGUE_EXPORT void resize_window( key_t, int w, int h );

/*
*/
ROGUE_EXPORT void repos_window( key_t, int x, int y );

/*
	@brief 	Prepares the window for drawing
			Should be called before any drawing functions
*/
ROGUE_EXPORT void start_drawing( key_t );

/*
	@brief 	Update the current drawing window with what is in context
			Actual clearing of buffer happens by this function
*/
ROGUE_EXPORT void finish_drawing();

/*
	@brief 	Set window icon from image file at path

	@return Returns non-zero value on failure, otherwise - 0
*/
ROGUE_EXPORT int set_window_icon_from_file( key_t, const char* path );

/*
	Functions for getting actual system window metrics
*/
ROGUE_EXPORT int get_window_x_position( key_t w_key );
ROGUE_EXPORT int get_window_y_position( key_t w_key );

ROGUE_EXPORT int get_window_width( key_t w_key );
ROGUE_EXPORT int get_window_height( key_t w_key );
