set(TARGET_NAME candle_api_example)

add_executable(${TARGET_NAME}
	main.c
)

target_link_libraries(${TARGET_NAME}
	PRIVATE
		candle_api
		project_warnings
)