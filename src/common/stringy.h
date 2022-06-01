/*
	In-house implementation of safe string manipulations
	The purpose of it is to have more sane workflow with strings
*/

// UTF32 and other representations if needed

#ifndef STRINGY_H
#define STRINGY_H

#include <wchar.h>
#include <stdbool.h>

typedef struct {
	// 'buff' is internal storage and should not be manipulated directly
	// it could be NULL if allocation wasn't successful or buffer was already freed
	wchar_t* buff;
	// 'len' is always the number of factual characters without terminating one
	size_t len;
	// 'cap' is n of allocated wchar_t elements and not n of bytes
	size_t cap;
}
str_t;

/*
	@brief 	Create new string object
			If NULL is passed then default capped string is created,
			otherwise content of string is set to passed wchar_t array
*/
str_t str_new( const wchar_t* );

/*
	@brief	str_new variant with customized initial cap
	@warn 	if cap is less or equal to len of init string
			then uninitialized string is returned
*/
str_t str_new_capped( const wchar_t*, size_t cap );

/*
	@brief	Check is given string valid
	@return If string is successfully initialized and wasn't consumed
			then true is returned, otherwise false
*/
bool str_is_valid( const str_t* );

/*
	@brief 	Free all data from string object
	@return Non-zero value on error, otherwise 0
*/
int str_free( str_t* );

/*
	@brief 	Returns pointer to new copy of string object
*/
str_t str_clone( const str_t* );

// TODO Should decode UTF-8
/*
	@brief	Create new string from file at wchar_t* supplied path
	@warn 	String isn't initialized if problem with file reading occurred,
			you should check returned struct by 'str_is_valid'
*/
str_t str_from_file( const wchar_t* );

/*
	TODO

	@brief	Replace content of string object with src string
	@return Non-zero value on error, otherwise 0
*/
int str_set( str_t*, str_t* src );

/*
	TODO

	@brief	Replace content of string object with given wchar_t string
	@return Non-zero value on error, otherwise 0
*/
int str_set_wcstr( str_t*, wchar_t* );

/*
	@brief 	Create new string object from given string,
			slicing it starting from 'l' to 'h' indexes
*/
str_t str_slice( const str_t*, size_t l, size_t h );

/*
	@brief 	Insert src string to dst at idx position
	@return Non-zero value on error, otherwise 0
*/
int str_insert( str_t* dst, const str_t* src, size_t idx );

/*
	@brief	Append src string to the end of dst
	@return Non-zero value on error, otherwise 0
*/
int str_append( str_t* dst, const str_t* src );

/*
	@brief 	Search for first encounter of supplied wchar_t string
	@return If nothing is found then negative value -2 is returned,
			otherwise it's valid index at which wchar_t string is found
*/
ssize_t str_find_wcstr( const str_t*, const wchar_t* );

/*
	@brief	Compare two strings
	@return True if given string objects are equal or False if not
*/
bool str_is_eq( const str_t*, const str_t* );

/*
	TODO
*/
int str_buff( const str_t*, wchar_t* );

/*
	@brief	Write ASCII representation of string to provided buffer
	@warn 	Buffer array shouldn't be shorter that capacity of string object
	@return Non-zero value on error, otherwise 0
			* Most common error is encounter of non-ASCII characters, error code -2
*/
int str_buff_ascii( const str_t*, char* );

/*
	TODO

	@brief	
	@warn 	This function does change string after use
			And you should not use it anymore after it was exhausted to NULL
	@return Index to 
*/
size_t str_tokenize( str_t*, str_t* token );

/*
	@brief	Print debug information about provided string object
*/
void str_dbprint( const str_t* s );


#endif
