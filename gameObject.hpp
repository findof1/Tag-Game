#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"

class BufferManager;
class DescriptorManager;
class GameObject
{
public:
  BufferManager &bufferManager;
  DescriptorManager &descriptorManager;
  glm::vec3 pos;
  float pitch;
  float yaw;
  glm::vec3 scale;
  int id;

  GameObject(BufferManager &bufferManager, DescriptorManager &descriptorManager, int id, int MAX_FRAMES_IN_FLIGHT, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const glm::vec3 &pos, const glm::vec3 &scale, float yaw, float pitch, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  ~GameObject() {}
  void draw(int currentFrame, int MAX_FRAMES_IN_FLIGHT, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

private:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};