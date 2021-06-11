/*
	Event definitions
*/

#pragma once

#include <inttypes.h>

/*
	Generic event for all controllers which describes actions in relative position
*/
struct PointerEvent {
	bitmask_t mouse_action;
	bool is_pressed;
	// TODO touch_screen_action; i.e touch_down, touch_press, touch_up
	// TODO controllers axis motions
	int32_t x;
	int32_t y;
	int32_t x_motion;
	int32_t y_motion;
};

/*
	Generic event for all controllers which describes named actions
*/
struct InputEvent {
	bitmask_t action;
	bool is_key_pressed;
	bool is_key_repeat;
	uint32_t keycode;
	bitmask_t keymod;
};

/*
	System window was resized
*/
struct ResizeEvent
{
	int32_t width;
	int32_t height;
};

/*
	System window was repositioned
*/
struct ReposEvent
{
	int32_t x;
	int32_t y;
};

/*
	Event union that encapsulates every event type
*/
typedef struct {
	bitmask_t type;
	// uint32_t timestamp;
	// uint32_t windowid; // events should always be dispatched by event window
	union {
		struct PointerEvent pointer_event;
		struct InputEvent input_event;
		struct ResizeEvent resize_event;
		struct ReposEvent repos_event;
		// TODO text_event; // Unicode text output
	};
}
Event;

/*
	Used for containing and delivering event signals to API caller
*/
typedef struct {
	Event* events;
	uint32_t len;
}
EventQueue;


enum EventType {
	UNKNOWN_EVENT	= 0,
	POINTER_EVENT 	= 1,
	INPUT_EVENT 	= 2,
	CLOSE_EVENT		= 4,
	RESIZE_EVENT	= 8,
	REPOS_EVENT 	= 16,
};


enum MouseAction {
	MOUSE_CLEAR 		= 0,
	MOUSE_MOTION		= 1,
	MOUSE_BUTTON_LEFT 	= 2,
	MOUSE_BUTTON_RIGHT 	= 4,
	MOUSE_BUTTON_MIDDLE = 8,
	MOUSE_WHEEL_UP 		= 16,
	MOUSE_WHEEL_DOWN 	= 32,
};


enum InputAction {
	INPUT_CLEAR		= 0,
	INPUT_KEYDOWN	= 1,
	INPUT_KEYUP		= 2,
};


enum KeyMod {
	KEYMOD_NONE			= 0,
	KEYMOD_LEFT_SHIFT	= 1,
	KEYMOD_RIGHT_SHIFT	= 2,
	KEYMOD_LEFT_CTRL	= 4,
	KEYMOD_RIGHT_CTRL	= 8,
	KEYMOD_LEFT_ALT		= 16,
	KEYMOD_RIGHT_ALT	= 32,
};
