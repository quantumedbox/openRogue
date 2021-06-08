/*
	Cross compiler definition for threading and thread safety
*/

#pragma once

// Needs testing
// TODO Error checking

#ifdef _MSC_VER
#include <windows.h>
#define mutex_t HANDLE
#define mutex_init(mutex) mutex = CreateMutex(NULL, FALSE, NULL)
#define mutex_lock(mutex) WaitForSingleObject(mutex, INFINITE)
#define mutex_unlock(mutex) ReleaseMutex(mutex)
#define mutex_destroy(mutex) CloseHandle(mutex)
#endif

#ifdef __GNUC__
#include <pthread.h>
#define mutex_t pthread_mutex_t
#define mutex_init(mutex) pthread_mutex_init(&mutex, NULL)
#define mutex_lock(mutex) pthread_mutex_lock(&mutex)
#define mutex_unlock(mutex) pthread_mutex_unlock(&mutex)
#define mutex_destroy(mutex) pthread_mutex_destroy(&mutex)
#endif
