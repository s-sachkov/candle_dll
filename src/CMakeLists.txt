
set(TARGET_NAME ${PROJECT_NAME})

add_library(${TARGET_NAME}
	SHARED
		candle.c
		candle_ctrl_req.c
)

target_compile_definitions(${TARGET_NAME}
	PRIVATE
		-DCANDLE_API_LIBRARY
		-DUNICODE
)

target_include_directories(${TARGET_NAME}
	PRIVATE
		.
	PUBLIC
		../include
)

set_target_properties(${TARGET_NAME}
	PROPERTIES
		PREFIX ""
		VERSION ${PROJECT_VERSION}
)

target_link_libraries(${TARGET_NAME}
	PRIVATE
		SetupApi
		winusb
		Ole32
)

install(TARGETS ${TARGET_NAME})
