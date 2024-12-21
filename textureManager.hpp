#pragma once
#include <vulkan/vulkan.h>
#include <memory>
class BufferManager;
class TextureManager
{
public:
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
  BufferManager &bufferManager;
  TextureManager(BufferManager &bufferManager) : bufferManager(bufferManager)
  {
  }
  ~TextureManager()
  {
  }
  void createTextureImageView(VkDevice device);
  void createTextureImage(std::string texturePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice);
  void cleanup(VkDevice device);
};