#include "gameObject.hpp"
#include "bufferManager.hpp"
#include "descriptorManager.hpp"
#include "textureManager.hpp"
#include "renderer.hpp"
#include <vulkan/vulkan.h>
#include <tiny_obj_loader.h>
#include <glm/gtc/quaternion.hpp>
#include "gameObjectPhysicsConfig.hpp"

GameObject::GameObject(Renderer &renderer, int id, PhysicsConfig &config, const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &rotationZYX, std::vector<Vertex> vertices, std::vector<uint32_t> indices, GameObjectTags tag) : id(id), config(config), pos(pos), scale(scale), rotationZYX(rotationZYX), vertices(vertices), indices(indices), textureManager(renderer.bufferManager), tag(tag)
{
}

void GameObject::initGraphics(Renderer &renderer, std::string texturePath)
{
  textureManager.createTextureImage(texturePath, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
  textureManager.createTextureImageView(renderer.deviceManager.device);
  textureManager.createTextureSampler(renderer.deviceManager.device, renderer.deviceManager.physicalDevice);

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
  transformation = glm::rotate(transformation, glm::radians(rotationZYX.x), glm::vec3(0.0f, 0.0f, 1.0f));
  transformation = glm::rotate(transformation, glm::radians(rotationZYX.y), glm::vec3(0.0f, 1.0f, 0.0f));
  transformation = glm::rotate(transformation, glm::radians(rotationZYX.z), glm::vec3(1.0f, 0.0f, 0.0f));
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

void GameObject::initPhysics(btDiscreteDynamicsWorld *dynamicsWorld)
{
  if (config.collider == ColliderType::None)
  {
    return;
  }

  if (config.collider == ColliderType::Box && config.boxColliderSize == glm::vec3(-1))
  {

    glm::vec3 minCorner(std::numeric_limits<float>::max());
    glm::vec3 maxCorner(std::numeric_limits<float>::lowest());

    for (const auto &vertex : vertices)
    {
      glm::vec3 scaledVertex = vertex.pos * scale;
      minCorner = glm::min(minCorner, scaledVertex);
      maxCorner = glm::max(maxCorner, scaledVertex);
    }

    glm::vec3 size = maxCorner - minCorner;

    collisionShape = new btBoxShape(btVector3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f));
  }
  else if (config.collider == ColliderType::Mesh)
  {
    btTriangleMesh *triangleMesh = new btTriangleMesh();
    for (size_t i = 0; i < indices.size(); i += 3)
    {
      glm::vec3 v0 = vertices[indices[i]].pos * scale;
      glm::vec3 v1 = vertices[indices[i + 1]].pos * scale;
      glm::vec3 v2 = vertices[indices[i + 2]].pos * scale;

      triangleMesh->addTriangle(btVector3(v0.x, v0.y, v0.z), btVector3(v1.x, v1.y, v1.z), btVector3(v2.x, v2.y, v2.z));
    }
    collisionShape = new btBvhTriangleMeshShape(triangleMesh, true);

    btCompoundShape *compoundShape = new btCompoundShape();
    btTransform localTransform;
    localTransform.setIdentity();
    compoundShape->addChildShape(localTransform, collisionShape);

    collisionShape = compoundShape;
    collisionShape->setMargin(config.meshColliderMargin);
  }
  else
  {
    collisionShape = new btBoxShape(btVector3(config.boxColliderSize.x, config.boxColliderSize.y, config.boxColliderSize.z));
  }

  if (!config.isRigidBody)
  {
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    collisionObject = new btCollisionObject();
    collisionObject->setCollisionShape(collisionShape);

    if (config.interactable == false)
    {
      collisionObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }

    collisionObject->setWorldTransform(transform);

    dynamicsWorld->addCollisionObject(collisionObject);
    return;
  }

  btTransform initialTransform;
  initialTransform.setIdentity();
  initialTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));
  initialTransform.setRotation(btQuaternion(glm::radians(rotationZYX.z), glm::radians(rotationZYX.y), glm::radians(rotationZYX.x)));
  motionState = new btDefaultMotionState(initialTransform);

  btVector3 inertia(0, 0, 0);
  collisionShape->calculateLocalInertia(config.mass, inertia);

  btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(config.mass, motionState, collisionShape, inertia);
  rigidBody = new btRigidBody(rigidBodyCI);

  int flags = rigidBody->getFlags();
  if (!config.canMove)
  {
    rigidBody->setMassProps(0, btVector3(0, 0, 0));
  }
  else
  {
    rigidBody->setMassProps(config.mass, inertia);
  }

  if (!config.canRotateX || !config.canRotateY || !config.canRotateZ)
  {
    btVector3 angularFactor(1, 1, 1);
    if (!config.canRotateX)
      angularFactor.setX(0);
    if (!config.canRotateY)
      angularFactor.setY(0);
    if (!config.canRotateZ)
      angularFactor.setZ(0);
    rigidBody->setAngularFactor(angularFactor);
  }

  if (config.interactable == false)
  {
    rigidBody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
  }

  rigidBody->setGravity(btVector3(0, -20.f, 0));

  rigidBody->setFriction(config.friction);

  rigidBody->setDamping(config.linearDamping, config.angularDamping);

  rigidBody->setRestitution(config.restitution);

  rigidBody->setUserPointer((void *)this);

  rigidBody->activate();

  dynamicsWorld->addRigidBody(rigidBody);
}

