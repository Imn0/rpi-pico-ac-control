pico_generate_pio_header(${PROGRAM_NAME} ${CMAKE_CURRENT_LIST_DIR}/mitsubishi_heavy_152b_protocol.pio)

target_include_directories (${PROGRAM_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)