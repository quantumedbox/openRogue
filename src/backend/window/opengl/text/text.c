#include <stdbool.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glew.h>

#include "text.h"
#include "window/opengl/render_program.h"
#include "map.h"
#include "error.h"


// QUESTION: From what should we set the size? Width or height? Or both?

// TODO Refcounts for usages of fonts, their ranges and sized of those ranges?

// N of characters per page should be dissected from face metrics and maximum texture size
// What about using texture arrays for packing more characters in a single texture?
// Form a buffer of multiple characters if part of string is in the same character range, we should reduce the amount of texture switching and draw calls

// -------------------------------------------------------------------- Definitions -- //

/*
	Stores and manages loaded fonts
*/
typedef struct
{
	Map* fonts;
}
FontManager;


typedef enum {
	// Indicates that source file is freetype font
	FONT_SOURCE_FREETYPE,
	// Indicates that source file is image // TODO
	FONT_SOURCE_BITMAP,
}
FontSourceType;

/*
	Loaded font of generic type
*/
typedef struct
{
	FontSourceType type;
	void* source;
	// size_t refcount; // lifetime of font is dependent of 'sizes' map len. If there's no sizes - there's no fonts in use
	Map* sizes;
}
Font;

/*
	Stores refcounts to a given font size and map to buffered char ranges
*/
typedef struct
{
	size_t refcount;
	Map* ranges;
}
FontSize;

/*
	Buffered packed character range
*/
typedef struct
{
	GLuint texture;
	int width;
	int height;
}
FontRange;


// ----------------------------------------------------------------- Global objects -- //


static bool is_text_subsystem_initialized = false;

static FT_Library ft;

static FontManager fm;

// Runtime const that is used for packing glyphs into the texture atlases
static GLint MAX_TEXTURE_SIZE = 0;


// -------------------------------------------------------------------- Realization -- //


// What about making it inlined and check if it was initialized at the beginning?
int
init_text_subsystem ()
{
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not initialize freetype\n");
		SIGNAL_ERROR();
		return -1;
	}

	fm.fonts = mapNew();

	// Get hardware specific consts
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MAX_TEXTURE_SIZE);
	printf("-- max texture size: %dpx\n", MAX_TEXTURE_SIZE);

	is_text_subsystem_initialized = true;

	return 0;
}


/*
	Returns reference to a newly heap allocated FontSize
	It has a refcount of 1 on creation
*/
__forceinline
FontSize*
new_font_size ()
{
	FontSize* new = (FontSize*)malloc(sizeof(FontSize));
	new->refcount = 1;
	new->ranges = mapNew();

	return new;
}


/*
	@brief	If desired path is not valid then default font path is used
			If default font isn't valid - returns NULL to signal critical error
*/
static
const char*
find_suitable_font (const char* desired)
{
	// check if given file exists
	FILE* check;
	if ((check = fopen(desired, "r")) == NULL)
	{
		// if not - try to open default font
		FILE* dflt;
		if ((dflt = fopen(DEFAULT_FONT, "r")) == NULL)
		{
			fprintf(stderr, "Cannot open desired font nor default one\n");
			SIGNAL_ERROR();

			fclose(check);
			fclose(dflt);
			return NULL;
		}
		fclose(dflt);
		return DEFAULT_FONT;
	}
	fclose(check);
	return desired;
}


// TODO Bitmap fonts
/*
	@brief 	Creates new font from file at the path argument
			Returns NULL on critical error
*/
__forceinline
Font*
new_font (const char* path, int size)
{
	const char* suitable = find_suitable_font(path);
	if (suitable == NULL)
		return NULL;

	FT_Face* face = (FT_Face*)malloc(sizeof(FT_Face));

	if (FT_New_Face(ft, suitable, 0, face) != 0)
	{
		fprintf(stderr, "Cannot load font at %s\n", suitable);
		SIGNAL_ERROR();
		free(face);
		return NULL;
	}

	FT_Set_Pixel_sizes(face, 0, size);

	Font* new = (Font*)malloc(sizeof(Font));

	new->type = FONT_SOURCE_FREETYPE;
	new->source = face;
	new->sizes = mapNew();

	return new;
}


