#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"
class BufferManager
{
public:
  BufferManager()
  {
  }
  ~BufferManager()
  {
  }
  std::vector<VkBuffer> vertexBuffers;
  std::vector<VkDeviceMemory> vertexBufferMemory;

  std::vector<VkBuffer> indexBuffers;
  std::vector<VkDeviceMemory> indexBufferMemory;

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void *> uniformBuffersMapped;

  void createUniformBuffers(int MAX_FRAMES_IN_FLIGHT, VkDevice device, VkPhysicalDevice physicalDevice, int count);
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDevice device, VkPhysicalDevice physicalDevice);

  void freeVertexBuffer(int index, VkDevice device);
  void freeIndexBuffer(int index, VkDevice device);

  void createVertexBuffer(std::vector<Vertex> verts, int targetBuffer, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void createIndexBuffer(std::vector<uint32_t> inputIndices, int targetBuffer, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);
  void cleanup(VkDevice device);
  void updateUniformBuffer(uint32_t currentImage, glm::mat4 transformation, glm::mat4 view, glm::mat4 proj);
};