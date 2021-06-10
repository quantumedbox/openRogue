#pragma once

/*
	System specific definitions
*/

#ifdef _MSC_VER
#define ROGUE_EXPORT __declspec(dllexport)
#else
#define ROGUE_EXPORT //
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#endif
