#pragma once

//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// #include "errors.h"


// ------------------------------------------------------------ Interface -- //


GLuint new_render_program(char* vertexDir, char* fragmentDir);

char* load_shader_from_file(char *dir);

void compile_shader(GLuint *p_shader, const char *shader_code, GLenum shader_type);

void link_shader_program(GLuint program, unsigned char n, ...);


// ---------------------------------------------------------------------- -- //


char*
load_shader_from_file (char *dir)
{
	FILE *p_file = fopen(dir, "r");
	if (!p_file) {
		fprintf(stderr, "cannot open %s\n", dir);
	}
	// Cheking the size of the file to know how much memory to allocate
	fseek(p_file, 0L, SEEK_END);
	char *code = (char*) malloc(ftell(p_file) + 1);
	rewind(p_file);

	char read_buff = 0;
	char *p_code = code;
	while ((read_buff = getc(p_file)) != EOF) {
		*(p_code++) = read_buff;
	}
	*p_code = '\0';

	fclose(p_file);
	return code;
}


static
const char*
get_opengl_log (GLuint ptr,
                GLenum status_type)
{
	int log_size;
	glGetShaderiv(ptr, status_type, &log_size);
	const char* log = (char*)malloc(log_size);
	glGetShaderInfoLog(ptr, log_size, NULL, log);
	return log;
}


void
compile_shader (GLuint *p_shader,
                const char *shader_code,
                GLenum shader_type)
{
	*p_shader = glCreateShader(shader_type);
	glShaderSource(*p_shader, 1, &shader_code, NULL);
	glCompileShader(*p_shader);

	GLint is_successful;
	glGetShaderiv(*p_shader, GL_COMPILE_STATUS, &is_successful);
	if (!is_successful)
	{
		const char* log = get_opengl_log(*p_shader, GL_COMPILE_STATUS);
		fprintf(stderr, "error while compiling shader:\n%s\n", log);
		free(log);
	}
}


void
link_shader_program (GLuint program,
                     unsigned char n,
                     ...)
{
	va_list(args);
	va_start(args, n);
	while (n--) {
		glAttachShader(program, va_arg(args, GLuint));
	}
	glLinkProgram(program);

	GLint is_successful;
	glGetProgramiv(program, GL_LINK_STATUS, &is_successful);
	if (!is_successful) {
		const char* log = get_opengl_log(program, GL_LINK_STATUS);
		fprintf(stderr, "error while linking shader:\n%s\n", log);
		free(log);
	}
}


GLuint
new_render_program (char* vertexDir,
                    char* fragmentDir)
{
	GLuint vertexShader, fragmentShader;
	char *vertexShaderCode = load_shader_from_file(vertexDir);
	char *fragmentShaderCode = load_shader_from_file(fragmentDir);

	compile_shader(&vertexShader, vertexShaderCode, GL_VERTEX_SHADER);
	compile_shader(&fragmentShader, fragmentShaderCode, GL_FRAGMENT_SHADER);

	GLuint renderProgram = glCreateProgram();
	link_shader_program(renderProgram, 2, vertexShader, fragmentShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	free(vertexShaderCode);
	free(fragmentShaderCode);

	return renderProgram;
}
