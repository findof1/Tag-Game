#include "gameObject.hpp"
#include "bufferManager.hpp"
#include "descriptorManager.hpp"
#include <vulkan/vulkan.h>

GameObject::GameObject(BufferManager &bufferManager, DescriptorManager &descriptorManager, int id, int MAX_FRAMES_IN_FLIGHT, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const glm::vec3 &pos, const glm::vec3 &scale, float yaw, float pitch, std::vector<Vertex> vertices, std::vector<uint32_t> indices) : bufferManager(bufferManager), descriptorManager(descriptorManager), id(id), pos(pos), scale(scale), yaw(yaw), pitch(pitch), vertices(vertices), indices(indices)
{
  bufferManager.createVertexBuffer(vertices, id, device, physicalDevice, commandPool, graphicsQueue);

  bufferManager.createIndexBuffer(indices, id, device, physicalDevice, commandPool, graphicsQueue);

  bufferManager.createUniformBuffers(MAX_FRAMES_IN_FLIGHT, device, physicalDevice, 1);

  descriptorManager.addDescriptorSets(device, MAX_FRAMES_IN_FLIGHT, 1);
}

void GameObject::draw(int currentFrame, int MAX_FRAMES_IN_FLIGHT, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
  glm::mat4 transformation = glm::mat4(1.0f);
  transformation = glm::translate(transformation, pos);
  transformation = glm::rotate(transformation, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));   // Yaw around Y-axis
  transformation = glm::rotate(transformation, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch around X-axis
  transformation = glm::scale(transformation, scale);

  VkBuffer vertexBuffersArray[] = {bufferManager.vertexBuffers[id]};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersArray, offsets);

  vkCmdBindIndexBuffer(commandBuffer, bufferManager.indexBuffers[id], 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorManager.descriptorSets[currentFrame + id * MAX_FRAMES_IN_FLIGHT], 0, nullptr);

  bufferManager.updateUniformBuffer(currentFrame + id * MAX_FRAMES_IN_FLIGHT, transformation, view, projectionMatrix);

  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}