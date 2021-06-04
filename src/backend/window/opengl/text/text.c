#include <stdbool.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <GL/glew.h>

#include "text.h"
#include "window/opengl/render_program.h"
#include "map.h"
#include "error.h"

// TODO Garbage collector-like that keeps track of usages per each texture in a certain time window

// TODO We don't need buffered textures that much. Maybe in the future if there would be such need

// NOPE!
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

/*
*/
typedef enum {
	// Indicates that source file is freetype font
	FONT_SOURCE_FREETYPE,
	// Indicates that source file is image // TODO
	FONT_SOURCE_BITMAP,
}
FontSourceType;

/*
	Loaded font of generic type

	TODO Should it be freed if there's no usages of it ?
*/
typedef struct
{
	FontSourceType type;
	void* source;
	Map* sizes;
}
Font;

// ??? Should be self-aware about its size ???
/*
	Stores refcounts to a given font size and map to buffered char ranges
*/
typedef struct
{
	// signals for GC the fact of recent usage
	bool was_used;
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

/*
	Holds associated with strip buffer data for drawing and freeing purposes
*/
// typedef struct
// {
// 	GLuint vao;
// 	GLuint vbo;
// 	GLuint texture;
// }
// StripBuffer;


// ----------------------------------------------------------------- Global objects -- //


static bool is_text_subsystem_initialized = false;

static FT_Library ft;

static FontManager fm;

static GLuint text_render_program;

// static GLuint text_render_vao;

// static GLuint text_render_vbo;


extern WindowHandler* current_drawing_window;

extern mat4 current_window_projection;


// -------------------------------------------------------------- Runtime constants -- //


// TODO Maybe give the ability to API caller to set size of textures ?

// Runtime const that is used for packing glyphs into the texture atlases
static GLint MAX_TEXTURE_SIZE;

// Client side buffer of zeroes, used on texture initialization
// static GLuint PX_BUFFER;


// ------------------------------------------------------------------------ Helpers -- //


#ifndef DEBUG
void
check_opengl_state( char* description )
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
init_text_subsystem()
{
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not initialize freetype\n");
		SIGNAL_ERROR();
		return -1;
	}

	text_render_program = new_render_program("resources/shaders/text.vert", "resources/shaders/text.frag");

	fm.fonts = mapNew();
	// strip_buffer_map = mapNew();

	#ifdef DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	#endif

	// glGenVertexArrays(1, &text_render_vao);
	// glBindVertexArray(text_render_vao);

	// glEnableVertexAttribArray(0);

	// glGenBuffers(1, &text_render_vbo);
	// glBindBuffer(GL_ARRAY_BUFFER, text_render_vbo);
	// glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	// glBindBuffer(GL_ARRAY_BUFFER, 0);

	// glBindVertexArray(0);

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
new_font_size()
{
	FontSize* new = (FontSize*)malloc(sizeof(FontSize));
	new->was_used = false;
	new->ranges = mapNew();

	return new;
}


/*
	@brief	If desired path is not valid then default font path is used
			If default font isn't valid - returns NULL to signal critical error
*/
static
const char*
find_suitable_font( const char* desired )
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
new_font( const char* path )
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

	Font* new = (Font*)malloc(sizeof(Font));

	new->type = FONT_SOURCE_FREETYPE;
	new->source = face;
	new->sizes = mapNew();

	fprintf(stdout, "Created new font from %s\n", suitable);

	return new;
}


/*
	@brief 	Returns the valid font hash
			Makes sure that given font at path is loaded

	@warn 	API caller should listen to ERRORCODE value for panics!

	@param 	path -- hint to a font file

	@return Key identification for a font
*/
size_t
resolve_font( const char* path )
{
	size_t path_hash = hash_string(path);

	Font* font = mapGet(fm.fonts, path_hash);
	if (font == NULL)
	{
		Font* font_n = new_font(path);
		if (font_n == NULL)
			return 0;
		mapAdd(fm.fonts, path_hash, font_n);
	}

	return path_hash;
}


// TODO Do it in a proper way
static
void
draw_text_buffer( float* buffer, size_t buffer_len )
{
	GLuint vao, vbo;

	// Generating them over and over is stupid as fuck
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, buffer_len * sizeof(float) * 6 * 4, buffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)(sizeof(GLint) * 2));

	glDrawArrays(GL_TRIANGLES, 0, buffer_len * 6);

	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);
}



