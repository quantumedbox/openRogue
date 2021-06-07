/*
	System specific definitions
*/

#pragma once

#ifdef _MSC_VER
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL //
#endif
