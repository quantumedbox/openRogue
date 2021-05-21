#define OPENGL_MINOR_VER 30
#define OPENGL_MAJOR_VER 46
#define DEFAULT_WINDOW_NAME "openRogue"

// Event buffer is used for back-and-forth transfer when event polling
// If event does not belong to current window it should be saved in event queue for another window
#define EVENT_BUFFER_SIZE 1024

// How many events could be transfered to python with single dispatch call
// More precisely - the number describes how many Event types should be allocated
// TODO Maybe its reasonable to reallocate buffer on demand, but events are usually do not stack up that much
#define DISPATCH_BUFFER_SIZE 32

// Milliseconds in which events are considered to be valid
#define EVENT_TIMEOUT 1024


// TODO Descriptions

// TODO process_window and dispatch_window_events should be united

/*
*/
SDL_Window* init_window(int width, int height, const char* title);

/*
*/
void update_window();

/*
*/
void close_window(SDL_Window*);

/*
*/
bitmask_t process_window(SDL_Window*);

/*
*/
EventQueue* dispatch_window_events(SDL_Window*);


/*
	Python code should call to free memory when it's done
*/
void _free_event_queue(EventQueue*);
