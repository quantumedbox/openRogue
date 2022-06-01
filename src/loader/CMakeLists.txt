cmake_minimum_required(VERSION 3.0)

project(OpenRogue_Loader C)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../)

add_executable(OpenRogue_Loader loader.c)

set_target_properties(OpenRogue_Loader PROPERTIES
	RUNTIME_OUTPUT_NAME "run")
