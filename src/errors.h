#include <stdio.h>

// TODO

// error call implementation

#define NEW_ERROR_TYPE(errtype, errmessage)	const char* _ERROR_##errtype = errmessage
#define GET_ERROR_MSG(errtype) _ERROR_##errtype

#define STATUS int8_t
#define OK_STATUS 0
#define ERR_STATUS -1

void _printerr(const char* msg) {
	fprintf(stderr, "%s\n", msg);
}

#define PUSH_ERROR(errtype) _printerr(GET_ERROR_MSG(errtype))
#define PUSH_ERRORM(errmsg) _printerr(errmsg)

#define NULL_CHECK(ref, errtype) if(!ref) PY_ERROR(errtype)
#define ERR_CHECK(ref, errtype) if(ref==-1) PY_ERROR(errtype)

// NEW_ERROR_TYPE(SUCCESS, "executed successfuly");
NEW_ERROR_TYPE(_, "execution aborted");

NEW_ERROR_TYPE(SDL_CANNOT_CREATE_CONTEXT, "Cannot create SDL context");
// NEW_ERROR_TYPE(GLEW_INIT_ERR, 		"Cannot initialize OpenGL");
// NEW_ERROR_TYPE(GLFW_INIT_ERR, 		"Cannot initialize GLFW");
// NEW_ERROR_TYPE(WINDOW_CREATION_ERR, 	"Error on window context creation");
