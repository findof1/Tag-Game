#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <btBulletDynamicsCommon.h>
#include "gameObject.hpp"

enum Camera_Movement
{
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
};

enum Camera_Types
{
  FlyCam,
  FirstPerson
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 20.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;

  float Yaw;
  float Pitch;

  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  Camera_Types type;

  Camera(Camera_Types type = Camera_Types::FlyCam, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), type(type)
  {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
  }

  glm::mat4 GetViewMatrix()
  {
    return glm::lookAt(Position, Position + Front, Up);
  }

  void ProcessKeyboard(Camera_Movement direction, float deltaTime)
  {
    float velocity = MovementSpeed * deltaTime;
    if (type == FlyCam)
    {
      if (direction == FORWARD)
        Position += Front * velocity;
      if (direction == BACKWARD)
        Position -= Front * velocity;
      if (direction == LEFT)
        Position -= Right * velocity;
      if (direction == RIGHT)
        Position += Right * velocity;
    }
  }

  void ProcessKeyboard(GLFWwindow *window, float deltaTime, GameObject &player, btDynamicsWorld *dynamicsWorld)
  {
    MovementSpeed = SPEED;
    float velocity = MovementSpeed * deltaTime;
    if (type == FlyCam)
    {
      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Position += Front * velocity;
      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= Front * velocity;
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Position -= Right * velocity;
      if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Position += Right * velocity;
    }
    if (type == FirstPerson)
    {
      if (player.rigidBody)
      {
        MovementSpeed = 100 * player.rigidBody->getMass();
        float velocity = MovementSpeed;
        btVector3 force(0, 0, 0);

        glm::vec3 forward = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
          force += btVector3(forward.x, 0.0f, forward.z);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
          force -= btVector3(forward.x, 0.0f, forward.z);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
          force -= btVector3(Right.x, 0.0f, Right.z);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
          force += btVector3(Right.x, 0.0f, Right.z);

        if (force == btVector3(0, 0, 0))
        {
          return;
        }

        force = force.normalize() * velocity;

        float playerHeight = 3.0f;
        btVector3 feetPosition = player.rigidBody->getCenterOfMassPosition() - btVector3(0, playerHeight / 2, 0);
        btVector3 headPosition = player.rigidBody->getCenterOfMassPosition() + btVector3(0, playerHeight / 2, 0);

        bool hasHit = false;
        for (float i = feetPosition.getY(); i <= headPosition.getY(); i += 1.f)
        {
          btVector3 position(feetPosition.getX(), i, feetPosition.getZ());
          btCollisionWorld::ClosestRayResultCallback rayCallback(position, position + force.normalized());
          dynamicsWorld->rayTest(position, position + force.normalized(), rayCallback);
          if (rayCallback.hasHit())
          {
            const btRigidBody *rigidBody = dynamic_cast<const btRigidBody *>(rayCallback.m_collisionObject);
            if (rigidBody)
            {
              btScalar mass = rigidBody->getMass();
              if (mass == 0)
              {
                hasHit = true;
                break;
              }
            }
          }
        }

        if (!hasHit)
        {
          player.rigidBody->applyCentralForce(force);
          player.rigidBody->activate();
        }
      }
      else
      {
        std::cerr << "No GameObject passed to camera's ProcessKeyboard" << std::endl;
      }
    }
  }

  void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
  {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
      if (Pitch > 89.0f)
        Pitch = 89.0f;
      if (Pitch < -89.0f)
        Pitch = -89.0f;
    }

    updateCameraVectors();
  }

  void ProcessMouseScroll(float yoffset)
  {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
      Zoom = 1.0f;
    if (Zoom > 90.0f)
      Zoom = 90.0f;
  }

private:
  void updateCameraVectors()
  {

    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
  }
};
#endif