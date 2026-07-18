cmake_minimum_required(VERSION 3.50)
project(dongle LANGUAGES CXX)

add_executable(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Check if Vulkan SDK is installed
if (NOT IS_DIRECTORY $ENV{VULKAN_SDK})
  message(FATAL_ERROR "Vulkan SDK not found")
endif()

message(STATUS "Vulkan SDK found in $ENV{VULKAN_SDK}")
# Include everything from Vulkan SDK
include_directories(SYSTEM $ENV{VULKAN_SDK}/include)
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

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

# Main
include_directories(src/include)
target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/Core.cpp
)
