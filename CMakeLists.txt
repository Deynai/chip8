cmake_minimum_required(VERSION 3.5)
project(chip8)

option(MAIN_VENDORED "Use vendored libraries" OFF)

if(MAIN_VENDORED)
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
else()
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

file(GLOB sources src/*.cpp)
add_executable(chip8 ${sources})

target_link_libraries(chip8 PRIVATE SDL3::SDL3)