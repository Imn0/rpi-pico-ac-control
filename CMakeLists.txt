# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
   set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(PICO_SDK_PATH ${USERHOME}/.pico-sdk/sdk/1.5.1)
set(PICO_TOOLCHAIN_PATH ${USERHOME}/.pico-sdk/toolchain/13_2_Rel1)
if(WIN32)
    set(pico-sdk-tools_DIR ${USERHOME}/.pico-sdk/tools/1.5.1)
    include(${pico-sdk-tools_DIR}/pico-sdk-tools-config.cmake)
    include(${pico-sdk-tools_DIR}/pico-sdk-tools-config-version.cmake)
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(PROGRAM_NAME ac-control)
set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)

project(ac-control)

pico_sdk_init()

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.c" "${SRC_DIR}/*.h")
add_executable(ac-control  ${SRC_FILES} )

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/secrets/secrets.pass)
    include(${CMAKE_CURRENT_SOURCE_DIR}/secrets/secrets.pass)
else()
    message(WARNING "no wifi credentials")
endif()

target_include_directories(${PROGRAM_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${SRC_DIR}
)

target_link_libraries(${PROGRAM_NAME}
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    pico_lwip_mqtt
    pico_stdlib
    pico_multicore
    hardware_adc
    hardware_pio
    hardware_interp
)

target_compile_options(${PROGRAM_NAME} PRIVATE -Wall -Wextra  -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-but-set-parameter)
pico_enable_stdio_usb(${PROGRAM_NAME} FALSE)
pico_enable_stdio_uart(${PROGRAM_NAME} TRUE)

pico_add_extra_outputs(${PROGRAM_NAME})


set(output_html_fsdata "${CMAKE_CURRENT_BINARY_DIR}/src/fsdata.dat")
add_custom_command(
    OUTPUT ${output_html_fsdata}
    COMMAND
    py ./html_fsdata.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(run_html_fsdata ALL
  DEPENDS ${output_html_fsdata}
)

include(config.cmake)
add_subdirectory(src)


add_dependencies(ac-control run_html_fsdata)