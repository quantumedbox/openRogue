#pragma once

/*
	Naive map implementation
*/

// TODO Unit tests

// TODO Map func

// TODO Reimplement thread saftly (maybe make it optional on compile time)
// TODO Iteration through items
// TODO It's easy to leak memory in current implementation which is not good
// TODO Consideration for easy deallocation of custom data types

// #include <pthread.h>

#define MAP_INIT_CAPACITY 	16		// should be power of 2

#define MAP_INIT_THRESHOLD 	0.75

#define MAP_MAX_CAPACITY	4096


// -------------------------------------------------------------- Type degfinitions -- //

// Hash bitdepth is system-dependent
typedef size_t key_t;

typedef void* data_t;


typedef struct
{
	// !!! PROBLEM: first bucket elements cannot contain any data or key, but still have fields for it which is unnecessary
	struct Bucket* 	buckets;

	// num of allocated buckets in memory
	size_t 			capacity;

	// percentage threshold that triggers resizing if len / capacity > threshold // should be in (0, 1]
	float 			threshold;

	// item count
	size_t 			len;

	// mutex lock for accessing a map from multiple threads
	// pthread_mutex_t lock;
}
Map;

// ------------------------------------------------------------ Function signatures -- //

key_t 	hash_string (const char*);

Map* 	mapNew		();
void 	mapAdd 		(Map*, key_t key, data_t);
bool	mapHas		(Map*, key_t key);
data_t	mapGet		(Map*, key_t key);
void	mapDel		(Map*, key_t key);
void 	mapDestroy	(Map*);
void 	mapClear	(Map*);
void 	mapPrint 	(Map*);

static 	struct Bucket* mapNewBucket		();
static  void 	mapExtend			(Map*);
static 	void 	mapAllocateBuckets	(Map*, size_t n);
static  void 	mapExtend			(Map*);
static  void 	mapClearStack		(struct Bucket*);
static 	void 	mapPrintRecur		(struct Bucket*);

static  void 	mapReallocateBucketStack	(struct Bucket* buckets, struct Bucket* stack, size_t cap);
static  void 	mapReallocateBucketRecur	(struct Bucket* stack, struct Bucket* bucket);
