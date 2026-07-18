#ifndef DONGLE_CORE_HPP
#define DONGLE CORE_HPP

#include <GLFW/glfw3.h>

class Core {
private:
  GLFWwindow *window;

public:
  Core(int window_width, int window_height);
  void run();
  ~Core();
};

#endif
