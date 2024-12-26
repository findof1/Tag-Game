#include <string>
#include <iostream>
#include <glm/glm.hpp>

enum class ColliderType
{
  None,
  Box,
  Mesh
};

struct PhysicsConfig
{
  bool isRigidBody;
  ColliderType collider;
  bool interactable = true;
  bool canMove = true;
  bool canRotateX = true;
  bool canRotateY = true;
  bool canRotateZ = true;
  float mass = 1;
  float friction = 0.5;
  float linearDamping = 0.0;
  float angularDamping = 0.0;
  glm::vec3 boxColliderSize;
  PhysicsConfig() : boxColliderSize(-1) {}
};