/*
	Naive map
*/
#pragma once

// TODO Unit tests

// TODO Optimize recurrent functions
// TODO Iteration through items
// TODO It's easy to leak memory in current implementation which is not good
// TODO Consideration for easy deallocation of custom data types

// TODO Possibly use smart pointers ?

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
// #include <pthread.h>

#define MAP_INIT_CAPACITY 	16		// should be the power of 2

#define MAP_INIT_THRESHOLD 	0.75

#define MAP_MAX_CAPACITY	4096

#ifdef USE_OMP
#define OPTIONAL_OMP_PARALLEL_FOR #pragma omp parallel for
#else
#define OPTIONAL_OMP_PARALLEL_FOR //
#endif

// -------------------------------------------------------------- Type degfinitions -- //

// Hash bitdepth is system-dependent
typedef size_t key_t;

typedef void* data_t;


// Stack-like structure that holds a key and void pointer to data
typedef struct Bucket
{
	key_t			key;

	void* 			data;

	struct Bucket* 	next;
}
Bucket;

typedef struct
{
	Bucket* 		buckets;

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

Map* 	mapNew		();
void 	mapAdd 		(Map*, key_t key, data_t);
bool	mapHas		(Map*, key_t key);
data_t	mapGet		(Map*, key_t key);
void	mapDel		(Map*, key_t key);
void 	mapDestroy	(Map*);
void 	mapClear	(Map*);
void 	mapPrint 	(Map*);

static 	Bucket* _mapNewBucket		();
static  void 	_mapExtend			(Map*);
static 	bool 	_mapHasKeyRecur		(Bucket*, key_t key);
static 	void 	_mapAllocateBuckets	(Map*, size_t n);
static 	bool 	_mapAddRecur		(Bucket*, key_t key, data_t);
static  void 	_mapExtend			(Map*);
static  void 	_mapClearStack		(Bucket*);
static 	void 	_mapPrintRecur		(Bucket*);

static  void 	_mapReallocateBucketStack	(Bucket* buckets, Bucket* stack, size_t cap);
static  void 	_mapReallocateBucketRecur	(Bucket* stack,   Bucket* bucket);

// ----------------------------------------------------------------- Hash functions -- //

/*
	Returns integer value that represents a value of given null terminated string
*/
size_t hash_string (char *s)
{
	const int p = 97;
	const int m = 1e9 + 9;
	size_t hash = 0;
	size_t p_pow = 1;

	unsigned int s_len = strlen(s);
	for (char *ps = s; ps - s < s_len; ++ps) {
		hash = (hash + (*ps - 31) * p_pow) % m;
		p_pow = (p_pow * p) % m;
	}

	return hash;
}

// ---------------------------------------------------------------------- Functions -- //

/*
	Allocates new map structure
	You have to call mapDestroy() to free the memory
*/
Map* mapNew()
{
	Map* new = (Map*)malloc(sizeof(Map));

	_mapAllocateBuckets(new, MAP_INIT_CAPACITY);

	// pthread_mutex_init(&new->lock, NULL);

	new->threshold 	= MAP_INIT_THRESHOLD;
	new->len 		= 0;

	return new;
}


static inline Bucket* _mapNewBucket()
{
	Bucket* new = (Bucket*)calloc(1, sizeof(Bucket));

	return new;
}


static inline void _mapAllocateBuckets(Map* m, size_t n)
{
	m->buckets = (Bucket*)calloc(n, sizeof(Bucket));
	m->capacity = n;
}

/*
	Add new elem under the key with reference to data
	WARNING! If given key is already in the map then the data will be replaced.
	If previous data was the reference to a heap-allocated segment - it will leak
*/
void mapAdd(Map* m, key_t key, data_t data)
{
	// pthread_mutex_lock(&m->lock);

	uint32_t idx = key % m->capacity;

	if (m->buckets[idx].next == NULL)
	{
		m->len += 1;

		Bucket* new = _mapNewBucket();
		m->buckets[idx].next = new;
		new->key 	= key;
		new->data 	= data;
	}
	else if (_mapAddRecur(m->buckets[idx].next, key, data))
		m->len += 1;

	if ((m->len < MAP_MAX_CAPACITY) && (((float)m->len / m->capacity) >= m->threshold))
		_mapExtend(m);

	// pthread_mutex_unlock(&m->lock);
}

/*
	Returns true if a new bucket was added to the stack
	Returns false if there was an item under the given key already and it was replaced
	WARNING! If replaced data was heap-allocated it leaks memory
*/
static inline bool _mapAddRecur(Bucket* b, key_t key, data_t data)
{
	if (b->key == key) {
		b->data = data;

		return false;
	}

	else if (b->next == NULL)
	{
		Bucket* new = _mapNewBucket();
		b->next 	= new;
		new->key 	= key;
		new->data 	= data;

		return true;
	}

	return _mapAddRecur(b->next, key, data);
}

/*
	Returns true if given key is present in the map, false if not
*/
bool mapHas(Map* m, key_t key)
{
	bool return_data = false;

	// pthread_mutex_lock(&m->lock);

	uint32_t idx = key % m->capacity;

	if (m->buckets[idx].next != NULL)
		return_data = _mapHasKeyRecur(m->buckets[idx].next, key);

	// pthread_mutex_unlock(&m->lock);

	return return_data;
}


static inline bool _mapHasKeyRecur(Bucket* b, key_t key)
{
	if (b->key == key)
		return true;

	else if (b->next == NULL)
		return false;

	return _mapHasKeyRecur(b->next, key);
}

/*
	Returns the void* data that is under given key or NULL if nothing to find
*/
data_t mapGet(Map* m, key_t key)
{
	uint32_t idx = key % m->capacity;

	Bucket* base = &m->buckets[idx];
	while (base->next != NULL)
	{
		if (base->next->key == key)
			return base->next->data;

		base = base->next;
	}

	return NULL;
}

/*
	Deletes the item with a given key
	WARNING! This does not free the allocated heap data of the item itself, only the inner data of map
*/
void mapDel(Map* m, key_t key)
{
	uint32_t idx = key % m->capacity;

	Bucket* base = &m->buckets[idx];
	while (base->next != NULL)
	{
		if (base->next->key == key)
		{
			Bucket* to_free = base->next;
			base->next = base->next->next;
			free(to_free);
			break;
		}
		base = base->next;
	}
}

/*
	Create new bucket array and relocate existing elems to it
*/
static void _mapExtend(Map* m)
{
	size_t old_capacity = m->capacity;

	m->capacity = pow(2, log2(m->capacity) + 1);

	Bucket* new_array = (Bucket*)calloc(m->capacity, sizeof(Bucket));

	OPTIONAL_OMP_PARALLEL_FOR
	for (register int i = 0; i < old_capacity; i++)
	{
		if (m->buckets[i].next != NULL)
			_mapReallocateBucketStack(new_array, m->buckets[i].next, m->capacity);
	}

	free(m->buckets);
	m->buckets = new_array;
}

/*
	Recursevly distribute all elements of bucket stack to the buckets array
*/
static void _mapReallocateBucketStack(Bucket* buckets, Bucket* stack, size_t cap)
{
	uint32_t idx = stack->key % cap;

	Bucket* stack_next = stack->next;

	if (buckets[idx].next == NULL) {
		buckets[idx].next = stack;
		stack->next = NULL;
	}
	else
		_mapReallocateBucketRecur(buckets[idx].next, stack);

	if (stack_next != NULL)
		_mapReallocateBucketStack(buckets, stack_next, cap);
}

/*
	Attach bucket argument to the last element of bucket stack
*/
static void _mapReallocateBucketRecur(Bucket* stack, Bucket* bucket)
{
	if (stack->next == NULL) {
		stack->next = bucket;
		bucket->next = NULL;
	}
	else
		_mapReallocateBucketRecur(stack->next, bucket);
}

/*
	Free all allocated data that is associated with map
	WARNING! Stored items are not freed by this function, you have to make sure that map is empty or you will leak memory
*/
void mapDestroy(Map* m)
{
	// mapClear(m);
	free(m->buckets);
	// pthread_mutex_destroy(&m->lock);
	free(m);
}

/*
	Deallocate all buckets and their elems
	WARNING! This not frees the data associated with items, you could easily leak memory
*/
void mapClear(Map* m)
{
	// pthread_mutex_lock(&m->lock);

	OPTIONAL_OMP_PARALLEL_FOR
	for (register int i = 0; i < m->capacity; i++)
	{
		if (m->buckets[i].next != NULL) {
			_mapClearStack(m->buckets[i].next);
			m->buckets[i].next = NULL;
		}
	}

	m->len = 0;

	if (m->capacity > MAP_INIT_CAPACITY)
	{
		free(m->buckets);
		_mapAllocateBuckets(m, MAP_INIT_CAPACITY);
	}

	// pthread_mutex_unlock(&m->lock);
}


static void _mapClearStack(Bucket* stack)
{
	// Data could be not heap allocated
	// Or could have references to another allocated data
	// free(stack->data);

	if (stack->next != NULL)
		_mapClearStack(stack->next);

	free(stack);
}


void mapPrint(Map* m)
{
	fprintf(stdout, "hashmap object at %p\n", m);
	fprintf(stdout, "len: %llu, threshold: %.2f, buckets count: %llu\n", m->len, m->threshold, m->capacity);
	fprintf(stdout, "-----------------------\n");

	for (int i = m->capacity; i--;)
	{
		if (m->buckets[i].next != NULL) {
			fprintf(stdout, "---BUCKET %d\n", i);
			_mapPrintRecur(m->buckets[i].next);
		}
	}
}


static void _mapPrintRecur(Bucket* b)
{
	fprintf(
	    stdout, "(%p) key: %llu, data at: %p, next: %p\n",
	    b,
	    b->key,
	    b->data,
	    b->next
	);

	if (b->next != NULL)
		_mapPrintRecur(b->next);
}
