cmake_minimum_required(VERSION 3.20)

add_library(ds1302_rtc INTERFACE)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

target_sources(ssd1306_oled_pico INTERFACE ${SRC_FILES})
target_include_directories(ssd1306_oled_pico INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

