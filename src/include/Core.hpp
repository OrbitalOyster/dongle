#ifndef DONGLE_CORE_HPP
#define DONGLE CORE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

class Core {
private:
  GLFWwindow *window;

public:
  Core(int window_width, int window_height);
  void run();
  ~Core();
};

#endif
