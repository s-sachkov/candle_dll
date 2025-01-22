cmake_minimum_required(VERSION 3.20)

project(candle_api
	VERSION 0.0.1
	LANGUAGES C
)

option(CANDLE_DLL_EXAMPLE "Enable the building of the example" OFF)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings
	INTERFACE
		-Wall
		-Wpedantic
		-Wextra
		-Werror
)

add_subdirectory(src)

if(${CANDLE_DLL_EXAMPLE})
	add_subdirectory(example)
endif()
