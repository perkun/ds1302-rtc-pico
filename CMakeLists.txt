cmake_minimum_required(VERSION 3.20)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
pico_sdk_init()

add_library(ds1302_rtc INTERFACE)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

target_sources(ds1302_rtc INTERFACE ${SRC_FILES})
target_include_directories(ds1302_rtc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

