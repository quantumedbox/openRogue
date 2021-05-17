/*
	Executable that resolves the path to Python executable and config file
	For now only POSIX, compile it with MinGW ubder Windows
*/

// It should be so fucking error-prone, damn
// For now i don't really care that much tho

// TODO divide the module into several files for each os-dependent way
// TODO wide char/utf-8 support
// TODO free() on allocated strings

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef _MSC_VER
#include <dirent.h>
#else
#error "Microsoft Compiler isn't supported for now as it doesn't implement dirent.h\nSources are available and you could add support for it through Win32 api"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

// Be careful with it!
#define BUFFER_SIZE 1024
#define COMMAND_LINE_BUFFER 32768 // Maximum for windows

#define CONFIG_PATH "config"

const char* interpreters_list[] = {
	"python", "py", "python3", "pypy3",
};
const size_t n_interpreters = sizeof(interpreters_list) / sizeof(*interpreters_list);

const char* dirs_to_lookup[] = {
	"/python",
};
const size_t n_dirs = sizeof(dirs_to_lookup) / sizeof(*dirs_to_lookup);


char* nix_path_to_windows(const char* path) {
	char* n;
	// discrad relative './'
	if (strstr(path, "./") == path) {
		n = (char*)calloc(strlen(&path[2])+1, sizeof(char));
		strcpy(n, &path[2]);
	} else {
		n = (char*)calloc(strlen(path)+1, sizeof(char));
		strcpy(n, path);
	}

	// swap '/' to '\'
	char* p = strtok(n, "/");
	while (p != NULL) {
		char* next = strtok(NULL, "/");
		if (next != NULL) {
			p[strlen(p)] = '\\';
		}
		p = next;
	}

	// if extension isn't present then assume that the file is .exe
	// (which means that paths cannot contain dots unless they're indicating extensions which might be a problem)
	if (strchr(n, '.') == NULL) {
		size_t len = strlen(n);
		n = (char*)realloc(n, (len+5) * sizeof(char));
		strcpy(n + len, ".exe");
	}

	return n;
}


const char* get_interpreter_from_path() {
	const char* _PATH = getenv("PATH");
	if (_PATH == NULL) {
		perror("PATH variable isn't present in environment");
		return NULL;
	}

	char* PATH = (char*)calloc(strlen(_PATH)+1, sizeof(char));
	strcpy(PATH, _PATH);

	#ifdef _WIN32
	char *p = strtok(PATH, ";");
	#else
	char *p = strtok(PATH, ":");
	#endif

	char* inter_path = NULL;

	while(p != NULL) {
		DIR *dir;
		struct dirent *ent;
		// Loop through every directory in PATH
		if ((dir = opendir(p)) != NULL) {
			// Loop through every file in the directory
			while ((ent = readdir(dir)) != NULL) {
				// If name of the file consist a dot, - divide the name by it
				char* dot = strchr(ent->d_name, '.');
				char* f_name = {'\0'};
				if (dot != NULL) {
					ptrdiff_t f_name_s = dot - ent->d_name;
					f_name = (char*)calloc(f_name_s+1, sizeof(char));
					strncpy(f_name, ent->d_name, f_name_s);
				// ... or use the name as it is
				} else {
					f_name = ent->d_name;
				}
				// Finally, compare found file to a interpreter list
				for (size_t i = n_interpreters; i--;) {
					if (!strcmp(interpreters_list[i], f_name)) {
						// If match is found - store a full path to it into 'inter_path' and break from loop
						inter_path = (char*)calloc(strlen(p)+ent->d_namlen+2, sizeof(char));
						strcpy(inter_path, p);
						#ifdef _WIN32
						inter_path[strlen(inter_path)] = '\\';
						#else
						inter_path[strlen(inter_path)] = '/';
						#endif
						strcpy(&inter_path[strlen(p)+1], ent->d_name);
						goto FOUND;
					}
				}
			}
		FOUND:
			closedir(dir);
		} else {
			perror("Cannot open directory to fetch files:\n");
			exit(EXIT_FAILURE);
		}

		if (inter_path != NULL) {
			break;
		}

		#ifdef _WIN32
		p = strtok(NULL, ";");
		#else
		p = strtok(NULL, ":");
		#endif
	}

	free(PATH);

	return inter_path;
}


char* get_config_variable(const char* var_name) {
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
			printf("Line %d is too long for buffer to contain\n", cur_line);
			exit(EXIT_FAILURE);
		}

		// Divide line by '=' and consider first part of it being a key
		char* divisor = strchr(buff, '=');
		if (divisor != NULL) {
			char* name = (char*)calloc(divisor-buff+1, sizeof(char));
			strncpy(name, buff, divisor-buff);
			name[divisor-buff] = '\0';

			// If key is matched, - return second part of divided line
			if (!strcmp(name, var_name)) {
				free(name);
				char* var = (char*)calloc(strlen(divisor+2)+1, sizeof(char));
				strcpy(var, divisor+2);
				var[strlen(var)-2] = '\0'; // erase "\n ending
				fclose(f);
				return var;
			}
		}
	}
	fclose(f);
	return NULL;
}


void init_environment() {
	char* scriptPath_v = get_config_variable("scriptPath");
	if (scriptPath_v) {
		_putenv_s("PATH", scriptPath_v);
		free(scriptPath_v);
	}

	char* c_compiler_v = get_config_variable("c_compiler");
	if (c_compiler_v) {
		_putenv_s("c_compiler", c_compiler_v);
		free(c_compiler_v);
	}
}


void start_python(const char* py_path) {
	init_environment();

	char* engineModule = get_config_variable("enginePath");
	if (engineModule == NULL) {
		printf("Engine module isn't specified in config\n");
		exit(EXIT_FAILURE);
	}

	{
	// POSIX variant
		#if defined __unix__ || defined __unix || defined unix
		char exec[COMMAND_LINE_BUFFER] = {'\0'};
		sprintf(exec, "%s -m %s", py_path, engineModule);
	
		free(engineModule);
	
		if (!system(NULL)) {
			perror("Cannot call process\n");
			exit(EXIT_FAILURE);
		}
		int exit_code = system(exec);
		#endif
	}

	{
	// WINAPI call of CreateProcessA
		#if defined _WIN32 || defined _WIN64
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
	
		// zeroing for initialization
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		char exec[COMMAND_LINE_BUFFER] = {'\0'};
		sprintf(exec, "-m %s", engineModule);
	
		free(engineModule);
		char* winpath = nix_path_to_windows(py_path);

		if (CreateProcessA(
				winpath,
				exec, NULL, NULL,
				FALSE, 0, NULL,
				NULL, &si, &pi) == 0
		) {
			int err = GetLastError();
			printf("Error code %d while starting a new process\n", err);
			exit(EXIT_FAILURE);
		}
		WaitForSingleObject(pi.hProcess, INFINITE);
	
		free(winpath);
		#endif
	}
}


int main(int argc, const char** argv) {
	const char* py = get_config_variable("python");
	if (py) {
		start_python(py);
	} else {
		py = get_interpreter_from_path();
		if (py) {
			start_python(py);
		} else {
			printf("Cannot find python interpreter in PATH and 'python' config setting wasn't given\n");
			exit(EXIT_FAILURE);
		} 
		// TODO Search in default dirs if nothing found
	}

	return 0;
}
