#include <stdbool.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <GL/glew.h>

#include "text.h"
#include "window/opengl/render_program.h"
#include "map.h"
#include "error.h"

// Textures: internalFormat = GL_ALPHA, type = GL_BITMAP
// This way it's packed to 1px per 1 bit

// QUESTION: From what should we set the size? Width or height? Or both?

// Problem with that method is that a single language could be divided into several textures which is not ideal

// It actually may be really expensive to calculate character ranges for small font sizes
// Literal thousands of glyph should be buffered for that

// We should limits the maximum amount of glyphs per texture
// The most rational way is to limit them to widely supported texture size of 1024px

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

static GLuint text_render_program;


// -------------------------------------------------------------- Runtime constants -- //


// Runtime const that is used for packing glyphs into the texture atlases
static GLint MAX_TEXTURE_SIZE;

// Client side buffer of zeroes, used on texture initialization
static GLuint PX_BUFFER;


// ------------------------------------------------------------------------ Helpers -- //


#ifndef DEBUG
void
check_opengl_state(char* description)
{
	GLenum err;
	if ((err = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "OPENGL ERROR :: CODE 0x%x :: %s\n", err, description);
	}
}
#endif

#ifdef DEBUG
// Turn off checking as it's not necessary with verbose callback
#define check_opengl_state(_) //


void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
	fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	         ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
	         type, severity, message );
}
#endif


// -------------------------------------------------------------------- Realization -- //


// What about making it inlined and check if it was initialized at the beginning?
/*
	@brief 	Initialize everything that is needed for text functionality
	@warn 	OpenGL should be already initialized before this call
	@return Returns non-zero value on error
*/
int
init_text_subsystem ()
{
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not initialize freetype\n");
		SIGNAL_ERROR();
		return -1;
	}

	// Buffering of texture that is filled with zeros for initializing new range textures
	{
		// Get hardware specific consts
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MAX_TEXTURE_SIZE);
		fprintf(stdout, "-- max texture dimension: %dpx\n", MAX_TEXTURE_SIZE);

		// Prepare zero init buffer
		glGenBuffers(1, &PX_BUFFER);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PX_BUFFER);

		glBufferData(GL_PIXEL_UNPACK_BUFFER, FONT_TEXTURE_SIZE * FONT_TEXTURE_SIZE, NULL, GL_STATIC_DRAW);
		check_opengl_state("Error on zero init data buffering\n");

		GLbyte* px_data = (GLbyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		if (!px_data) {
			fprintf(stderr, "Could not allocate zero init pixel unpack buffer\n");
			return -1;
		}
		memset(px_data, 0x00, FONT_TEXTURE_SIZE * FONT_TEXTURE_SIZE / sizeof(GLbyte));
		if (!glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER))
			fprintf(stderr, "Could not unmap zero init pixel unpack buffer\n");

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	text_render_program = new_render_program("resources/shaders/text_vert.vs", "resources/shaders/text_frag.fs");

	fm.fonts = mapNew();

	#ifdef DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	#endif

	is_text_subsystem_initialized = true;

	return 0;
}


/*
	@brief 	Returns reference to a newly heap allocated FontSize
			It has a refcount of 1 on creation
*/
static
inline
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
		fclose(check);
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

	@warn 	API caller should listen to ERRORCODE value for panics!
*/
static
Font*
new_font (const char* path, uint32_t size)
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

	if (FT_Set_Pixel_Sizes(*face, 0, size) != 0)
	{
		fprintf(stderr, "Cannot set the font size of font at %s\n", suitable);
		SIGNAL_ERROR();
		free(face);
		return NULL;
	}

	Font* new = (Font*)malloc(sizeof(Font));

	new->type = FONT_SOURCE_FREETYPE;
	new->source = face;
	new->sizes = mapNew();

	fprintf(stdout, "Created new font: %s\n", suitable);

	return new;
}


/*
	@brief 	Check if given font is loaded and if not, - return its hash
			This function increments refcounts on repeating references of the same font size

	@warn 	API caller should listen to ERRORCODE value for panics!

	@param 	path -- hint to a font file
	@param 	size -- max height of a glyph, width is considered to be the same

	@return Key identification for a font
*/
size_t
resolve_font (const char* path, uint32_t size)
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
		mapAdd(fm.fonts, path_hash, font_n);
		mapAdd(font_n->sizes, size, new_font_size());
	}

	return path_hash;
}

