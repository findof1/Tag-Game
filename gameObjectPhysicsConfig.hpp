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
  glm::vec3 boxColliderSize;
  PhysicsConfig() : boxColliderSize(-1) {}
};