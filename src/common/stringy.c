#include <stdlib.h>
#include <wchar.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>

#include "stringy.h"

#ifndef STRINGY_DEFAULT_CAP
#define STRINGY_DEFAULT_CAP 16
#endif

// initial n of allocated character for stream reading
#ifndef STRINGY_DEFAULT_STREAM_BUFFER_LEN
#define STRINGY_DEFAULT_STREAM_BUFFER_LEN 1024
#endif


// TODO Current error catching behavior is really bad
// TODO Standardized error codes


static
inline
str_t
str_constructor( const wchar_t* init_wcstr, size_t init_cap )
{
	wchar_t* buff;
	size_t len;
	size_t cap;

	if (init_wcstr == NULL) {
		len = 0;
		cap = init_cap;
		buff = (wchar_t*)malloc(cap * sizeof(wchar_t));
		buff[0] = L'\0';
	} else {
		len = wcslen(init_wcstr);
		if (len >= init_cap) {
			cap = len + 1;
		} else {
			cap = init_cap;
		}
		buff = (wchar_t*)malloc(cap * sizeof(wchar_t));
		if (buff) {
			wmemcpy(buff, init_wcstr, len + 1);
		}
	}

	str_t ret = { .buff = buff, .len = len, .cap = cap };
	return ret;
}

str_t
str_new( const wchar_t* init_wcstr )
{
	return str_constructor(init_wcstr, STRINGY_DEFAULT_CAP);
}

str_t
str_new_capped( const wchar_t* init_wcstr, size_t cap )
{
	if (cap <= wcslen(init_wcstr)) {
		str_t ret = { .buff = NULL, .len = 0, .cap = 0 };
		return ret;
	}
	return str_constructor(init_wcstr, cap);
}

str_t
str_from_file( const wchar_t* path )
{
	FILE* f;

	errno_t err = _wfopen_s(&f, path, L"r");
	if (err != 0) {
		str_t ret = { .buff = NULL, .len = 0, .cap = 0 };
		return ret;
	}

	wchar_t* buff = (wchar_t*)malloc(STRINGY_DEFAULT_STREAM_BUFFER_LEN * sizeof(wchar_t));
	size_t cap = STRINGY_DEFAULT_STREAM_BUFFER_LEN;
	size_t len = 0;

	wchar_t cur;
	do {
		cur = fgetwc(f);
		if (len == cap) {
			buff = (wchar_t*)realloc(buff, ++cap);
		}
		// TODO
		if (!(cur & 0x80)) {
			buff[len++] = cur;
		}
	} while (cur != WEOF);

	if (len == cap) {
		buff = (wchar_t*)realloc(buff, ++cap);
	}
	buff[len+1] = L'\0';

	fclose(f);

	str_t ret = { .buff = buff, .len = len-1, .cap = cap };
	return ret;
}

inline
bool
str_is_valid( const str_t* s)
{
	return s->buff != NULL;
}

str_t
str_clone( const str_t* s )
{
	wchar_t* buff = (wchar_t*)malloc(s->cap * sizeof(wchar_t));
	if (buff) {
		wmemcpy(buff, s->buff, s->len + 1);
	}
	str_t ret = { .buff = buff, .len = s->len, .cap = s->cap };
	return ret;
}

str_t
str_slice( const str_t* s, size_t l, size_t h )
{
	size_t len = h - l;

	str_t new = str_new(NULL);

	if (l >= h) {
		str_free(&new);
		return new;
	}

	if (new.cap <= len) {
		new.cap += len - new.cap + 1;
		wchar_t* allocated = (wchar_t*)realloc(new.buff, new.cap);
		if (!allocated) {
			str_free(&new);
			return new;
		} else {
			new.buff = allocated;
		}
	}

	wmemcpy(new.buff, &s->buff[l], len);
	new.len = len;
	new.buff[len] = L'\0';

	return new;
}

int
str_free( str_t* s )
{
	if (s->buff == NULL) {
		return -1;
	}
	free(s->buff);
	s->buff = NULL;
	return 0;
}

int
str_insert( str_t* dst, const str_t* src, size_t idx)
{
	if (dst->buff == NULL || src->buff == NULL) {
		return -1;
	}
	if (idx > dst->len) {
		return -2;
	}
	if (dst->len + src->len >= dst->cap ) {
		dst->cap += (dst->len + src->len) - dst->cap + 1;
		wchar_t* allocated = (wchar_t*)realloc(dst->buff, dst->cap * sizeof(wchar_t));
		if (!allocated) {
			return -3;
		} else {
			dst->buff = allocated;
		}
	}
	if (idx != dst->len) {
		wmemcpy(&dst->buff[idx + src->len], &dst->buff[idx], dst->len);
	}
	wmemcpy(&dst->buff[idx], src->buff, src->len);

	dst->len += src->len;
	dst->buff[dst->len] = L'\0';

	return 0;
}

int
str_append( str_t* dst, const str_t* src )
{
	return str_insert(dst, src, dst->len);
}

ssize_t
str_find_wcstr( const str_t* s, const wchar_t* substr )
{
	if (s->buff == NULL) {
		return -1;
	}

	wchar_t* occurance = wcsstr(s->buff, substr);
	if (!occurance) {
		return -2;
	}

	return (ssize_t)((size_t)((ptrdiff_t)occurance - (ptrdiff_t)s->buff) / sizeof(wchar_t));
}

bool
str_is_eq( const str_t* s0, const str_t* s1 )
{
	return wcscmp(s0->buff, s1->buff) == 0;
}

int
str_buff_ascii( const str_t* s, char* buff)
{
	if (s->buff == NULL) {
		return -1;
	}

	for (size_t i = 0; i < s->cap; i++) {
		if (s->buff[i] > CHAR_MAX) {
			return -2;
		}
		buff[i] = (signed char)s->buff[i];
	}

	return 0;
}

void
str_dbprint( const str_t* s )
{
	if (s->buff == NULL) {
		wprintf(L"!!! len: %llu, cap: %llu, -> #UNDEFINED !!!", s->len, s->cap);
	}
	else {
		wprintf(L"len: %llu, cap: %llu, -> \'%ls\'\n", s->len, s->cap, s->buff);
	}
}


// -------------------------------------------------- Test -- //

int main()
{
	str_t file_test = str_from_file(L"test.txt");
	if (str_is_valid(&file_test)) {
		str_dbprint(&file_test);
	} else {
		printf("error on file load\n");
	}

// 	str_t test = str_new(L"хах! test me!");
// 	str_dbprint(&test);

// 	// str_t clone_test = str_clone(&test);
// 	// wprintf(L"clone: %ls\n", clone_test.buff);

// 	str_t ins = str_new(L"WHAT?");
// 	str_insert(&test, &ins, 0);
// 	str_append(&test, &ins);
// 	str_dbprint(&test);

// 	str_t slice = str_slice(&test, 0, 5);
// 	str_dbprint(&slice);

// 	size_t idx = str_find_wcstr(&test, L"test");
// 	wprintf(L"'test' search result: %d\n", idx);

// 	wprintf(L"equal test: %d\n", str_is_eq(&slice, &ins));

// 	char buff[128];
// 	str_buff_ascii(&test, buff);
// 	printf("%s\n", buff);
}
