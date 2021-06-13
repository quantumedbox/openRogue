/*
	Commonly used types and definitions throughout the backend code
*/

#pragma once

#include <inttypes.h>

#include "system.h"

// ----------------------------------------------------- Numeric key -- //

typedef size_t key_t;

#define NONE_KEY 0

// --------------------------------------------------------- Bitmask -- //

typedef uint32_t bitmask_t;

#define mask_set_bit(mask, bit)  mask |= bit
#define mask_zero_bit(mask, bit) mask ^= bit

// ------------------------------------------------------- Hex color -- //

typedef uint32_t hex_t;

#define hex_r_float(hex) ((hex & 0xFF000000) >> 24) / 255.f
#define hex_g_float(hex) ((hex & 0xFF0000) >> 16) / 255.f
#define hex_b_float(hex) ((hex & 0xFF00) >> 8) / 255.f
#define hex_a_float(hex) (hex & 0xFF) / 255.f

// Helper for setting the shader uniform 4f values
#define hex_uniform4f(hex) hex_r_float(hex), hex_g_float(hex), hex_b_float(hex), hex_a_float(hex)

// Helper for setting the shader uniform 3f values, without alpha transparency
#define hex_uniform3f(hex) hex_r_float(hex), hex_g_float(hex), hex_b_float(hex)
