cmake_minimum_required(VERSION 3.0)

project(SDL_Backend C)

# TODO Release and debug flags

set(LIBRARY_OUTPUT_DIRECTORY)

set(CMAKE_C_STANDARD 11)

set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../backends)

set(SOURCES window/window.c
			window/render.c
			../map.c)

set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIRECTORY})

set(OS_SPECIFIC_LIBS "opengl32")

set(DEPS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../deps)

set(GNU_DEBUG_ARGS -Wall -Wextra -Wpedantic -Werror -g
				   -Wno-unused-parameter -Wno-unused-variable)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../
					${CMAKE_CURRENT_SOURCE_DIR}
					${DEPS_PATH}/include)

add_library(SDL_Backend SHARED ${SOURCES})

set_target_properties(SDL_Backend PROPERTIES
	RUNTIME_OUTPUT_NAME "openRogue_SDL"
	CMAKE_LIBRARY_OUTPUT_PATH ${OUTPUT_DIRECTORY})

# MSVC specific setting
if(CMAKE_C_COMPILER_ID MATCHES MSVC)
	if(CMAKE_SIZEOF_VOID_P MATCHES 8)
		link_directories(${LINK_DIRECTORIES} ${DEPS_PATH}/msvc64)
	endif()
endif()

# MinGW/GNU specific settings
if(CMAKE_C_COMPILER_ID MATCHES GNU)
	add_definitions(-DGLEW_STATIC)
	link_directories(${LINK_DIRECTORIES} ${DEPS_PATH}/mingw64)
	foreach(ARG ${GNU_DEBUG_ARGS})
		set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS} ${ARG})
	endforeach()
endif()


if(WIN32)
	# Needed for static linking of SDL2
	list(APPEND OS_SPECIFIC_LIBS imagehlp dinput8
							dxguid dxerr8
							user32 gdi32
							winmm imm32
							ole32 oleaut32
							shell32 version
							uuid setupapi)
endif()

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
								  ${OS_SPECIFIC_LIBS})
