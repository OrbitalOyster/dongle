#include "Core.hpp"
#include "debug.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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
  uint32_t selected_physical_device_index{0};
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
        selected_physical_device_index = i;
    }
  selected_physical_device = devices[selected_physical_device_index];
  /* Find queue family */
  uint32_t queue_family_count{0};
  vkGetPhysicalDeviceQueueFamilyProperties(selected_physical_device,
                                           &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      selected_physical_device, &queue_family_count, queue_families.data());
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
  if (vkCreateDevice(devices[selected_physical_device_index],
                     &device_create_info, nullptr, &device) != VK_SUCCESS)
    throw std::runtime_error("Unable to create logical device");
  INFO("Created logical device")

  /* Device queue */
  vkGetDeviceQueue(device, selected_queue_family, 0, &queue);

  /* Set up VMA */
  VmaVulkanFunctions vkFunctions{.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                                 .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
                                 .vkCreateImage = vkCreateImage};
  VmaAllocatorCreateInfo allocator_create_info{
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = devices[selected_physical_device_index],
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
  vkGetPhysicalDeviceSurfaceSupportKHR(devices[selected_physical_device_index],
                                       selected_queue_family, surface,
                                       &supported);

  VkSurfaceCapabilitiesKHR surface_capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selected_physical_device, surface,
                                            &surface_capabilities);

  /* Swapchain extent */
  VkExtent2D swapchainExtent{surface_capabilities.currentExtent};
  if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
    swapchainExtent = {.width = static_cast<uint32_t>(window_width),
                       .height = static_cast<uint32_t>(window_width)};
  }
  const VkFormat image_format{VK_FORMAT_B8G8R8A8_SRGB};
  VkSwapchainCreateInfoKHR swapchain_create_info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = surface_capabilities.minImageCount,
      .imageFormat = image_format,
      .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
      .imageExtent{.width = swapchainExtent.width,
                   .height = swapchainExtent.height},
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR};
  VkSwapchainKHR swapchain{VK_NULL_HANDLE};
  if (vkCreateSwapchainKHR(device, &swapchain_create_info, VK_NULL_HANDLE,
                           &swapchain) != VK_SUCCESS)
    throw std::runtime_error("Unable to create swapchain");
  INFO("Created swapchain")
  /* Swapchain images */
  uint32_t image_count{0};
  if (vkGetSwapchainImagesKHR(device, swapchain, &image_count,
                              VK_NULL_HANDLE) != VK_SUCCESS)
    throw std::runtime_error("Unable to get swapchain images");
  INFOF("Created %d swapchain images", image_count)
  std::vector<VkImage> swapchain_images;
  swapchain_images.resize(image_count);
  std::vector<VkImageView> swapchain_image_views;
  if (vkGetSwapchainImagesKHR(device, swapchain, &image_count,
                              swapchain_images.data()) != VK_SUCCESS)
    throw std::runtime_error("Unable to get swapchain image views");
  swapchain_image_views.resize(image_count);
  INFOF("Created %d swapchain image views", image_count)
  /* Depth */
  std::vector<VkFormat> depth_format_list{VK_FORMAT_D32_SFLOAT_S8_UINT,
                                          VK_FORMAT_D24_UNORM_S8_UINT};
  VkFormat depth_format{VK_FORMAT_UNDEFINED};
  for (VkFormat &format : depth_format_list) {
    VkFormatProperties2 format_properties{
        .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
    vkGetPhysicalDeviceFormatProperties2(selected_physical_device, format,
                                         &format_properties);
    if (format_properties.formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      depth_format = format;
      break;
    }
  }
  VkImageCreateInfo depth_image_create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = depth_format,
      .extent{.width = static_cast<uint32_t>(window_width),
              .height = static_cast<uint32_t>(window_height),
              .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo allocCI{
      .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO};
  VkImage depth_image;
  VmaAllocation depthImageAllocation;
  if (vmaCreateImage(allocator, &depth_image_create_info, &allocCI,
                     &depth_image, &depthImageAllocation,
                     VK_NULL_HANDLE) != VK_SUCCESS)
    throw std::runtime_error("Unable to create depth image");
  INFO("Created depth image")

  VkImageViewCreateInfo depth_view_create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = depth_image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = depth_format,
      .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                        .levelCount = 1,
                        .layerCount = 1}};
  VkImageView depth_image_view;
  if (vkCreateImageView(device, &depth_view_create_info, nullptr,
                        &depth_image_view) != VK_SUCCESS)
    throw std::runtime_error("Unable to create depth image view");
  INFO("Created depth image view")

  /* Mesh */
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr,
                        "assets/suzanne.obj"))
    throw std::runtime_error("Unable to load mesh");
  INFO("Loaded mesh")
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
