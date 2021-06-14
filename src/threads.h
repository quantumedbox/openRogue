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
#define rogue_mutex_init(mutex) mutex = CreateMutex(NULL, FALSE, NULL)
#define rogue_mutex_lock(mutex) WaitForSingleObject(mutex, INFINITE)
#define rogue_mutex_unlock(mutex) ReleaseMutex(mutex)
#define rogue_mutex_destroy(mutex) CloseHandle(mutex)
#endif

#ifdef __GNUC__
#include <pthread.h>
#define rogue_mutex_t pthread_mutex_t
#define rogue_mutex_init(mutex) pthread_mutex_init(&mutex, NULL)
#define rogue_mutex_lock(mutex) pthread_mutex_lock(&mutex)
#define rogue_mutex_unlock(mutex) pthread_mutex_unlock(&mutex)
#define rogue_mutex_destroy(mutex) pthread_mutex_destroy(&mutex)
#endif
