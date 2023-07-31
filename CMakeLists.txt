cmake_minimum_required (VERSION 3.6)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/stm32_core/arm-none-eabi-gcc.cmake) 

project(Project)

file(GLOB_RECURSE USER_SOURCE  
        src/*.cpp
)

list (APPEND USER_SOURCES ${USER_SOURCE})

include_directories(
        inc
)

add_subdirectory(stm32_core)
