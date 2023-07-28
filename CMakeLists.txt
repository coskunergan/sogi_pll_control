cmake_minimum_required (VERSION 3.6)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/stm32_core/arm-none-eabi-gcc.cmake) 

project(Project)

file(GLOB_RECURSE USER_SOURCES_EX  
        src/*.cpp
)

include_directories(
        inc
)

add_subdirectory(stm32_core)
