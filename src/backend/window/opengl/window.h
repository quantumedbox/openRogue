#define OPENGL_MINOR_VER 3
#define OPENGL_MAJOR_VER 4

#define DEFAULT_WINDOW_NAME "openRogue"

#define WINDOW_FILL_COLOR_R 0.2
#define WINDOW_FILL_COLOR_G 0
#define WINDOW_FILL_COLOR_B 0.062

// Size of staticly allocated event queue buffer
#define EVENT_BUFFER_SIZE 1

// Milliseconds in which events are considered to be valid
#define EVENT_TIMEOUT 1024

typedef uint32_t window_id_t;

//
typedef struct {
	SDL_Window* window;
	SDL_GLContext context;
	window_id_t id;

	EventQueue* queue;

	uint32_t time_delta;
	uint32_t prev_timestamp;
}
WindowHandler;

// TODO Descriptions

/*
*/
int event_queue_former(void*, SDL_Event*);

/*
*/
window_id_t init_window(int width, int height, const char* title);

/*
*/
void update_window();

/*
*/
void close_window(window_id_t);

/*
*/
EventQueue* process_window(window_id_t);

/*
*/
void resize_window(window_id_t, int width, int height);

/*
*/
void repos_window(window_id_t, int x, int y);

/*
	Python code should call to free memory when it's done
*/
void free_event_queue(EventQueue*);
