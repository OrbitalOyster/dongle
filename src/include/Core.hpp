#ifndef DONGLE_CORE_HPP
#define DONGLE CORE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct ShaderData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model[3];
    glm::vec4 lightPos{ 0.0f, -10.0f, 10.0f, 0.0f };
    uint32_t selected{1};
};

class Core {
private:
  GLFWwindow *window = nullptr;
  VkInstance instance{VK_NULL_HANDLE};
  VkPhysicalDevice selected_physical_device{VK_NULL_HANDLE};
  VkDevice device{VK_NULL_HANDLE};
  VkQueue queue{VK_NULL_HANDLE};

public:
  Core(int window_width, int window_height);
  void run();
  ~Core();
};

#endif
