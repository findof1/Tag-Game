#include "gameObject.hpp"
#include "bufferManager.hpp"
#include "descriptorManager.hpp"
#include "textureManager.hpp"
#include "renderer.hpp"
#include <vulkan/vulkan.h>
#include <tiny_obj_loader.h>

GameObject::GameObject(Renderer &renderer, int id, const glm::vec3 &pos, const glm::vec3 &scale, float yaw, float pitch, std::vector<Vertex> vertices, std::vector<uint32_t> indices) : id(id), pos(pos), scale(scale), yaw(yaw), pitch(pitch), vertices(vertices), indices(indices)
{
}

void GameObject::initGraphics(Renderer &renderer, TextureManager &textureManager)
{
  renderer.bufferManager.createVertexBuffer(vertices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  renderer.bufferManager.createIndexBuffer(indices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  renderer.bufferManager.createUniformBuffers(renderer.MAX_FRAMES_IN_FLIGHT, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, 1);

  renderer.descriptorManager.addDescriptorSets(renderer.deviceManager.device, renderer.MAX_FRAMES_IN_FLIGHT, 1, textureManager);
}

/*
GameObject::GameObject(BufferManager &bufferManager, DescriptorManager &descriptorManager, int id, int MAX_FRAMES_IN_FLIGHT, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const glm::vec3 &pos, const glm::vec3 &scale, float yaw, float pitch, std::vector<Vertex> vertices, std::vector<uint32_t> indices) : bufferManager(bufferManager), descriptorManager(descriptorManager), id(id), pos(pos), scale(scale), yaw(yaw), pitch(pitch), vertices(vertices), indices(indices)
{
  bufferManager.createVertexBuffer(vertices, id, device, physicalDevice, commandPool, graphicsQueue);

  bufferManager.createIndexBuffer(indices, id, device, physicalDevice, commandPool, graphicsQueue);

  bufferManager.createUniformBuffers(MAX_FRAMES_IN_FLIGHT, device, physicalDevice, 1);

  descriptorManager.addDescriptorSets(device, MAX_FRAMES_IN_FLIGHT, 1);
}
*/

void GameObject::draw(Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer)
{
  glm::mat4 transformation = glm::mat4(1.0f);
  transformation = glm::translate(transformation, pos);
  transformation = glm::rotate(transformation, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
  transformation = glm::rotate(transformation, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
  transformation = glm::scale(transformation, scale);

  VkBuffer vertexBuffersArray[] = {renderer->bufferManager.vertexBuffers[id]};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersArray, offsets);

  vkCmdBindIndexBuffer(commandBuffer, renderer->bufferManager.indexBuffers[id], 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.pipelineLayout, 0, 1, &renderer->descriptorManager.descriptorSets[currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT], 0, nullptr);

  renderer->bufferManager.updateUniformBuffer(currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT, transformation, view, projectionMatrix);

  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void GameObject::loadModel(const std::string MODEL_PATH)
{
  vertices.clear();
  indices.clear();

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  bool ret = tinyobj::LoadObj(
      &attrib,
      &shapes,
      &materials,
      &err,
      MODEL_PATH.c_str());

  if (!ret)
  {
    throw std::runtime_error(err);
  }

  for (const auto &shape : shapes)
  {
    for (const auto &index : shape.mesh.indices)
    {
      Vertex vertex{};

      vertex.pos = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2]};

      vertex.texPos = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

      vertex.color = {1.0f, 1.0f, 1.0f};

      vertices.push_back(vertex);
      indices.push_back(indices.size());
    }
  }
}