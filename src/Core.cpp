#include "Core.hpp"
#include "debug.hpp"
#include <stdexcept>

Core::Core(int window_width, int window_height) {
  /* Init GLFW */
  if (!glfwInit())
    throw std::runtime_error("Unable to initialize GLFW");
  INFO("Initialized GLFW");

  /* GLFW window */
  window = glfwCreateWindow(window_width, window_height, "Dongle", NULL, NULL);
  glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Unable to create GLFW window");
  }
  glfwMakeContextCurrent(window);
  INFOF("Created %dx%d window", window_width, window_height);
}

void Core::run() {
  while (!glfwWindowShouldClose(window)) {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

Core::~Core() {
    glfwTerminate();
    INFO("Terminated GLFW");
}