/*
	@brief 	Check if given font is loaded and if not, - return its hash
			This function increments refcounts on repeating references of the same font size

	@warn 	API caller should listen to ERRORCODE value for panics!

	@param 	path -- hint to a font file
	@param 	size -- max height of a glyph, width is considered to be the same
*/
size_t
resolve_font (const char* path, int size)
{
	size_t path_hash = hash_string(path);

	Font* font = mapGet(fm.fonts, path_hash);
	if (font != NULL)
	{
		FontSize* font_s = mapGet(font->sizes, size);
		if (font_s == NULL) {
			mapAdd(font->sizes, size, new_font_size());
		} else {
			font_s->refcount++;
		}
	} else {
		Font* font_n = new_font(path, size);
		if (font_n == NULL)
			return 0;
		mapAdd(font_n->sizes, size, new_font_size());
	}

	return path_hash;
}

/*
	@brief 	Buffers a utf-32 string to a framebuffer for future drawing

	@warn 	API caller should listen to ERRORCODE value for panics!
	@warn 	Manual freeing by free_buffer_strip() is required
*/
void
new_buffer_strip (size_t font_hash,
                  int size,
                  int x_offset,
                  int y_offset,
                  uint32_t utf_string,
                  size_t string_len)
{
	Font* font = mapGet(fm.fonts, font_hash);

	if (font == NULL) {
		fprintf(stderr, "Cannot create buffer texture from unresolved font\n");
		SIGNAL_ERROR();
		return;
	}

	FontSize* font_s = mapGet(font->sizes, size);

	if (font_s == NULL) {
		fprintf(stderr, "Cannot create buffer texture from unresolved font size\n");
		SIGNAL_ERROR();
		return;
	}

	// TODO Case of zero len string
	if (string_len == 0) {

	}

	// size is actually width and height but for now they're the same
	int size_range = (MAX_TEXTURE_SIZE / size) * (MAX_TEXTURE_SIZE / size);

	float buffer[STRIP_BUFFER_SIZE * 2];
	size_t buffer_len = 0;
	int current_range = *utf_string % size_range;

	while (string_len > 0)
	{
		// Change of range, draw everything that was buffered and bind a new texture 
		if (*utf_string != current_range)
		{
			if (buffer_len > 0)
			{
				// TODO Draw
			}

			buffer_len = 0;

			// TODO Check if range was created already and if not, - make a new range

			FontRange* range = mapGet(font_s->ranges, size_range);
		}

		// TODO Buffer texture coords of a new glyph

		utf_string++;
	}
}

// int load_font(const char* fontpath, int size)
// {
// 	// if (!_is_text_subsystem_initialized) {
// 	// 	if (init_text() == -1) {
// 	// 		return -1;
// 	// 	}
// 	// }

// 	FT_Face face;
// 	if (FT_New_Face(ft, fontpath, 0, &face)) {
// 		printf("Could not load font %s\n", fontpath);
// 		return -1;
// 	}

// 	// Should we only allow monospace fonts?
// 	// if (!FT_IS_FIXED_WIDTH(face)) {
// 	// }

// 	FT_Set_Pixel_sizes(face, 0, size);

// 	GLuint program = newRenderProgram("shaders/text_vert.vs", "shaders/text_frag.fs");

// 	Gluint texture;
// 	glActiveTexture(GL_TEXTURE0);
// 	glGenTextures(1, &texture);
// 	glBindTexture(GL_TEXTURE_2D, texture);
// 	glUniform1i(glGetUniformLocation(program, "texture"), 0);

// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// 	glPixelStorei(GL_UNPACK_ALIGMENT, 1);

// 	GLuint vbo;
// 	glGenVuffers(1, &vbo);
// 	glEnableVertexAttribArray(0);
// 	glBindBuffer(GL_ARRAY_BUFFER, vbo);
// 	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

// 	// glDisableVertexAttribArray(0);
// }
