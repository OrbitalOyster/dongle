#include "Core.hpp"
#include "debug.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

Core::Core(int window_width, int window_height) {
  /* Init GLFW */
  if (!glfwInit())
    throw std::runtime_error("Unable to initialize GLFW");
  INFO("Initialized GLFW");
  /* GLFW window */
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(window_width, window_height, "Dongle", NULL, NULL);
  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Unable to create GLFW window");
  }
  glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
  glfwMakeContextCurrent(window);
  INFOF("Created %dx%d window", window_width, window_height);
  /* GLFW extensions */
  uint32_t glfw_extension_count{0};
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
  VkInstance instance{VK_NULL_HANDLE};
  if (vkCreateInstance(&instance_create_info, VK_NULL_HANDLE, &instance) !=
      VK_SUCCESS) {
    throw std::runtime_error("Unable to create Vulkan instance");
  }
  INFO("Created Vulkan instance")
  /* Physical devices */
  uint32_t device_count{0};
  if (vkEnumeratePhysicalDevices(instance, &device_count, nullptr) !=
      VK_SUCCESS)
    throw std::runtime_error("Unable to enumerate physical devices");
  std::vector<VkPhysicalDevice> devices(device_count);
  if (vkEnumeratePhysicalDevices(instance, &device_count, devices.data()) !=
      VK_SUCCESS)
    throw std::runtime_error("Unable to enumerate physical devices");
  INFOF("Physical devices: %d", device_count);
  /* Find discrete GPU */
  uint32_t selected_physical_device{0};
  if (device_count > 1)
    for (uint32_t i = 0; i < device_count; i++) {
      VkPhysicalDeviceProperties2 device_properties{
          .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
      vkGetPhysicalDeviceProperties2(devices[i], &device_properties);
      bool discrete = device_properties.properties.deviceType ==
                      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
      INFOF("\t[%d] %s, discrete: %s", i,
            device_properties.properties.deviceName, discrete ? "yes" : "no");
      if (discrete)
        selected_physical_device = i;
    }
  /* Find queue family */
  uint32_t queue_family_count{0};
  vkGetPhysicalDeviceQueueFamilyProperties(devices[selected_physical_device],
                                           &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(devices[selected_physical_device],
                                           &queue_family_count,
                                           queue_families.data());
  INFOF("Queue families: %d", queue_family_count);
  uint32_t selected_queue_family{0};
  for (size_t i = 0; i < queue_families.size(); i++) {
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      selected_queue_family = i;
      break;
    }
  }
  INFOF("Selected queue family: %d", selected_queue_family);
  /* TODO Present support (?) */
  const float queue_priorities{1.0f};
  VkDeviceQueueCreateInfo queue_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = selected_queue_family,
      .queueCount = 1,
      .pQueuePriorities = &queue_priorities};

  const std::vector<const char *> device_extensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkPhysicalDeviceVulkan12Features vulkan_12_features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .descriptorIndexing = true,
      .shaderSampledImageArrayNonUniformIndexing = true,
      .descriptorBindingVariableDescriptorCount = true,
      .runtimeDescriptorArray = true,
      .bufferDeviceAddress = true};
  VkPhysicalDeviceVulkan13Features vulkan_13_features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = &vulkan_12_features,
      .synchronization2 = true,
      .dynamicRendering = true,
  };
  VkPhysicalDeviceFeatures vulkan_10_features{.samplerAnisotropy = VK_TRUE};

  /* Logical device */
  VkDeviceCreateInfo device_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &vulkan_13_features,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_create_info,
      .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
      .ppEnabledExtensionNames = device_extensions.data(),
      .pEnabledFeatures = &vulkan_10_features};
  VkDevice device{VK_NULL_HANDLE};
  if (vkCreateDevice(devices[selected_physical_device], &device_create_info,
                     nullptr, &device) != VK_SUCCESS)
    throw std::runtime_error("Unable to create logical device");
  INFO("Created logical device")

  VkQueue queue{VK_NULL_HANDLE};
  vkGetDeviceQueue(device, selected_queue_family, 0, &queue);

  /* Set up VMA */
  VmaVulkanFunctions vkFunctions{.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                                 .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
                                 .vkCreateImage = vkCreateImage};
  VmaAllocatorCreateInfo allocator_create_info{
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = devices[selected_physical_device],
      .device = device,
      .pVulkanFunctions = &vkFunctions,
      .instance = instance};
  VmaAllocator allocator{VK_NULL_HANDLE};
  if (vmaCreateAllocator(&allocator_create_info, &allocator) != VK_SUCCESS)
    throw std::runtime_error("Unable to create allocator");

  /* Surface */
  VkSurfaceKHR surface{VK_NULL_HANDLE};
  if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS)
    throw std::runtime_error("Unable to create Vulkan surface");
  /* Verify surface support */
  VkBool32 supported{VK_FALSE};
  vkGetPhysicalDeviceSurfaceSupportKHR(devices[selected_physical_device],
                                       selected_queue_family, surface,
                                       &supported);
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