void GameObject::updatePhysics()
{
  if (config.collider == ColliderType::None)
  {
    return;
  }

  if (rigidBody && rigidBody->getMotionState())
  {
    btTransform transform;
    rigidBody->getMotionState()->getWorldTransform(transform);

    pos.x = transform.getOrigin().getX();
    pos.y = transform.getOrigin().getY();
    pos.z = transform.getOrigin().getZ();

    if (config.canRotateX || config.canRotateY || config.canRotateZ)
    {
      btQuaternion rotation = transform.getRotation();
      btVector3 eulerAngles;
      rotation.getEulerZYX(eulerAngles[0], eulerAngles[1], eulerAngles[2]);

      if (config.canRotateZ)
        rotationZYX.x = glm::degrees(eulerAngles[0]);
      if (config.canRotateY)
        rotationZYX.y = glm::degrees(eulerAngles[1]);
      if (config.canRotateX)
        rotationZYX.z = glm::degrees(eulerAngles[2]);
    }
  }
  else if (collisionObject)
  {
    const btTransform &transform = collisionObject->getWorldTransform();
    pos.x = transform.getOrigin().getX();
    pos.y = transform.getOrigin().getY();
    pos.z = transform.getOrigin().getZ();
  }
}

void GameObject::cleanupPhysics(btDiscreteDynamicsWorld *dynamicsWorld)
{
  if (config.collider == ColliderType::None)
  {
    return;
  }

  if (rigidBody)
  {
    dynamicsWorld->removeRigidBody(rigidBody);
    delete rigidBody;
    rigidBody = nullptr;
  }

  if (motionState)
  {
    delete motionState;
    motionState = nullptr;
  }

  if (collisionObject)
  {
    dynamicsWorld->removeCollisionObject(collisionObject);
    delete collisionObject;
    collisionObject = nullptr;
  }

  if (collisionShape)
  {
    delete collisionShape;
    collisionShape = nullptr;
  }
}

void GameObject::setScale(const glm::vec3 &newScale)
{
  scale = newScale;

  if (config.collider != ColliderType::None && collisionShape)
  {
    if (config.collider == ColliderType::Box)
    {
      glm::vec3 minCorner(std::numeric_limits<float>::max());
      glm::vec3 maxCorner(std::numeric_limits<float>::lowest());

      for (const auto &vertex : vertices)
      {
        glm::vec3 scaledVertex = vertex.pos * scale;
        minCorner = glm::min(minCorner, scaledVertex);
        maxCorner = glm::max(maxCorner, scaledVertex);
      }

      glm::vec3 size = maxCorner - minCorner;

      delete collisionShape;
      collisionShape = new btBoxShape(btVector3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f));

      if (rigidBody)
      {
        btVector3 inertia(0, 0, 0);
        collisionShape->calculateLocalInertia(config.mass, inertia);
        rigidBody->setCollisionShape(collisionShape);
        rigidBody->setMassProps(config.mass, inertia);
        rigidBody->activate();
      }
    }
    else if (config.collider == ColliderType::Mesh)
    {
      btTriangleMesh *triangleMesh = new btTriangleMesh();
      for (size_t i = 0; i < indices.size(); i += 3)
      {
        glm::vec3 v0 = vertices[indices[i]].pos * scale;
        glm::vec3 v1 = vertices[indices[i + 1]].pos * scale;
        glm::vec3 v2 = vertices[indices[i + 2]].pos * scale;

        triangleMesh->addTriangle(btVector3(v0.x, v0.y, v0.z), btVector3(v1.x, v1.y, v1.z), btVector3(v2.x, v2.y, v2.z));
      }

      delete collisionShape;
      collisionShape = new btBvhTriangleMeshShape(triangleMesh, true);

      if (rigidBody)
      {
        btVector3 inertia(0, 0, 0);
        collisionShape->calculateLocalInertia(config.mass, inertia);
        rigidBody->setCollisionShape(collisionShape);
        rigidBody->setMassProps(config.mass, inertia);
        rigidBody->activate();
      }
    }
  }
}