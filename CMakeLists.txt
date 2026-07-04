cmake_minimum_required(VERSION 3.50)
project(dongle LANGUAGES C)
add_executable(dongle)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include_directories(src/include)
target_sources(dongle PRIVATE
    src/main.c
)
