/*
	Cross compiler definition for threading and thread safety

	pthread-like named macros naming is used for crossplatform usage
*/

#pragma once

// Needs testing
// TODO Error checking

#ifdef _MSC_VER
#include <windows.h>
#define rogue_mutex_t HANDLE
#define rogue_mutex_init(m) m = CreateMutex(NULL, FALSE, NULL)
#define rogue_mutex_lock(m) WaitForSingleObject(m, INFINITE)
#define rogue_mutex_unlock(m) ReleaseMutex(m)
#define rogue_mutex_destroy(m) CloseHandle(m)
#endif

#ifdef __GNUC__
#include <pthread.h>
#define rogue_mutex_t pthread_mutex_t
#define rogue_mutex_init(m) pthread_mutex_init(&m, NULL)
#define rogue_mutex_lock(m) pthread_mutex_lock(&m)
#define rogue_mutex_unlock(m) pthread_mutex_unlock(&m)
#define rogue_mutex_destroy(m) pthread_mutex_destroy(&m)
#endif
