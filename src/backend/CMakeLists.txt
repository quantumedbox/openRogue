cmake_minimum_required(VERSION 3.0)

project(SDL_Backend C)

set(LIBRARY_OUTPUT_DIRECTORY)

set(CMAKE_C_STANDARD 11)

set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../backends)

set(SOURCES window/opengl/window.c
			window/opengl/text/text.c
			map.c)

set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIRECTORY})

# set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(SDL_Backend SHARED ${SOURCES})

set_target_properties(SDL_Backend PROPERTIES
	LIBRARY_OUTPUT_NAME "openRogue_SDL"
	# OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
	CMAKE_LIBRARY_OUTPUT_PATH ${OUTPUT_DIRECTORY})
	# LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})


find_library(SDL2_LIB sdl2)

if(NOT SDL2_LIB)
	message(SEND_ERROR "Cannot resolve SDL2 lib")
endif()

find_library(GLEW_LIB glew32)

if(NOT GLEW_LIB)
	message(SEND_ERROR "Cannot resolve GLEW lib")
endif()

find_library(FREETYPE_LIB freetype)

if(NOT FREETYPE_LIB)
	message(SEND_ERROR "Cannot resolve freetype lib")
endif()

# find_library(OPENGL_LIB opengl32)

# if(NOT OPENGL_LIB)
# 	message(SEND_ERROR "Cannot resolve opengl lib")
# endif()

# # Should be optional in the future
# find_library(OPENMP_LIB openmp)

# if(NOT OPENMP_LIB)
# 	message(SEND_ERROR "Cannot resolve openmp lib")
# endif()


target_link_libraries(SDL_Backend ${WindowSubmodule}
								  ${TextSubmodule}
								  ${SDL2_LIB}
								  ${GLEW_LIB}
								  ${FREETYPE_LIB}
								  opengl32)
								  # openmp)