/*
	@brief 	Buffers a utf-32 string to a framebuffer for future drawing

	@warn 	Bytes should be encoded in 'utf-32-le'
	@warn 	API caller should listen to ERRORCODE value for panics!
	@warn 	Manual freeing by free_buffer_strip() is required
*/
void
new_buffer_strip (size_t font_hash,
                  uint32_t size,
                  int32_t x_offset,
                  int32_t y_offset,
                  uint32_t* utf_string,
                  uint32_t string_len)
{
	fprintf(stdout, "strip len: %d\n", string_len);

	Font* font = mapGet(fm.fonts, font_hash);

	// Should we really double-check ?
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

	if (string_len == 0) {
		// TODO Case of zero len string
	}

	// Size is actually width and height but for now they're the same
	// Int range_size = (MAX_TEXTURE_SIZE / size) * (MAX_TEXTURE_SIZE / size);
	int range_size = (FONT_TEXTURE_SIZE / size) * (FONT_TEXTURE_SIZE / size);

	// TODO Maybe we should directly operate on GL vertex buffer?
	// Buffer of texture coordinates
	float buffer[STRIP_BUFFER_SIZE * 2];
	size_t buffer_len = 0;

	// Negative values of range should signal that none of the possible ranges is binded
	int current_range = -1;

	while (string_len > 0)
	{
		fprintf(stdout, "Current char: %d\n", *utf_string);
		// Encountered a symbol that is not in currently binded texture
		if (*utf_string / range_size != current_range)
		{
			// Draw everything that was buffered (if buffered at all)
			if (buffer_len > 0)
			{
				fprintf(stdout, "Buffer draw!\n");

				// TODO Draw

				// Clear buffer
				buffer_len = 0;
			}

			FontRange* range = mapGet(font_s->ranges, *utf_string / range_size);

			// If there's no such range - create a new one
			if (range == NULL)
			{
				fprintf(stdout, "Created new range: %d\n", *utf_string / range_size);

				GLuint texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

				// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, float{0.0f, 0.0f, 0.0f, 0.0f});

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				// Fill newly created texture with zeros from PX_BUFFER
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PX_BUFFER);
				// TODO Check if we actually should divide on 8
				// What about compression?
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_TEXTURE_SIZE / 8, FONT_TEXTURE_SIZE / 8, 0, GL_COLOR_INDEX, GL_BITMAP, NULL);
				check_opengl_state("Creation of texture buffer");
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

				// Write glyphs to glyps_data and then set subtexture to it
				for (int i = 0; i < range_size; i++)
				{
					int err = FT_Load_Glyph(
					              *(FT_Face*)font->source,
					              FT_Get_Char_Index(*(FT_Face*)font->source, *utf_string),
					              FT_LOAD_MONOCHROME);
					// If there's a problem with loading a glyph (for example when it is not present), - just skip it
					// TODO Maybe we should have some generic symbol that says that given character is not present in the font?
					if (err != 0) {
						continue;
					}

					FT_Glyph glyph;
					if (FT_Get_Glyph((*(FT_Face*)font->source)->glyph, &glyph) != 0) {
						fprintf(stderr, "Cannot get glyph\n");
						continue;
					}

					if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_MONO, 0, true) != 0) {
						fprintf(stderr, "Cannot transform glyph to a bitmap\n");
						FT_Done_Glyph(glyph);
						continue;
					}
					FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

					glTexSubImage2D(
						GL_TEXTURE_2D, 0,
						(i % (FONT_TEXTURE_SIZE / size))*size / 8,
						(i / (FONT_TEXTURE_SIZE / size))*size / 8,
						bitmap_glyph->bitmap.width / 8,
						bitmap_glyph->bitmap.rows / 8,
						GL_COLOR_INDEX,
						GL_BITMAP,
						bitmap_glyph->bitmap.buffer);

					FT_Done_Glyph(glyph);
				}

				float test_buff[] = {
					-0.5, -0.5, 0.0,	0.0, 0.0,
					+0.5, -0.5, 0.0,	1.0, 0.0,
					+0.5, +0.5, 0.0,	1.0, 1.0,
					-0.5, -0.5, 0.0,	0.0, 0.0,
					+0.5, +0.5, 0.0,	1.0, 1.0,
					-0.5, +0.5,	0.0,	0.0, 1.0,
				};

				// Create and store a new font range in a map
				FontRange* range_new = (FontRange*)malloc(sizeof(FontRange));
				range_new->texture = texture;
				range_new->width = size;
				range_new->height = size;

				mapAdd(font_s->ranges, *utf_string / range_size, range_new);
			}

			// TODO Bind the texture to render program
			current_range = *utf_string / range_size;
		}

		// TODO Buffer texture coords of a new glyph

		buffer_len++;
		string_len--;

		if (string_len == 0)
		{
			// TODO Draw
		}
		else
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
