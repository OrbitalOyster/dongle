#include "debug.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <stdlib.h>

int main() {
  std::cout << "Hello, World!" << std::endl;
  INFO("Info message")

  /* GLFW window */
  if (!glfwInit())
    return EXIT_FAILURE;
  GLFWwindow *window = glfwCreateWindow(640, 480, "Dongle", NULL, NULL);
  glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
  if (!window) {
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(window);
  while (!glfwWindowShouldClose(window)) {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();

  return EXIT_SUCCESS;
}
