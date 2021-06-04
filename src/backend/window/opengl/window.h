#define OPENGL_MINOR_VER 3
#define OPENGL_MAJOR_VER 4

#define DEFAULT_WINDOW_NAME "openRogue"

#define WINDOW_FILL_COLOR_R 0.2
#define WINDOW_FILL_COLOR_G 0
#define WINDOW_FILL_COLOR_B 0.062

// Size of staticly allocated event queue buffer
#define EVENT_BUFFER_SIZE 16


// TODO
// Idea is that APIs could give the information about their functionalities
// The fact that it is done via strings gives it the ability to have non-standard features without modifying headers or engines
const char* FEATURE_LIST[] = {
	"shaders",
	"...",
};

// TODO It is kinda bad to return char**, maybe we should do something else?
const char** get_feature_list ()
{
	return FEATURE_LIST;
}


/*
*/
typedef struct {
	SDL_Window* window;
	// SDL_GLContext context; // Now GL context is global for all windows of single thread
	window_id_t id;

	// Switching queues
	// At given time only one of them should be writable and another - readable
	int current_queue;
	EventQueue* queue0;
	EventQueue* queue1;

	// Updated on start_drawing()
	int width;
	int height;

	uint32_t time_delta;
	uint32_t prev_timestamp;
}
WindowHandler;

// TODO Descriptions

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
*/
void start_drawing(window_id_t);

/*
*/
void finish_drawing();

/*
	Python code should call to free memory when it's done
*/
// void free_event_queue(EventQueue*);
