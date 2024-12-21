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
  TextureManager &textureManager;
  DescriptorManager(BufferManager &bufferManager, TextureManager &textureManager) : bufferManager(bufferManager), textureManager(textureManager)
  {
  }
  ~DescriptorManager()
  {
  }
  void createDescriptorSetLayout(VkDevice device);
  void createDescriptorPool(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);
  void createDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);
  void addDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);
  void cleanup(VkDevice device);
};