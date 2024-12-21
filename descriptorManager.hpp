#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class BufferManager;
class TextureManager;
class DescriptorManager
{
public:
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  BufferManager &bufferManager;
  DescriptorManager(BufferManager &bufferManager) : bufferManager(bufferManager)
  {
  }
  ~DescriptorManager()
  {
  }
  void createDescriptorSetLayout(VkDevice device);
  void createDescriptorPool(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);
  void createDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, TextureManager &textureManager);
  void addDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, TextureManager &textureManager);
  void cleanup(VkDevice device);
};