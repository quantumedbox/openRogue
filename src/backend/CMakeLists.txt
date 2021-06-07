cmake_minimum_required(VERSION 3.0)

project(SDL_Backend C)

# TODO Release and debug flags

set(LIBRARY_OUTPUT_DIRECTORY)

set(CMAKE_C_STANDARD 11)

set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../backends)

set(SOURCES window/opengl/window.c
			window/opengl/text/text.c
			map.c)

set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIRECTORY})

set(LINKER_LIBS "opengl32")


include_directories(${CMAKE_CURRENT_SOURCE_DIR}
					${INCLUDE_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/include)

add_library(SDL_Backend SHARED ${SOURCES})

set_target_properties(SDL_Backend PROPERTIES
	RUNTIME_OUTPUT_NAME "openRogue_SDL"
	CMAKE_LIBRARY_OUTPUT_PATH ${OUTPUT_DIRECTORY})


if(CMAKE_C_COMPILER_ID MATCHES MSVC)
	# list(APPEND LINKER_LIBS kernel32)

	if(CMAKE_SIZEOF_VOID_P MATCHES 8)
		link_directories(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/msvc64/lib)
		set(CMAKE_LIBRARY_PATH ${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/msvc64/lib)
	endif()
endif()

# if(CMAKE_C_COMPILER_ID MATCHES GCC)
# 	set(CMAKE_C_FLAGS_DEBUG -Wall -Wextra -Wpedantic -Werror
# 							-g -Wno-unused-parameter -Wno-unused-variable)
# endif()


find_library(SDL2_LIB sdl2)

if(NOT SDL2_LIB)
	message(SEND_ERROR "Cannot resolve SDL2 lib")
endif()

find_library(GLEW_LIB glew32)

if(NOT GLEW_LIB)
	message(SEND_ERROR "Cannot resolve GLEW lib")
endif()

find_library(FREETYPE_LIB freetype freetyped)

if(NOT FREETYPE_LIB)
	message(SEND_ERROR "Cannot resolve freetype lib")
endif()


target_link_libraries(SDL_Backend ${WindowSubmodule}
								  ${TextSubmodule}
								  ${SDL2_LIB}
								  ${GLEW_LIB}
								  ${FREETYPE_LIB}
								  ${LINKER_LIBS})
