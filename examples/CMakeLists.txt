cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_FETCH_FROM_GIT on)

include(pico_sdk_import.cmake)
project(epd C CXX ASM)
set(PICO_BOARD_HEADER_DIRS ${CMAKE_SOURCE_DIR})

pico_sdk_init()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(epd_driver)

target_sources(epd_driver PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/../epd/platforms/rp2040_thinkink.c
    ${CMAKE_CURRENT_LIST_DIR}/../epd/panels/E2B98FS081.c
    )

target_include_directories(epd_driver PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../
    )

pico_enable_stdio_usb(epd_driver 1)
pico_enable_stdio_uart(epd_driver 0)

pico_add_extra_outputs(epd_driver)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message("Building with DEBUG")
    add_compile_definitions("DEBUG")
endif()

target_link_libraries(epd_driver pico_stdlib hardware_spi)
