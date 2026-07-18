#include "Core.hpp"
#include "debug.hpp"
#include <cstdlib>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

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

  /* GLFW extensions */
  uint32_t glfw_extension_count = 0;
  glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  INFOF("GLFW extensions: %i", glfw_extension_count)

  const char **glfw_extension_names =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  for (uint32_t i = 0; i < glfw_extension_count; i++)
    INFOF("\t%s", glfw_extension_names[i])

  /* Instance */
  VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .pApplicationName = "Dongle",
                             .apiVersion = VK_API_VERSION_1_3};
  VkInstanceCreateInfo instance_create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
      .enabledExtensionCount = glfw_extension_count,
      .ppEnabledExtensionNames = glfw_extension_names,
  };
  VkInstance instance;
  if (vkCreateInstance(&instance_create_info, VK_NULL_HANDLE, &instance) != VK_SUCCESS) {
      throw std::runtime_error("Unable to create Vulkan instance");
  }
  INFO("Created Vulkan instance")
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
