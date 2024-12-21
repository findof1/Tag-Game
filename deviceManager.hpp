#pragma once
#include <vulkan/vulkan.h>
#include <vector>
class SwapchainManager;
class DeviceManager
{
public:
  VkDevice device;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  SwapchainManager &swapchainManager;
  DeviceManager(SwapchainManager &swapchainManager) : swapchainManager(swapchainManager)
  {
  }
  ~DeviceManager()
  {
  }

  void createLogicalDevice(bool enableValidationLayers, const std::vector<const char *> &deviceExtensions, const std::vector<const char *> &validationLayers, VkQueue *presentQueue, VkQueue *graphicsQueue);
  void pickPhysicalDevice(VkInstance instance, const std::vector<const char *> &deviceExtensions);

private:
  bool isDeviceSuitable(VkPhysicalDevice device, const std::vector<const char *> &deviceExtensions);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &deviceExtensions);
};
