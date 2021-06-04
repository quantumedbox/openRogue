// C API for various low-level functionalities which should be done outside of python

// TODO REDO

#include <inttypes.h>
#include <stdbool.h>

typedef uint32_t window_id_t;

//
typedef uint32_t bitmask_t;

#define mask_set_bit(mask, bit) mask |= bit
#define mask_zero_bit(mask, bit) mask ^= bit

typedef uint32_t hex_t;

#define hex_r_float(hex) ((hex & 0xFF000000) >> 24) / 255.f
#define hex_g_float(hex) ((hex & 0xFF0000) >> 16) / 255.f
#define hex_b_float(hex) ((hex & 0xFF00) >> 8) / 255.f
#define hex_a_float(hex) (hex & 0xFF) / 255.f

// Helper for setting the shader uniform 4f values
#define hex_uniform4f(hex) hex_r_float(hex), hex_g_float(hex), hex_b_float(hex), hex_a_float(hex)

// Helper for setting the shader uniform 3f values, without alpha transparency
#define hex_uniform3f(hex) hex_r_float(hex), hex_g_float(hex), hex_b_float(hex)

// TODO Unified sindow creation hints

#include "definitions.h"

#define CONTEXT_OPENGL

#ifdef CONTEXT_OPENGL
#include "window/opengl/window.c"
#else
#error "No renderer specified.\nVariants: CONTEXT_OPENGL\n"
#endif
