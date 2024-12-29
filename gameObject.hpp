#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"
#include <btBulletDynamicsCommon.h>
#include "textureManager.hpp"

class Renderer;
class TextureManager;
struct PhysicsConfig;

enum class GameObjectTags
{
  None,
  Ground,
  Player,
  SpeedPowerup,
  JumpPowerup
};

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
  TextureManager textureManager;
  GameObjectTags tag;

  GameObject(Renderer &renderer, int id, PhysicsConfig &config, const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &rotationZYX, std::vector<Vertex> vertices, std::vector<uint32_t> indices, GameObjectTags tag = GameObjectTags::None);
  ~GameObject() {}

  void draw(Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer);
  void loadModel(const std::string MODEL_PATH);
  void setVerticesAndIndices(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  void initGraphics(Renderer &renderer, std::string texturePath);

  void initPhysics(btDiscreteDynamicsWorld *dynamicsWorld);
  void updatePhysics();
  void cleanupPhysics(btDiscreteDynamicsWorld *dynamicsWorld);

  void setScale(const glm::vec3 &newScale);

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};