#pragma once
#include <vulkan/vulkan.h>
#include <memory>
class BufferManager;
class Renderer;
class TextureManager
{
public:
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
  BufferManager &bufferManager;
  Renderer &renderer;
  TextureManager(BufferManager &bufferManager, Renderer &renderer) : bufferManager(bufferManager), renderer(renderer)
  {
  }
  ~TextureManager()
  {
  }
  void createTextureImageView(VkDevice device);
  void createTextureImage(std::string texturePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice);

  void updateTexture(std::string newTexturePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

  void cleanup(VkDevice device);
};