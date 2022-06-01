#include <stdbool.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <GL/glew.h>

#include "rogue_definitions.h"
#include "render.h"
#include "window.h"
#include "render_program.h"
#include "map.h"

// Default texture size in both dimensions
#ifndef FONT_TEXTURE_SIZE
#define FONT_TEXTURE_SIZE 1024
#endif

// Max amount of characters per strip
#ifndef TEXT_BUFFER_SIZE
#define TEXT_BUFFER_SIZE 128
#endif

// Maybe it's better to have it in config ?
#ifndef DEFAULT_FONT
#define DEFAULT_FONT "resources/fonts/FSEX300.ttf"
#endif

#ifndef ENCODING
#define ENCODING "utf-32-le"
#endif


// TODO Multiline text drawing calls for tilemaps and such

// TODO Font style modificators

// TODO Garbage collector-like mechanism that keeps track of usages per each texture in a certain time window

// TODO Function to render text vertically ?

// TODO Setting custom texture spacing

// TODO Setting custom text rendering spacing


// -------------------------------------------------------------------- Definitions -- //


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
	Holder for commonly used render configuration
*/
typedef struct
{
	GLuint program;
	GLuint vao;
	GLuint vbo;
}
RenderObject;


// ----------------------------------------------------------------- Global objects -- //


static FT_Library freetype;

static Map* font_pool;

extern Map* window_pool;


static RenderObject text_renderer;

static RenderObject rect_renderer;


// ------------------------------------------------------------------------ Helpers -- //


// Should return C-string containing the hint in which encoding rendering text should be passed
const char*
get_encoding ()
{
	return ENCODING;
}


#ifndef ROGUE_DEBUG
void
check_opengl_state( char* description )
{
	GLenum err;
	if ((err = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "OPENGL ERROR :: CODE 0x%x :: %s\n", err, description);
	}
}
#endif

