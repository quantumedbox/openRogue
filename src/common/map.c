#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "map.h"


#ifdef USE_OMP
#define OPTIONAL_OMP_PARALLEL_FOR #pragma omp parallel for
#else
#define OPTIONAL_OMP_PARALLEL_FOR //
#endif


// Stack-like structure that holds a key and void pointer to data
struct Bucket
{
	key_t			key;

	void* 			data;

	struct Bucket* 	next;
}
Bucket;


// ----------------------------------------------------------------- Hash functions -- //

/*
	Returns integer value that represents a value of given null terminated string
*/
key_t
hash_string (const char* s)
{
	const int p = 97;
	const int m = 1e9 + 9;
	key_t hash = 0;
	key_t p_pow = 1;

	unsigned int s_len = strlen(s);
	for (const char *ps = s; ps - s < s_len; ++ps) {
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
Map*
mapNew()
{
	Map* new = (Map*)malloc(sizeof(Map));

	mapAllocateBuckets(new, MAP_INIT_CAPACITY);

	// pthread_mutex_init(&new->lock, NULL);

	new->threshold 	= MAP_INIT_THRESHOLD;
	new->len 		= 0;

	return new;
}


static inline
struct
Bucket* mapNewBucket()
{
	struct Bucket* new = (struct Bucket*)calloc(1, sizeof(struct Bucket));

	return new;
}


static inline
void
mapAllocateBuckets(Map* m, size_t n)
{
	m->buckets = (struct Bucket*)calloc(n, sizeof(struct Bucket));
	m->capacity = n;
}

/*
	Add new elem under the key with reference to data
	WARNING! If given key is already in the map then the data will be replaced.
	If previous data was the reference to a heap-allocated segment - it will leak
*/
void
mapAdd(Map* m, key_t key, data_t data)
{
	// pthread_mutex_lock(&m->lock);

	key_t idx = key % m->capacity;

	// if (m->buckets[idx].next == NULL)
	// {
	// 	m->len += 1;

	// 	Bucket* new = mapNewBucket();
	// 	m->buckets[idx].next = new;
	// 	new->key 	= key;
	// 	new->data 	= data;
	// }
	// else if (mapAddRecur(m->buckets[idx].next, key, data))
		// m->len += 1;

	struct Bucket* b = &m->buckets[idx];

	while(true)
	{
		if (b->next == NULL)
		{
			struct Bucket* new = mapNewBucket();
			new->key 	= key;
			new->data 	= data;
			b->next 	= new;

			m->len++;

			if ((m->len < MAP_MAX_CAPACITY) && (((float)m->len / m->capacity) >= m->threshold))
				mapExtend(m);
			
			return;
		}

		if (b->next->key == key)
		{
			b->next->data = data;
			return;
		}

		b = b->next;
	}

	// pthread_mutex_unlock(&m->lock);
}

/*
	Returns true if a new bucket was added to the stack
	Returns false if there was an item under the given key already and it was replaced
	WARNING! If replaced data was heap-allocated it leaks memory
*/
// static inline
// bool
// mapAddRecur(Bucket* b, key_t key, data_t data)
// {
// 	if (b->key == key) {
// 		b->data = data;

// 		return false;
// 	}

// 	else if (b->next == NULL)
// 	{
// 		Bucket* new = mapNewBucket();
// 		b->next 	= new;
// 		new->key 	= key;
// 		new->data 	= data;

// 		return true;
// 	}

// 	return mapAddRecur(b->next, key, data);
// }

/*
	Returns true if given key is present in the map, false if not
*/
bool
mapHas(Map* m, key_t key)
{
	key_t idx = key % m->capacity;

	struct Bucket* b = m->buckets[idx].next;

	while(b != NULL)
	{
		if (b->key == key)
			return true;

		b = b->next;
	}

	return false;
}


// static inline
// bool
// mapHasKeyRecur(Bucket* b, key_t key)
// {
// 	do
// 	{
// 		if (b->key == key)
// 			return true;

// 		b = b->next;
// 	}
// 	while(b != NULL);

// 	return false;
// }

/*
	Returns the void* data that is under given key or NULL if nothing to find
*/
data_t
mapGet(Map* m, key_t key)
{
	key_t idx = key % m->capacity;

	struct Bucket* base = &m->buckets[idx];
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
void
mapDel(Map* m, key_t key)
{
	key_t idx = key % m->capacity;

	struct Bucket* base = &m->buckets[idx];
	while (base->next != NULL)
	{
		if (base->next->key == key)
		{
			struct Bucket* to_free = base->next;
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
static
void
mapExtend(Map* m)
{
	size_t old_capacity = m->capacity;

	m->capacity = pow(2, log2(m->capacity) + 1);

	struct Bucket* new_array = (struct Bucket*)calloc(m->capacity, sizeof(struct Bucket));

	// It definitely could raise race condition as all threads are adding to the same bucket array
	// OPTIONAL_OMP_PARALLEL_FOR
	for (register size_t i = 0; i < old_capacity; i++)
	{
		if (m->buckets[i].next != NULL)
			mapReallocateBucketStack(new_array, m->buckets[i].next, m->capacity);
	}

	free(m->buckets);
	m->buckets = new_array;
}

/*
	Recursevly distribute all elements of bucket stack to the buckets array
*/
static
void
mapReallocateBucketStack(struct Bucket* buckets, struct Bucket* stack, size_t cap)
{
	key_t idx = stack->key % cap;

	struct Bucket* stack_next = stack->next;

	if (buckets[idx].next == NULL) {
		buckets[idx].next = stack;
		stack->next = NULL;
	}
	else
		mapReallocateBucketRecur(buckets[idx].next, stack);

	if (stack_next != NULL)
		mapReallocateBucketStack(buckets, stack_next, cap);
}

/*
	Attach bucket argument to the last element of bucket stack
*/
static
void
mapReallocateBucketRecur(struct Bucket* stack, struct Bucket* bucket)
{
	if (stack->next == NULL) {
		stack->next = bucket;
		bucket->next = NULL;
	}
	else
		mapReallocateBucketRecur(stack->next, bucket);
}

/*
	Free all allocated data that is associated with map
	WARNING! Stored items are not freed by this function, you have to make sure that map is empty or you will leak memory
*/
void
mapDestroy(Map* m)
{
	mapClear(m);
	free(m->buckets);
	// pthread_mutex_destroy(&m->lock);
	free(m);
}

/*
	Deallocate all buckets and their elems
	WARNING! This not frees the data associated with items, you could easily leak memory
*/
void
mapClear(Map* m)
{
	// pthread_mutex_lock(&m->lock);

	OPTIONAL_OMP_PARALLEL_FOR
	for (register size_t i = 0; i < m->capacity; i++)
	{
		if (m->buckets[i].next != NULL) {
			mapClearStack(m->buckets[i].next);
			m->buckets[i].next = NULL;
		}
	}

	m->len = 0;

	if (m->capacity > MAP_INIT_CAPACITY)
	{
		free(m->buckets);
		mapAllocateBuckets(m, MAP_INIT_CAPACITY);
	}

	// pthread_mutex_unlock(&m->lock);
}


static
void
mapClearStack(struct Bucket* stack)
{
	// Data could be not heap allocated
	// Or could have references to another allocated data
	// free(stack->data);

	if (stack->next != NULL)
		mapClearStack(stack->next);

	free(stack);
}


void
mapPrint(Map* m)
{
	fprintf(stdout, "hashmap object at %p\n", m);
	fprintf(stdout, "len: %llu, threshold: %.2f, buckets count: %llu\n", m->len, m->threshold, m->capacity);
	fprintf(stdout, "-----------------------\n");

	for (int i = m->capacity; i--;)
	{
		if (m->buckets[i].next != NULL) {
			fprintf(stdout, "-- BUCKET %d\n", i);
			mapPrintRecur(m->buckets[i].next);
		}
	}
}


static
void
mapPrintRecur(struct Bucket* b)
{
	fprintf(
	    stdout, "(%p) key: %llu, data at: %p, next: %p\n",
	    b,
	    b->key,
	    b->data,
	    b->next
	);

	if (b->next != NULL)
		mapPrintRecur(b->next);
}


// TODO Check for memory leakage
/*
	Compile this file as executable for testing
*/
// int main(void)
// {
// 	Map* map = mapNew();

// 	#define TEST_VALUE 98765

// 	int* test = (int*)malloc(sizeof(int));
// 	*test = TEST_VALUE;

// 	// Basic functionality test
// 	mapAdd(map, 'A', test);
// 	if (!mapHas(map, 'A')) {
// 		fprintf(stderr, "mapHas -- failure\n");
// 	} else {
// 		fprintf(stderr, "mapHas -- check\n");
// 	}

// 	if (TEST_VALUE != *(int*)mapGet(map, 'A')) {
// 		fprintf(stderr, "mapGet -- failure\n");
// 	} else {
// 		fprintf(stderr, "mapGet -- check\n");
// 	}

// 	mapAdd(map, 'A', NULL);
// 	if (mapGet(map, 'A') != NULL) {
// 		fprintf(stderr, "mapGet after readding -- failure\n");
// 	} else {
// 		fprintf(stderr, "mapGet after readding -- check\n");
// 	}

// 	mapClear(map);
// 	free(test);

// 	// Test by adding a vast array of items
// 	#define N_TEST_VALUES 1024

// 	int test_array[N_TEST_VALUES];

// 	for (int i = 0; i < N_TEST_VALUES; i++)
// 	{
// 		test_array[i] = rand();
// 		mapAdd(map, i, &test_array[i]);
// 	}

// 	for (int i = 0; i < N_TEST_VALUES; i++)
// 	{
// 		if (!(int*)mapGet(map, i)) {
// 			fprintf(stderr, "mapGet on a vast set -- failure\n");
// 			goto N_TEST_FAILURE;
// 		} else {
// 			mapDel(map, i);
// 			if ((int*)mapGet(map, i)) {
// 				fprintf(stderr, "mapDel on a vast set -- failure\n");
// 				goto N_TEST_FAILURE;
// 			}
// 		}
// 	}
// 	fprintf(stderr, "mapGet on a vast set -- check\n");
// 	fprintf(stderr, "mapDel on a vast set -- check\n");

// N_TEST_FAILURE:

// 	mapDestroy(map);

// 	fprintf(stdout, "Test finished\n");
// }
