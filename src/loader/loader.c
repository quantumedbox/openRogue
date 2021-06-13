/*
	Executable that resolves the python executable path and config file variables
*/

// It's a fucking mess
// Maybe rewrite it in C++ as it's almost always present with C compiler that is needed for backend compilation

// TODO Err stream redirection if needed
// TODO Do not close console on fatal error
// TODO unix stuff isn't tested at all
// TODO Passing arguments to python
// TODO MSC comparability
// TODO Centralized error handling

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <dirent.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define get_dir _getcwd
#define set_env_var(name, var) _putenv_s(name, var)
#else
#include <unistd.h>
#define get_dir getcwd
#define set_env_var(name, var) setenv(name, var, 1)
#endif


// Max length of config line
#define BUFFER_SIZE 1024
//
#define MAX_COMMANDLINE_LENGTH 32768

#define CONFIG_PATH "config"

const char* interpreters[] = {
	"python", "py", "python3", "pypy3", "ipy",
};
const size_t n_interpreters = sizeof(interpreters) / sizeof(*interpreters);

const char* dirs_to_lookup[] = {
	"/python",
};
const size_t n_dirs = sizeof(dirs_to_lookup) / sizeof(*dirs_to_lookup);


char* ENGINEMODULE = NULL;


// char*
// nix_path_to_windows_path (const char* path)
// {
// 	char* n;
// 	// discrad relative './'
// 	if (strstr(path, "./") == path) {
// 		n = (char*)calloc(strlen(&path[2]) + 1, sizeof(char));
// 		strcpy(n, &path[2]);
// 	} else {
// 		n = (char*)calloc(strlen(path) + 1, sizeof(char));
// 		strcpy(n, path);
// 	}
// 	// swap '/' to '\'
// 	char* p = strtok(n, "/");
// 	while (p != NULL) {
// 		char* next = strtok(NULL, "/");
// 		if (next != NULL) {
// 			p[strlen(p)] = '\\';
// 		}
// 		p = next;
// 	}
// 	return n;
// }


// returned value should be freed
char*
get_config_variable (const char* var_name)
{
	FILE *f = fopen(CONFIG_PATH, "r");
	if (f == NULL) {
		perror("Cannot open a config file");
		return NULL;
	}

	char buff[BUFFER_SIZE] = {'\0'};
	size_t cur_line = 0;

	while (fgets(buff, BUFFER_SIZE, f) != NULL) {
		cur_line++;

		// Skip line that starts by '#' as commented
		if (buff[0] == '#') {
			continue;
		}

		// If buffer doesn't contain '\n' then the line could not fit into buffer
		// For now - just spit an error
		char* n_l = strchr(buff, '\n');
		if (n_l == NULL) {
			printf("Line %llu is too long for buffer to contain\n", cur_line);
			exit(EXIT_FAILURE);
		}

		// Divide line by '=' and consider first part of it being a key
		char* divisor = strchr(buff, '=');
		if (divisor != NULL) {
			char* name = (char*)calloc(divisor - buff + 1, sizeof(char));
			strncpy(name, buff, divisor - buff);
			name[divisor - buff] = '\0';

			// If key is matched, - return second part of divided line
			if (!strcmp(name, var_name)) {
				free(name);
				char* var = (char*)calloc(strlen(divisor + 2) + 1, sizeof(char));
				strcpy(var, divisor + 2);
				var[strlen(var) - 2] = '\0'; // erase "\n ending
				fclose(f);
				return var;
			}
		}
	}
	fclose(f);
	return NULL;
}

void
resolve_config ()
{
	char* scriptPath_v = get_config_variable("script_path");
	if (scriptPath_v) {
		set_env_var("PYTHONPATH", scriptPath_v);
		set_env_var("IRONPYTHONPATH", scriptPath_v);
		free(scriptPath_v);
	}

	char* c_compiler_v = get_config_variable("c_compiler");
	if (c_compiler_v) {
		set_env_var("CC", c_compiler_v);
		free(c_compiler_v);
	}

	char* engineModule_v = get_config_variable("engine_module");
	if (engineModule_v) {
		if (ENGINEMODULE != NULL) {
			free(ENGINEMODULE);
		}
		ENGINEMODULE = engineModule_v;
	} else {
		printf("Engine module isn't specified in config\n");
		exit(EXIT_FAILURE);
	}
}


bool
start_python (const char* py_path)
{
	char exec[MAX_COMMANDLINE_LENGTH];

	#ifdef _WIN32
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		// zeroing for initialization
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		sprintf(exec, "%s -m %s", py_path, ENGINEMODULE);

		if (CreateProcessA(
		            NULL, exec, NULL, NULL,
		            FALSE, 0, NULL,
		            NULL, &si, &pi
		        ) == 0
		   ) {
			int errc = GetLastError();
			if (errc == ENOENT) {
				return false;
			} else {
				printf("Error code %d while starting a python process:\n" \
				       "%s\nPlease check the 'python' configuration" \
				       "and make sure that it is available\n",
				       errc, exec
				      );
				exit(EXIT_FAILURE);
			}
		}
		// WaitForSingleObject(pi.hProcess, INFINITE);
	}
	#else
	{
		if (!system(NULL)) {
			perror("Cannot call process\n");
			exit(EXIT_FAILURE);
		}

		sprintf(exec, "%s -m %s", py_path, ENGINEMODULE);
		int errc = system(exec);

		if (errc == ENOENT) {
			return false;
		} else {
			perror("Error while trying to execute python\n");
			exit(EXIT_FAILURE);
		}
	}
	#endif

	return true;
}


bool
resolve_python ()
{
	// 'python' config line
	char* python_v = get_config_variable("python");
	if (python_v) {
		if (start_python(python_v)) {
			free(python_v);
			return true;
		}
		free(python_v);
	}
	// Search in PATH
	for (int i = 0; i < n_interpreters; i++) {
		if (start_python(interpreters[i])) {
			return true;
		}
	}
	// Search in subdirectories
	for (int i = 0; i < n_dirs; i++) {
		char buff[PATH_MAX];
		get_dir(buff, PATH_MAX);

		if (strlen(buff) + strlen(dirs_to_lookup[i] + 1) <= PATH_MAX) {
			strcpy(buff + strlen(buff), dirs_to_lookup[i]);

			// Check if dir exists
			// TODO WINAPI way
			DIR* dir = opendir(buff);
			if (dir)
				closedir(dir);
			else if (errno == ENOENT) {
				printf("Directory %s doesn't exits\n", buff);
				continue;
			} else
				exit(errno);

			// #ifdef _WIN32
			// char* winpath = nix_path_to_windows_path(buff);
			// strcpy(buff, winpath);
			// free(winpath);
			// #endif

			ssize_t dir_b = strlen(buff);
			buff[dir_b] = '/';

			// TODO Path len checks
			for (int i = 0; i < n_interpreters; i++) {
				strcpy(buff + dir_b + 1, interpreters[i]);
				if (start_python(buff)) {
					return true;
				}
			}

			if (start_python(buff)) {
				return true;
			}

		} else {
			printf("Warning: skipping possible dir checking because of path length\n");
		}
	}
	return false;
}


int
main (int argc, const char** argv)
{
	resolve_config();
	if (!resolve_python()) {
		printf("Cannot find any suitable python executable in the system\n" \
		       "If your system does have it, - configure your system PATH to contain it.\n" \
		       "Or set the 'python' variable in 'config' file of openRogue.\n" \
		       "Alternativly, you could install python in 'python' subdirectory of this folder.");
		return ENOENT;
	}

	return 0;
}