#ifdef ROGUE_DEBUG
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
int
init_text_subsystem()
{
	if (FT_Init_FreeType(&freetype)) {
		fprintf(stderr, "Could not initialize freetype\n");
		return -1;
	}

	font_pool = mapNew();

	#ifdef DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	#endif

	// Text rendering globals
	{
		text_renderer.program = new_render_program("resources/shaders/text.vert", "resources/shaders/text.frag");

		glGenVertexArrays(1, &text_renderer.vao);
		glBindVertexArray(text_renderer.vao);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &text_renderer.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, text_renderer.vbo);
		glBufferData(GL_ARRAY_BUFFER, TEXT_BUFFER_SIZE * sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)(sizeof(GLint) * 2));
	}

	// Rect rendering globals
	{
		rect_renderer.program = new_render_program("resources/shaders/rect.vert", "resources/shaders/rect.frag");

		glGenVertexArrays(1, &rect_renderer.vao);
		glBindVertexArray(rect_renderer.vao);

		glEnableVertexAttribArray(0);

		glGenBuffers(1, &rect_renderer.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, rect_renderer.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, NULL, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return 0;
}


/*
	@brief 	Returns reference to a newly heap allocated FontSize
			It has "used" mark to allow escaping GC
*/
static
inline
FontSize*
new_font_size()
{
	FontSize* new = (FontSize*)malloc(sizeof(FontSize));
	new->was_used = true;
	new->ranges = mapNew();

	return new;
}


/*
	@brief	If desired path is not valid then default font path is used
			If default font isn't valid - returns NULL to signal error
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
			Returns NULL on error
*/
static
Font*
new_font( const char* path )
{
	const char* suitable = find_suitable_font(path);
	if (suitable == NULL)
		return NULL;

	FT_Face* face = (FT_Face*)malloc(sizeof(FT_Face));

	if (FT_New_Face(freetype, suitable, 0, face) != 0)
	{
		fprintf(stderr, "Cannot load font at %s\n", suitable);
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


ROGUE_EXPORT
key_t
resolve_font( const char* path )
{
	key_t path_hash = hash_string(path);

	Font* font = mapGet(font_pool, path_hash);
	if (font == NULL)
	{
		Font* font_n = new_font(path);
		if (font_n == NULL)
			return NONE_KEY;
		mapAdd(font_pool, path_hash, font_n);
	}

	return path_hash;
}


static
void
draw_text_buffer( float* buffer, size_t buffer_len )
{
	glBindVertexArray(text_renderer.vao);

	glBindBuffer(GL_ARRAY_BUFFER, text_renderer.vbo);

	glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_len * sizeof(float) * 6 * 4, buffer);

	glDrawArrays(GL_TRIANGLES, 0, buffer_len * 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

// TODO Could be a lot better or at least more nicely structured
ROGUE_EXPORT
int
draw_text( key_t font_hash,
           uint32_t size,
           uint32_t width,
           int32_t x_offset,
           int32_t y_offset,
           char32_t* utf_string,
           uint32_t string_len,
           hex_t color )
{
	WindowHandler* window;
	Font* font;
	FontSize* font_s;
	FT_Face face;

	window = (WindowHandler*)mapGet(window_pool, get_current_drawing_window());
	if (!window)
		return -1;

	font = mapGet(font_pool, font_hash);

	if (font == NULL) {
		fprintf(stderr, "Cannot create buffer texture from unresolved font\n");
		return -1;
	}

	if (font->type == FONT_SOURCE_FREETYPE) {
		face = (*(FT_Face*)font->source);
	} else {
		// TODO Support for bitmaps
		return -1;
	}

	font_s = mapGet(font->sizes, size);

	if (font_s == NULL) {
		font_s = new_font_size();
		mapAdd(font->sizes, size, font_s);
	}

	glUseProgram(text_renderer.program);

	// ??? Is it okay to hardcode the uniform locations for optimization ?
	glUniform4f(0, hex_uniform4f(color));

	// How much glyphs contained in a single texture
	// Spacing between characters is exaggerated because of the need to accommodate the bearings
	uint32_t spacing = floor(size * 1.25);
	uint32_t range_size = pow((int32_t)floor(FONT_TEXTURE_SIZE / spacing), 2);

	// VBO constructor -- 6 vertices with 4 floats for each glyph
	float buffer[TEXT_BUFFER_SIZE * 6 * 4];
	size_t buffer_len = 0;

	// Negative values of range should signal that none of the possible ranges is binded
	int current_range = -1;

	for (uint32_t i = 0; i < string_len; i++)
	{
		// Optimization case for the whitespace
		if (*utf_string == ' ')
		{
			// Fill the buffer element with dummy zeroes
			memset(&buffer[buffer_len * 6 * 4], 0, 24 * sizeof(float));
			buffer_len++;

		}
		// General case
		else
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

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

					glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
					check_opengl_state("Creation of texture buffer");

					if (FT_Set_Pixel_Sizes(*(FT_Face*)font->source, size, size) != 0)
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
						              FT_LOAD_RENDER);
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
							              FT_LOAD_RENDER);
							if (err != 0)
								continue;

							if (FT_Get_Glyph(face->glyph, &glyph) != 0)
							{
								FT_Done_Glyph(glyph);
								continue;
							}
						}

						if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_LCD, 0, true) != 0)
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
			if (buffer_len < TEXT_BUFFER_SIZE)
			{
				register uint32_t idx = buffer_len * 6 * 4;

				float pos_left = ((float)x_offset + i * width) / (window->width / 2) - 1.0f;
				float pos_right = ((float)x_offset + i * width + size) / (window->width / 2) - 1.0f;
				float pos_top = ((float)window->height - y_offset) / (window->height / 2) - 1.0f;
				float pos_bottom = ((float)window->height - y_offset - size) / (window->height / 2) - 1.0f;

				float tex_left = (*utf_string % (FONT_TEXTURE_SIZE / spacing)) * ((float)spacing / FONT_TEXTURE_SIZE);

				float tex_right = (*utf_string % (FONT_TEXTURE_SIZE / spacing)) * ((float)spacing / FONT_TEXTURE_SIZE) + \
				                  ((float)size / FONT_TEXTURE_SIZE);

				float tex_top = 1.0 - ((*utf_string % range_size) / (FONT_TEXTURE_SIZE / spacing)) * \
				                ((float)spacing / FONT_TEXTURE_SIZE) - ((float)size / FONT_TEXTURE_SIZE) * 0.75;

				float tex_bottom = 1.0 - (((*utf_string % range_size) / (FONT_TEXTURE_SIZE / spacing))) * \
				                   ((float)spacing / FONT_TEXTURE_SIZE) + ((float)size / FONT_TEXTURE_SIZE) * 0.25;

				buffer[idx++] = pos_left;
				buffer[idx++] = pos_top;
				buffer[idx++] = tex_left;
				buffer[idx++] = tex_top;

				buffer[idx++] = pos_left;
				buffer[idx++] = pos_bottom;
				buffer[idx++] = tex_left;
				buffer[idx++] = tex_bottom;

				buffer[idx++] = pos_right;
				buffer[idx++] = pos_top;
				buffer[idx++] = tex_right;
				buffer[idx++] = tex_top;

				buffer[idx++] = pos_left;
				buffer[idx++] = pos_bottom;
				buffer[idx++] = tex_left;
				buffer[idx++] = tex_bottom;

				buffer[idx++] = pos_right;
				buffer[idx++] = pos_bottom;
				buffer[idx++] = tex_right;
				buffer[idx++] = tex_bottom;

				buffer[idx++] = pos_right;
				buffer[idx++] = pos_top;
				buffer[idx++] = tex_right;
				buffer[idx++] = tex_top;

				buffer_len++;
			}
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


// ??? For some reason this method is REALLY slow, needs fixing
ROGUE_EXPORT
void
draw_rect( int32_t x_offset,
           int32_t y_offset,
           int32_t width,
           int32_t height,
           hex_t color )
{
	WindowHandler* window = (WindowHandler*)mapGet(window_pool, get_current_drawing_window());
	if (!window) return;

	glUseProgram(rect_renderer.program);

	glUniform4f(0, hex_uniform4f(color));

	glBindVertexArray(rect_renderer.vao);

	glBindBuffer(GL_ARRAY_BUFFER, rect_renderer.vbo);

	float pos_left = (float)x_offset / (window->width / 2) - 1.0f;
	float pos_right = ((float)x_offset + width) / (window->width / 2) - 1.0f;
	float pos_top = ((float)window->height - y_offset) / (window->height / 2) - 1.0f;
	float pos_bottom = ((float)window->height - y_offset - height) / (window->height / 2) - 1.0f;

	float buffer[6 * 2] = {
		pos_left, pos_top,
		pos_left, pos_bottom,
		pos_right, pos_top,
		pos_left, pos_bottom,
		pos_right, pos_bottom,
		pos_right, pos_top,
	};

	glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 2 * sizeof(float), buffer);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}


/*
	@brief 	Should be invoked periodically to free resources that were not used in drawing since previous invocation of this function
*/
static
void
font_usage_collector()
{
	// TODO
}
