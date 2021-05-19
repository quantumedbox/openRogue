#define OPENGL_MINOR_VER 30
#define OPENGL_MAJOR_VER 46
#define DEFAULT_WINDOW_NAME "openRogue"

#define EVENT_BUFFER_SIZE 1024

// Used for indexing window array
typedef ssize_t WindowID;

#define WindowID_Err -1

typedef struct {
	SDL_Window* win;
}
VirtualHolder;


/*
*/
// VirtualHolder init_window(int width, int height, const char* title);

/*
*/
// Status_T process_window(SDL_Window*);

/*
	Returns Python list of SDL events
*/
// PyObject* get_window_events(SDL_Window*);
