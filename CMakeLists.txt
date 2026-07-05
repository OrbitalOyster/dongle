cmake_minimum_required(VERSION 3.50)
project(dongle LANGUAGES C)

add_executable(${PROJECT_NAME})

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# GLFW
include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY
  https://github.com/glfw/glfw.git
  GIT_TAG master
)
FetchContent_MakeAvailable(glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

include_directories(src/include)
target_sources(${PROJECT_NAME} PRIVATE
    src/main.c
)
