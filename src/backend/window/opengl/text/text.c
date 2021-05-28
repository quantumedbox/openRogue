#include <stdbool.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glew.h>

#include "text.h"
#include "window/opengl/render_program.h"
#include "map.h"
#include "error.h"

// TODO Refcounts for usages of fonts, their ranges and sized of those ranges?


// -------------------------------------------------------------------- Definitions -- //


typedef enum {
	// Indicates that source file is freetype font
	FONT_SOURCE_FREETYPE,
	// Indicates that source file is image
	FONT_SOURCE_BITMAP,
}
FontSourceType;

/*
*/
typedef struct
{
	FontSourceType type;
	void* source;
	Map* charmap;
}
FontSource;

/*
	Stores and manages loaded fonts
*/
typedef struct
{
	// Stores FT_Face by path keys
	Map* fonts;
}
FontManager;


// ----------------------------------------------------------------- Global objects -- //


static bool is_text_subsystem_initialized = false;

static FT_Library ft;

static FontManager fm;


// Used for packing glyphs into the texture atlases
static GLint MAX_TEXTURE_SIZE = 0;


// -------------------------------------------------------------------- Realization -- //


int
init_text_subsystem()
{
	if (FT_Init_FreeType(&ft)) {
		printf("Could not initialize freetype\n");
		// SIGNAL_ERROR();
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
	Check if given font is loaded and if not, - load it
*/
// int
// resolve_font(const char* path, int size)
// {

// 	return 0;
// }


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
