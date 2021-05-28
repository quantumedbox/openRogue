// C API for various low-level functionalities which should be done outside of python

// TODO REDO

#include <inttypes.h>
#include <stdbool.h>

typedef uint32_t window_id_t;

//
typedef uint32_t bitmask_t;

#define MASK_SET_BIT(mask, bit) 	mask |= bit
#define MASK_ZERO_BIT(mask, bit)	mask ^= bit

// TODO Unified sindow creation hints

#include "definitions.h"

#define CONTEXT_OPENGL

#ifdef CONTEXT_OPENGL
#include "window/opengl/window.c"
#else
#error "No renderer specified.\nVariants: CONTEXT_OPENGL\n"
#endif
