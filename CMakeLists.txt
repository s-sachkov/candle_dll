cmake_minimum_required(VERSION 3.10)

project(candle_api
	VERSION 0.0.1
	LANGUAGES C
)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings
	INTERFACE
		-Wall
		-Wpedantic
		-Wextra
		-Werror
)

add_subdirectory(example)
add_subdirectory(src)
