#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"

class Renderer;
class TextureManager;
class GameObject
{
public:
  glm::vec3 pos;
  float pitch;
  float yaw;
  glm::vec3 scale;
  int id;

  GameObject(Renderer &renderer, int id, const glm::vec3 &pos, const glm::vec3 &scale, float yaw, float pitch, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  ~GameObject() {}
  void draw(Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer);
  void loadModel(const std::string MODEL_PATH);
  void setVerticesAndIndices(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  void initGraphics(Renderer &renderer, TextureManager &textureManager);

private:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};