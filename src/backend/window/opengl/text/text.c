#include <stdbool.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <GL/glew.h>

#include "backend.h"
#include "text.h"
#include "window/opengl/window.h"
#include "window/opengl/render_program.h"
#include "map.h"
#include "error.h"


// TODO Garbage collector-like that keeps track of usages per each texture in a certain time window


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


// ----------------------------------------------------------------- Global objects -- //


static FT_Library ft;

static Map* font_pool;

extern key_t current_drawing_window;

extern Map* window_pool;

static GLuint text_render_program;

static GLuint text_render_vao;

static GLuint text_render_vbo;


// -------------------------------------------------------------- Runtime constants -- //


// TODO Maybe give the ability to API caller to set size of textures ?
// TODO Relocate to special module for communicating such metrics

// Runtime const that is used for packing glyphs into the texture atlases
static GLint MAX_TEXTURE_SIZE;


// ------------------------------------------------------------------------ Helpers -- //


// Should return C-string containing the hint in which encoding rendering text should be passed
const char*
get_encoding ()
{
    return ENCODING;
}


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
int
init_text_subsystem()
{
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not initialize freetype\n");
		return -1;
	}

	text_render_program = new_render_program("resources/shaders/text.vert", "resources/shaders/text.frag");

	font_pool = mapNew();
	// strip_buffer_map = mapNew();

	#ifdef DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	#endif

	glGenVertexArrays(1, &text_render_vao);
	glBindVertexArray(text_render_vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &text_render_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, text_render_vbo);
	glBufferData(GL_ARRAY_BUFFER, TEXT_BUFFER_SIZE * sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)(sizeof(GLint) * 2));

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


EXPORT_SYMBOL
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
	glBindVertexArray(text_render_vao);

	glBindBuffer(GL_ARRAY_BUFFER, text_render_vbo);

	glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_len * sizeof(float) * 6 * 4, buffer);

	glDrawArrays(GL_TRIANGLES, 0, buffer_len * 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}


EXPORT_SYMBOL
int
draw_text( key_t font_hash,
           uint32_t size,
           int32_t x_offset,
           int32_t y_offset,
           uint32_t* utf_string,
           uint32_t string_len,
           hex_t color )
{
	if (string_len == 0)
		return 0;

	WindowHandler* window = (WindowHandler*)mapGet(window_pool, current_drawing_window);
	if (window == NULL)
		return -1;

	glUseProgram(text_render_program);

	GLint color_modifier_position = glGetUniformLocation(text_render_program, "color_modifier");
	if (color_modifier_position == -1)
	{
		fprintf(stderr, "Cannot get \"color_modifier\" from text render program\n");
		// return -1;
	}
	glUniform4f(color_modifier_position, hex_uniform4f(color));

	Font* font = mapGet(font_pool, font_hash);

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
	// Spacing between characters is exaggerated because of the need to accommodate the bearings
	uint32_t spacing = floor(size * 1.25);
	uint32_t range_size = pow((int32_t)floor(FONT_TEXTURE_SIZE / spacing), 2);

	// VBO constructor -- 6 vertices with 4 floats with each
	float buffer[TEXT_BUFFER_SIZE * 6 * 4];
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

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
					    FONT_TEXTURE_SIZE - (i / (FONT_TEXTURE_SIZE / spacing))*spacing - face->glyph->bitmap_top - spacing / 2,
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
			uint32_t idx = buffer_len * 6 * 4;

			// Sorry to everyone who's reading this shit
			float pos_left = (float)(x_offset + i * size) / window->width - 1.0;
			float pos_right = (float)(x_offset + (i + 1) * size) / window->width - 1.0;
			float pos_top = (float)(window->height - y_offset) / window->height;
			float pos_bottom = (float)(window->height - y_offset - size) / window->height;

			float tex_left = (*utf_string % (FONT_TEXTURE_SIZE / spacing)) * \
							((float)spacing / FONT_TEXTURE_SIZE);

			float tex_right = (*utf_string % (FONT_TEXTURE_SIZE / spacing) + 1) * \
							((float)spacing / FONT_TEXTURE_SIZE) - \
							((float)spacing / FONT_TEXTURE_SIZE) / 2;

			float tex_top = 1.0 - ((*utf_string % range_size) / \
							(FONT_TEXTURE_SIZE / spacing) + 1) * \
							((float)spacing / FONT_TEXTURE_SIZE);

			float tex_bottom = 1.0 - ((*utf_string % range_size) / \
							(FONT_TEXTURE_SIZE / spacing)) * \
							((float)spacing / FONT_TEXTURE_SIZE) - \
							((float)spacing / FONT_TEXTURE_SIZE) / 2;

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


/*
	@brief 	Should be invoked periodically to free resources that were not used in drawing since previous invocation of this function
*/
static
void
font_usage_collector()
{

}
