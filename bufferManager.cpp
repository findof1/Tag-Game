#include "bufferManager.hpp"
#include "utils.h"
#include <stdexcept>
#include <chrono>

void BufferManager::createUniformBuffers(int MAX_FRAMES_IN_FLIGHT, VkDevice device, VkPhysicalDevice physicalDevice, int count)
{
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  uniformBuffers.resize(uniformBuffers.size() + MAX_FRAMES_IN_FLIGHT * count);
  uniformBuffersMemory.resize(uniformBuffersMemory.size() + MAX_FRAMES_IN_FLIGHT * count);
  uniformBuffersMapped.resize(uniformBuffersMapped.size() + MAX_FRAMES_IN_FLIGHT * count);

  for (size_t i = uniformBuffers.size() - MAX_FRAMES_IN_FLIGHT * count; i < uniformBuffers.size(); i++)
  {
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i], device, physicalDevice);

    vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
  }
}

void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDevice device, VkPhysicalDevice physicalDevice)
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void BufferManager::createIndexBuffer(std::vector<uint32_t> inputIndices, int targetBuffer, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  if (indexBuffers.size() <= targetBuffer)
  {
    indexBuffers.resize(targetBuffer + 1, VK_NULL_HANDLE);
    indexBufferMemory.resize(targetBuffer + 1, VK_NULL_HANDLE);
  }

  VkDeviceSize bufferSize = sizeof(inputIndices[0]) * inputIndices.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, inputIndices.data(), (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffers[targetBuffer], indexBufferMemory[targetBuffer], device, physicalDevice);

  copyBuffer(stagingBuffer, indexBuffers[targetBuffer], bufferSize, device, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void BufferManager::createVertexBuffer(std::vector<Vertex> verts, int targetBuffer, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  if (vertexBuffers.size() <= targetBuffer)
  {
    vertexBuffers.resize(targetBuffer + 1, VK_NULL_HANDLE);
    vertexBufferMemory.resize(targetBuffer + 1, VK_NULL_HANDLE);
  }

  VkDeviceSize bufferSize = sizeof(verts[0]) * verts.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, verts.data(), (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffers[targetBuffer], vertexBufferMemory[targetBuffer], device, physicalDevice);

  copyBuffer(stagingBuffer, vertexBuffers[targetBuffer], bufferSize, device, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void BufferManager::freeVertexBuffer(int index, VkDevice device)
{
  if (index < 0 || index >= vertexBuffers.size())
  {
    return;
  }

  if (vertexBuffers[index] != VK_NULL_HANDLE)
  {
    vkDestroyBuffer(device, vertexBuffers[index], nullptr);
  }

  if (vertexBufferMemory[index] != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, vertexBufferMemory[index], nullptr);
  }

  vertexBuffers.erase(vertexBuffers.begin() + index);
  vertexBufferMemory.erase(vertexBufferMemory.begin() + index);
}

void BufferManager::freeIndexBuffer(int index, VkDevice device)
{
  if (index < 0 || index >= indexBuffers.size())
  {
    return;
  }

  if (indexBuffers[index] != VK_NULL_HANDLE)
  {
    vkDestroyBuffer(device, indexBuffers[index], nullptr);
  }

  if (indexBufferMemory[index] != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, indexBufferMemory[index], nullptr);
  }

  indexBuffers.erase(indexBuffers.begin() + index);
  indexBufferMemory.erase(indexBufferMemory.begin() + index);
}

void BufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer, device, commandPool, graphicsQueue);
}

void BufferManager::cleanup(VkDevice device)
{
  for (auto &uniformBuffer : uniformBuffers)
  {
    vkDestroyBuffer(device, uniformBuffer, nullptr);
  }

  for (auto &uniformBufferMemory : uniformBuffersMemory)
  {
    vkFreeMemory(device, uniformBufferMemory, nullptr);
  }

  for (auto &indexBuffer : indexBuffers)
  {
    if (indexBuffer != VK_NULL_HANDLE)
    {
      vkDestroyBuffer(device, indexBuffer, nullptr);
      indexBuffer = VK_NULL_HANDLE;
    }
  }
  for (auto &indexBufferMem : indexBufferMemory)
  {
    if (indexBufferMem != VK_NULL_HANDLE)
    {
      vkFreeMemory(device, indexBufferMem, nullptr);
      indexBufferMem = VK_NULL_HANDLE;
    }
  }

  for (auto &vertexBuffer : vertexBuffers)
  {
    if (vertexBuffer != VK_NULL_HANDLE)
    {
      vkDestroyBuffer(device, vertexBuffer, nullptr);
      vertexBuffer = VK_NULL_HANDLE;
    }
  }
  for (auto &vertexBufferMem : vertexBufferMemory)
  {
    if (vertexBufferMem != VK_NULL_HANDLE)
    {
      vkFreeMemory(device, vertexBufferMem, nullptr);
      vertexBufferMem = VK_NULL_HANDLE;
    }
  }
}

void BufferManager::updateUniformBuffer(uint32_t currentImage, glm::mat4 transformation, glm::mat4 view, glm::mat4 proj)
{
  UniformBufferObject ubo{};

  ubo.model = transformation;
  ubo.view = view;

  ubo.proj = proj;

  ubo.proj[1][1] *= -1;

  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}