#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <stdexcept>
#include <GLFW/glfw3.h>

struct SwapChainSupportDetails;
class SwapchainManager
{
public:
  GLFWwindow *window;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  SwapchainManager(GLFWwindow *window) : window(window)
  {
  }
  SwapchainManager() : window(nullptr)
  {
  }
  void setWindow(GLFWwindow *windowPtr)
  {
    window = windowPtr;
  }
  ~SwapchainManager()
  {
  }

  void createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice);

  void createSurface(VkInstance instance);

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  void cleanupSwapChain(VkDevice device);
  void cleanupDepthImages(VkDevice device);

  void createImageViews(VkDevice device);

  void createFramebuffers(VkDevice device, VkRenderPass renderPass);

  void createDepthResources(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
};