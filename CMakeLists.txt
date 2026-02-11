cmake_minimum_required(VERSION 3.16)

project(marching_squares
    VERSION 0.1
    LANGUAGES C
)

find_package(SDL2 REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(marching_squares
  src/main.c
)

target_link_libraries(marching_squares PRIVATE SDL2::SDL2)
target_include_directories(marching_squares PRIVATE NES)
