#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"
#include <btBulletDynamicsCommon.h>
#include "textureManager.hpp"

class Renderer;
class TextureManager;
struct PhysicsConfig;
class GameObject
{
public:
  glm::vec3 pos;
  glm::vec3 rotationZYX; // degrees
  glm::vec3 scale;
  int id;
  PhysicsConfig &config;

  btCollisionShape *collisionShape = nullptr;
  btCollisionObject *collisionObject = nullptr;
  btMotionState *motionState = nullptr;
  btRigidBody *rigidBody = nullptr;
  btScalar mass;
  TextureManager textureManager;

  GameObject(Renderer &renderer, int id, PhysicsConfig &config, const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &rotationZYX, btScalar mass, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  ~GameObject() {}

  void draw(Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer);
  void loadModel(const std::string MODEL_PATH);
  void setVerticesAndIndices(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  void initGraphics(Renderer &renderer, std::string texturePath);

  void initPhysics(btDiscreteDynamicsWorld *dynamicsWorld);
  void updatePhysics();
  void cleanupPhysics(btDiscreteDynamicsWorld *dynamicsWorld);

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};