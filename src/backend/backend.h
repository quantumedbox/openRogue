// C API for various low-level functionalities which should be done outside of python

#include <inttypes.h>

#include "../metastack.h"


typedef uint32_t bitmask;

#define MASK_SET_BIT(mask, bit) 	mask |= bit
#define MASK_ZERO_BIT(mask, bit)	mask ^= bit

// Window change signals that used to form bit mask
typedef enum {
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
}
WINDOW_SIGNAL;

#define CONTEXT_OPENGL

#ifdef CONTEXT_OPENGL
#include "window/opengl/window.c"
#else
#error "No renderer specified.\nVariants: CONTEXT_OPENGL\n"
#endif
