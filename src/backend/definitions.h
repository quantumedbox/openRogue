#include <inttypes.h>


// Generic event for all controllers that describe actions in relative position
typedef struct {
	bitmask_t mouse_action;
	bool is_pressed;
	// TODO touch_screen_action; i.e touch_down, touch_press, touch_up
	// TODO controllers axis motions
	int32_t x;
	int32_t y;
	int32_t x_motion;
	int32_t y_motion;
}
PointerEvent;

// Generic event for all controllers that describe named actions
typedef struct {
	bitmask_t action;
	bool is_key_pressed;
	bool is_key_repeat;
	uint32_t keycode;
	bitmask_t keymod;
}
InputEvent;


typedef struct {
	bitmask_t type;
	uint32_t timestamp;
	// uint32_t windowid; // events should always be dispatched by window
	union {
		PointerEvent pointer_event;
		InputEvent input_event;
		// TODO text_event; // Unicode text output
	};
}
Event;

// 
typedef struct {
	Event* events;
	len_t len;
	bitmask_t window_signals;
}
EventQueue;


enum EventType {
	POINTER_EVENT 		= 1,
	INPUT_EVENT 		= 2,
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
	INPUT_CLEAR			= 0,
	INPUT_KEYDOWN		= 1,
	INPUT_KEYUP			= 2,
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

// Window change signals that used to form bit mask
enum WINDOW_SIGNAL {
	WINDOW_SIGNAL_CLEAR = 0,
	WINDOW_SIGNAL_CLOSED = 1,
	WINDOW_SIGNAL_RESIZED = 2,
	WINDOW_SIGNAL_SHOWN = 4,
	WINDOW_SIGNAL_HIDDEN = 8,
	WINDOW_SIGNAL_EXPOSED = 16,
	WINDOW_SIGNAL_MOVED = 32,
	WINDOW_SIGNAL_MINIMIZED = 64,
	WINDOW_SIGNAL_MAXIMIZED = 128,
	WINDOW_SIGNAL_RESTORED = 256,
	WINDOW_SIGNAL_MOUSE_ENTERED = 512,
	WINDOW_SIGNAL_MOUSE_EXITED = 1024,
	WINDOW_SIGNAL_FOCUS_GAINED = 2048,
	WINDOW_SIGNAL_FOCUS_LOST = 4096,
};
