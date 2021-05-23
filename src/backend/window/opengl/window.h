#define OPENGL_MINOR_VER 3
#define OPENGL_MAJOR_VER 4
#define DEFAULT_WINDOW_NAME "openRogue"

#define WINDOW_FILL_COLOR_R 0.2
#define WINDOW_FILL_COLOR_G 0
#define WINDOW_FILL_COLOR_B 0.062

// Event buffer is used for back-and-forth transfer when event polling
// If event does not belong to current window it should be saved in event queue for another window
#define EVENT_BUFFER_SIZE 1024

// How many events could be transfered to python with single dispatch call
// More precisely - the number describes how many Event types should be allocated
// TODO Maybe its reasonable to reallocate buffer on demand, but events are usually do not stack up that much
#define DISPATCH_BUFFER_SIZE 32

// Milliseconds in which events are considered to be valid
#define EVENT_TIMEOUT 1024

//
typedef struct {
	SDL_Window* window;
	SDL_GLContext context;
	bitmask_t id;

	uint32_t time_delta;
	uint32_t prev_timestamp;
}
WindowHandler;

// TODO Descriptions

/*
*/
WindowHandler* init_window(int width, int height, const char* title);

/*
*/
void update_window();

/*
*/
void close_window(WindowHandler*);

/*
*/
EventQueue* process_window(WindowHandler*);

/*
*/
void resize_window(WindowHandler*, int width, int height);

/*
*/
void repos_window(WindowHandler*, int x, int y);

/*
	Python code should call to free memory when it's done
*/
void _free_event_queue(EventQueue*);