#pragma once

/*
	Helper functions for compiling shaders and linking then to a render program
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


// ------------------------------------------------------------ Interface -- //


GLuint 	new_render_program( char* vertexDir,
                            char* fragmentDir );

char* 	load_shader_from_file( char *dir );

GLuint 	compile_shader( const char *shader_code,
                        GLenum shader_type );

int 	link_shader_program( GLuint program,
                             unsigned char n,
                             ... );


// ---------------------------------------------------------------------- -- //


// ??? Maybe use strings as they're native to glShaderSource ???
char*
load_shader_from_file( char *dir )
{
	FILE *p_file = fopen(dir, "r");
	if (!p_file) {
		fprintf(stderr, "Cannot open %s\n", dir);
	}
	// Cheking the size of the file to know how much memory to allocate
	fseek(p_file, 0L, SEEK_END);
	char *code = (char*)malloc(ftell(p_file) + 1);
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
char*
get_opengl_log( GLuint shader )
{
	int log_size;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
	char* log = (char*)malloc(log_size);
	glGetShaderInfoLog(shader, log_size, NULL, log);
	return log;
}


GLuint
compile_shader( const char *shader_code,
                GLenum shader_type )
{
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_code, NULL);
	glCompileShader(shader);

	GLint is_successful;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &is_successful);
	if (!is_successful)
	{
		char* log = get_opengl_log(shader);
		fprintf(stderr, "Error while compiling shader:\n%s\n", log);
		free(log);
	}

	return shader;
}


int
link_shader_program( GLuint program,
                     unsigned char n,
                     ... )
{
	int error_status = 0;

	va_list(args);
	va_start(args, n);
	while (n--) {
		glAttachShader(program, va_arg(args, GLuint));
	}
	glLinkProgram(program);

	GLint is_successful;
	glGetProgramiv(program, GL_LINK_STATUS, &is_successful);
	if (!is_successful) {
		char* log = get_opengl_log(program);
		fprintf(stderr, "Error while linking shader:\n%s\n", log);
		free(log);

		error_status = -1;
	}

	return error_status;
}


// TODO How to signal failure ?
GLuint
new_render_program( char* vertexDir,
                    char* fragmentDir )
{
	char *vertexShaderCode = load_shader_from_file(vertexDir);
	char *fragmentShaderCode = load_shader_from_file(fragmentDir);

	GLuint vertexShader = compile_shader(vertexShaderCode, GL_VERTEX_SHADER);
	GLuint fragmentShader = compile_shader(fragmentShaderCode, GL_FRAGMENT_SHADER);

	GLuint renderProgram = glCreateProgram();
	if (renderProgram != 0)
	{
		if (link_shader_program(renderProgram, 2, vertexShader, fragmentShader) == -1) {
			glDeleteProgram(renderProgram);
		}
	} else {
		glDeleteProgram(renderProgram);
		fprintf(stderr, "Cannot create render program\n");
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	free(vertexShaderCode);
	free(fragmentShaderCode);

	return renderProgram;
}
