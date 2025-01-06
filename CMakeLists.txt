cmake_minimum_required(VERSION 3.10)

set(VERSION 0.0.1)
set(TARGET_NAME candle_api)

project(${TARGET_NAME} VERSION ${VERSION})

add_library(${TARGET_NAME}
	SHARED
		src/candle.c
		src/candle_ctrl_req.c
)

target_compile_definitions(${TARGET_NAME} 
	PRIVATE
		-DCANDLE_API_LIBRARY 
		-DUNICODE
)

target_include_directories(${TARGET_NAME}
	PRIVATE	
		src
	PUBLIC
		include
)

set_target_properties(${TARGET_NAME} 
	PROPERTIES 
		PREFIX ""
)

target_link_libraries(${TARGET_NAME} 
	PRIVATE
		SetupApi
		winusb
		Ole32
)

install(TARGETS ${TARGET_NAME})
