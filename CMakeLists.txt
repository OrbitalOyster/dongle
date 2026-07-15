cmake_minimum_required(VERSION 3.50)
project(dongle LANGUAGES CXX)

add_executable(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GLFW
include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY
  https://github.com/glfw/glfw.git
  GIT_TAG master
)
FetchContent_MakeAvailable(glfw)
target_link_libraries(${PROJECT_NAME} glfw)

include_directories(src/include)
target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
)
