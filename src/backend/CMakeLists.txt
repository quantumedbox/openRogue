cmake_minimum_required(VERSION 3.0)

project(SDL_Backend C)

# TODO msvc args

set(LIBRARY_OUTPUT_DIRECTORY)

set(CMAKE_C_STANDARD 11)

# gcc adds 'lib' to the name, but msvc doesn't, what should we do ?
set(OUTPUT_NAME "libopenRogue_SDL")

set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/${OUTPUT_NAME})

set(SOURCES window/window.c
			window/render.c
			../map.c)

set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIRECTORY})

set(OS_SPECIFIC_LIBS "opengl32")

set(DEPS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../deps)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(GNU_ARGS -Wall -Wextra -Wpedantic -Werror -g
				 -Wno-unused-parameter -Wno-unused-variable)
else()
	set(GNU_ARGS -Wall -Wextra -Wpedantic -Werror -O3 -mavx2
				 -Wno-unused-parameter -Wno-unused-variable)
endif()

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
	foreach(ARG ${GNU_ARGS})
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