/*
	@brief 	Renders given string to the current drawing window with specified font at the offset position

	@warn 	Bytes should be encoded in 'utf-32-le'

	@param 	font_hash	-- Represents a loaded font from resolve_font() function
			size 		-- Height of the font
			x_offset	-- Offset pixels relative to top-left corner
			y_offset	-- Offset pixels relative to top-left corner
			utf_string	-- Bytes of string data that should be rendered
			string_len	-- Number of characters in the string

	@return Non-zero value on error, otherwise 0
*/
int
draw_text( size_t font_hash,
           uint32_t size,
           int32_t x_offset,
           int32_t y_offset,
           uint32_t* utf_string,
           uint32_t string_len,
           hex_t color )
{
	if (string_len == 0)
		return 0;

	glUseProgram(text_render_program);

	// glBindVertexArray(text_render_vao);
	// glBindBuffer(GL_ARRAY_BUFFER, text_render_vbo);

	// Manage the render program and projection matrix of drawing window
	{
		static uint32_t current_window_uniform_matrix = -1;

		if (current_window_uniform_matrix != current_drawing_window->id)
		{
			GLint projection_position = glGetUniformLocation(text_render_program, "projection");
			if (projection_position == -1)
			{
				fprintf(stderr, "Cannot get \"projection\" from text render program\n");
				// return -1;
			}
			glUniformMatrix4fv(projection_position, 1, GL_FALSE, current_window_projection[0]);

			// GLint viewport_size_position = glGetUniformLocation(text_render_program, "viewport_size");
			// if (viewport_size_position == -1)
			// {
			// 	fprintf(stderr, "Cannot get \"viewport_size\" from text render program\n");
			// 	// return -1;
			// }
			// glUniform2f(viewport_size_position, (float)current_drawing_window->width, (float)current_drawing_window->height);

			current_window_uniform_matrix = current_drawing_window->id;
		}
	}

	GLint color_modifier_position = glGetUniformLocation(text_render_program, "color_modifier");
	if (color_modifier_position == -1)
	{
		fprintf(stderr, "Cannot get \"color_modifier\" from text render program\n");
		// return -1;
	}
	glUniform4f(color_modifier_position, hex_uniform4f(color));

	Font* font = mapGet(fm.fonts, font_hash);

	if (font == NULL) {
		fprintf(stderr, "Cannot create buffer texture from unresolved font\n");
		return -1;
	}

	FontSize* font_s = mapGet(font->sizes, size);

	if (font_s == NULL) {
		font_s = new_font_size();
		mapAdd(font->sizes, size, font_s);
	}

	FT_Face face = (*(FT_Face*)font->source);

	// How much glyphs contained in a single texture
	// Spacing between characters is exaggerated because of the need to accommodate the baseline transformations
	uint32_t spacing = floor(size * 1.25);
	uint32_t range_size = pow((int32_t)floor(FONT_TEXTURE_SIZE / spacing), 2);

	// TODO Maybe we should directly operate on GL vertex buffer?
	// VBO constructor -- 6 vertices with 4 floats with each
	float buffer[STRIP_BUFFER_SIZE * 6 * 4];
	size_t buffer_len = 0;

	// Negative values of range should signal that none of the possible ranges is binded
	int current_range = -1;

	for (uint32_t i = 0; i < string_len; i++)
	{
		// Encountered character of non-binded range
		if ((int)(*utf_string / range_size) != current_range)
		{
			// Draw everything that was buffered (if buffered at all)
			if (buffer_len > 0)
			{
				draw_text_buffer(buffer, buffer_len);
				buffer_len = 0;
			}

			// Try to get desired character range
			FontRange* range = mapGet(font_s->ranges, *utf_string / range_size);

			// If there's no such range - create a new one
			if (range == NULL)
			{
				fprintf(stdout, "Creating range %d for the font %llu of size %d\n",
				        *utf_string / range_size, font_hash, size);

				// Create new texture
				GLuint texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

				// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, float{0.0f, 0.0f, 0.0f, 0.0f});

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
				check_opengl_state("Creation of texture buffer");

				if (FT_Set_Pixel_Sizes(*(FT_Face*)font->source, 0, size) != 0)
				{
					fprintf(stderr, "Cannot set the %llu font to the size of %d\n", font_hash, size);
					continue;
				}

				// Generate glyphs and write them to texture
				for (uint32_t i = 0; i < range_size; i++)
				{
					int err = FT_Load_Glyph(
					              *(FT_Face*)font->source,
					              FT_Get_Char_Index(*(FT_Face*)font->source, (*utf_string / range_size) * range_size + i),
					              FT_LOAD_DEFAULT);
					// If there's a problem with loading a glyph (for example when it is not present), - just skip it
					// TODO Maybe we should have some generic symbol that says that given character is not present in the font?
					if (err != 0) {
						fprintf(stderr, "Glyph %d skipped\n", (*utf_string / range_size) * range_size + i);
						continue;
					}

					FT_Glyph glyph;
					if (FT_Get_Glyph(face->glyph, &glyph) != 0) {
						fprintf(stderr, "Cannot get glyph\n");

						// TODO Possibly it detects it sooner
						// Load '?' if there's no such character (or problem with loading of it)
						int err = FT_Load_Glyph(
						              *(FT_Face*)font->source,
						              FT_Get_Char_Index(*(FT_Face*)font->source, '?'),
						              FT_LOAD_DEFAULT);
						if (err != 0)
							continue;

						if (FT_Get_Glyph(face->glyph, &glyph) != 0)
						{
							FT_Done_Glyph(glyph);
							continue;
						}
					}

					if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, true) != 0)
					{
						FT_Done_Glyph(glyph);
						continue;
					}

					FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

					glTexSubImage2D(
					    GL_TEXTURE_2D, 0,
					    (i % (FONT_TEXTURE_SIZE / spacing))*spacing + face->glyph->bitmap_left,
					    FONT_TEXTURE_SIZE - (i / (FONT_TEXTURE_SIZE / spacing))*spacing - face->glyph->bitmap_top,
					    bitmap_glyph->bitmap.width,
					    bitmap_glyph->bitmap.rows,
					    GL_RED,
					    GL_UNSIGNED_BYTE,
					    bitmap_glyph->bitmap.buffer);

					FT_Done_Glyph(glyph);
				}

				// Create and store a new font range
				FontRange* range_new = (FontRange*)malloc(sizeof(FontRange));
				range_new->texture = texture;
				range_new->width = size;
				range_new->height = size;

				mapAdd(font_s->ranges, *utf_string / range_size, range_new);
			}

			// If range already exist - bind its texture
			else
			{
				glBindTexture(GL_TEXTURE_2D, range->texture);
			}

			current_range = *utf_string / range_size;
		}

		// Glyph buffering
		if (buffer_len < STRIP_BUFFER_SIZE)
		{
			uint32_t idx = buffer_len * 6 * 4;

			// Casted to float to contain all of the data in a single array with UV coords
			float pos_left = x_offset + i * size;
			float pos_right = x_offset + (i + 1) * size;
			float pos_top = current_drawing_window->height - y_offset;
			float pos_bottom = current_drawing_window->height - y_offset - size;

			float tex_left = (*utf_string % (FONT_TEXTURE_SIZE / spacing)) * ((float)spacing / FONT_TEXTURE_SIZE);
			float tex_right = (*utf_string % (FONT_TEXTURE_SIZE / spacing) + 1) * ((float)spacing / FONT_TEXTURE_SIZE);
			float tex_top = 1.0 - ((*utf_string % range_size) / (FONT_TEXTURE_SIZE / spacing) + 1) * ((float)spacing / FONT_TEXTURE_SIZE);
			float tex_bottom = 1.0 - ((*utf_string % range_size) / (FONT_TEXTURE_SIZE / spacing)) * ((float)spacing / FONT_TEXTURE_SIZE);

			buffer[idx + 0] = pos_left;
			buffer[idx + 1] = pos_top;
			buffer[idx + 2] = tex_left;
			buffer[idx + 3] = tex_top;

			buffer[idx + 4] = pos_left;
			buffer[idx + 5] = pos_bottom;
			buffer[idx + 6] = tex_left;
			buffer[idx + 7] = tex_bottom;

			buffer[idx + 8] = pos_right;
			buffer[idx + 9] = pos_top;
			buffer[idx + 10] = tex_right;
			buffer[idx + 11] = tex_top;

			buffer[idx + 12] = pos_left;
			buffer[idx + 13] = pos_bottom;
			buffer[idx + 14] = tex_left;
			buffer[idx + 15] = tex_bottom;

			buffer[idx + 16] = pos_right;
			buffer[idx + 17] = pos_bottom;
			buffer[idx + 18] = tex_right;
			buffer[idx + 19] = tex_bottom;

			buffer[idx + 20] = pos_right;
			buffer[idx + 21] = pos_top;
			buffer[idx + 22] = tex_right;
			buffer[idx + 23] = tex_top;

			buffer_len++;
		}

		// If it was the last symbol - draw everything buffered and exit the loop
		if (i == string_len - 1 && buffer_len > 0)
		{
			draw_text_buffer(buffer, buffer_len);
			break;
		}

		utf_string++;
	}

	return 0;
}
