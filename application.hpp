#include "renderer.hpp"
#include <btBulletDynamicsCommon.h>
#include "gameObjectPhysicsConfig.hpp"
#include "camera.h"
#include <chrono>
#include <algorithm>
#include <cmath>
#include "playerCollisionCallback.hpp"
#include "socketManager.hpp"

#define DEFAULT_DAMPING_FACTOR 10
#define DEFAULT_MAX_SPEED 12.0f

enum MovementState
{
  Ground,
  Air,
  FallingFast
};

const std::vector<Vertex> cubeVertices = {
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 0
    {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // 1
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 2
    {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},  // 3

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 4
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // 5
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 6
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},  // 7

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 8
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // 9
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 10
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},  // 11

    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 12
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // 13
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 14
    {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},  // 15

    {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 16
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // 17
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 18
    {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},  // 19

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 20
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // 21
    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 22
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}   // 23
};

const std::vector<uint32_t> cubeIndices = {
    0, 1, 2, 2, 3, 0,
    4, 7, 6, 6, 5, 4,
    8, 11, 10, 10, 9, 8,
    12, 13, 14, 14, 15, 12,
    16, 19, 18, 18, 17, 16,
    20, 21, 22, 22, 23, 20};

const std::vector<uint32_t> skyBoxIndices = {
    0, 2, 1, 2, 0, 3,
    4, 6, 7, 6, 4, 5,
    8, 10, 11, 10, 8, 9,
    12, 14, 13, 14, 12, 15,
    16, 18, 19, 18, 16, 17,
    20, 22, 21, 22, 20, 23};

class Application
{
public:
  Renderer renderer;
  SocketManager socketManager;
  Camera camera;
  GLFWwindow *window;
  btDiscreteDynamicsWorld *dynamicsWorld;

  std::mutex objectsMutex;

  int nextGameObjectId = 0;

  int taggedPlayer = -1;

  uint32_t WIDTH = 1600;
  uint32_t HEIGHT = 1200;
  bool spacePressed = false;
  bool shiftPressed = false;
  bool controlPressed = false;
  bool grounded;

  std::chrono::high_resolution_clock::time_point lastTime;
  int frameCount = 0;
  bool firstMouse = true;
  float lastX = WIDTH / 2, lastY = HEIGHT / 2;
  float deltaTime = 0.0f;
  float lastFrame = 0.0f;
  std::unordered_map<int, GameObject> objects;

  std::chrono::system_clock::time_point currentTime;
  std::chrono::high_resolution_clock::time_point lastBroadcast = std::chrono::high_resolution_clock::now();

  long lastDashTime = 0;
  long lastSpeedBoostTime = 0;
  long lastJumpBoostTime = 0;

  MovementState movementState = MovementState::Air;
  float speedMultiplier = 1;
  float jumpMultiplier = 1;

  float dampingFactor = DEFAULT_DAMPING_FACTOR;
  float maxSpeed = DEFAULT_MAX_SPEED;
  float wallJumpCount;
  float dashCount;

  Application() : camera(FirstPerson), renderer(camera, WIDTH, HEIGHT), socketManager(this)
  {
    socketManager.init();
  }

  void run()
  {
    initWindow();
    renderer.initVulkan();

    PhysicsConfig config0;
    config0.collider = ColliderType::Mesh;
    config0.isRigidBody = true;
    config0.mass = 74;

    PhysicsConfig config1;
    config1.collider = ColliderType::Box;
    config1.isRigidBody = true;
    config1.canMove = false;
    config1.canRotateX = false;
    config1.canRotateY = false;
    config1.canRotateZ = false;
    config1.friction = 2;
    config1.mass = 0;
    config1.restitution = 0.1;

    PhysicsConfig config2;
    config2.collider = ColliderType::Box;
    config2.isRigidBody = true;
    config2.mass = 1;

    PhysicsConfig config3;
    config3.collider = ColliderType::Box;
    config3.isRigidBody = true;
    config3.mass = 1;

    PhysicsConfig config4;
    config4.collider = ColliderType::Box;
    config4.canMove = false;
    config4.interactable = false;
    config4.mass = 1;

    PhysicsConfig config5;
    config5.collider = ColliderType::None;
    config5.mass = 0;

    PhysicsConfig config6;
    config6.collider = ColliderType::Box;
    config6.isRigidBody = true;
    config6.canRotateX = false;
    config6.canRotateY = false;
    config6.canRotateZ = false;
    config6.friction = 0.8;
    config6.linearDamping = 0.1;
    config6.mass = 30;
    config6.restitution = 0.8;

    PhysicsConfig config7;
    config7.collider = ColliderType::Mesh;
    config7.isRigidBody = true;
    config7.canMove = false;
    config7.canRotateX = false;
    config7.canRotateY = false;
    config7.canRotateZ = false;
    config7.friction = 1;
    config7.mass = 0;
    config7.meshColliderMargin = 0.5;
    config7.restitution = 0.1;

    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config0, glm::vec3(0, 5, 0), glm::vec3(0.1, 0.1, 0.1), glm::vec3(10, 40, 50), {}, {}));
    objects.at(nextGameObjectId).loadModel("models/couch/couch1.obj");
    objects.at(nextGameObjectId).initGraphics(renderer, "models/couch/gray.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    nextGameObjectId++;

    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config1, glm::vec3(0, 0, 0), glm::vec3(50, 2, 50), glm::vec3(0, 0, 0), cubeVertices, cubeIndices, GameObjectTags::Ground));
    objects.at(nextGameObjectId).initGraphics(renderer, "textures/wood.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    nextGameObjectId++;

    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config2, glm::vec3(0, 30, 0), glm::vec3(1, 1, 1), glm::vec3(0, 30, 45), cubeVertices, cubeIndices));
    objects.at(nextGameObjectId).initGraphics(renderer, "textures/metal.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    nextGameObjectId++;

    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config3, glm::vec3(5.1, 15, 0), glm::vec3(2, 1, 1), glm::vec3(0, 10, 45), cubeVertices, cubeIndices));
    objects.at(nextGameObjectId).initGraphics(renderer, "textures/wall.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    nextGameObjectId++;

    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config4, glm::vec3(5, 5, 0), glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), cubeVertices, cubeIndices, GameObjectTags::SpeedPowerup));
    objects.at(nextGameObjectId).initGraphics(renderer, "textures/wood.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    nextGameObjectId++;

    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config5, glm::vec3(0, 0, 0), glm::vec3(500, 500, 500), glm::vec3(0, 0, 0), cubeVertices, skyBoxIndices));
    objects.at(nextGameObjectId).initGraphics(renderer, "textures/sky.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    nextGameObjectId++;

    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config6, glm::vec3(-5, 5, 0), glm::vec3(1, 3, 1), glm::vec3(0, 0, 0), cubeVertices, cubeIndices, GameObjectTags::Player));
    objects.at(nextGameObjectId).initGraphics(renderer, "textures/wall.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    nextGameObjectId++;

    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config7, glm::vec3(0, -50, 0), glm::vec3(50, 50, 50), glm::vec3(0, 0, 0), {}, {}, GameObjectTags::Ground));
    objects.at(nextGameObjectId).loadModel("models/testMap/testMap.obj");
    objects.at(nextGameObjectId).initGraphics(renderer, "textures/concrete.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    nextGameObjectId++;

    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config4, glm::vec3(15, -20, 16), glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), cubeVertices, cubeIndices, GameObjectTags::JumpPowerup));
    objects.at(nextGameObjectId).initGraphics(renderer, "textures/wood.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    nextGameObjectId++;

    mainLoop();
    renderer.cleanup();
    // socketManager.cleanup();
  }

  void initWindow()
  {
    glfwInit();

    // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    renderer.window = window;
    renderer.swapchainManager.setWindow(window);
  }

  static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
  {
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (app->firstMouse)
    {
      app->lastX = xpos;
      app->lastY = ypos;
      app->firstMouse = false;
    }

    float xoffset = xpos - app->lastX;
    float yoffset = app->lastY - ypos;

    app->lastX = xpos;
    app->lastY = ypos;

    app->camera.ProcessMouseMovement(xoffset, yoffset);
  }

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
  {
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    app->renderer.framebufferResized = true;
  }
  static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
  {
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    app->camera.ProcessMouseScroll(static_cast<float>(yoffset));
  }

  void mainLoop()
  {
    initFPSCounter();
    btBroadphaseInterface *broadphase = new btDbvtBroadphase();
    btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -20.f, 0));

    for (auto &gameObject : objects)
    {
      gameObject.second.initPhysics(dynamicsWorld);
    }
    objects.at(6).rigidBody->setCcdMotionThreshold(0.5f);
    objects.at(6).rigidBody->setCcdSweptSphereRadius(0.5f);
    PlayerContactCallback callback(objects.at(6).rigidBody);

    socketManager.startReceiving();

    while (!glfwWindowShouldClose(renderer.window))
    {
      {
        std::lock_guard<std::mutex> lock(objectsMutex);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float time = glfwGetTime();

        dynamicsWorld->stepSimulation(deltaTime);
        dynamicsWorld->contactTest(objects.at(6).rigidBody, callback);

        grounded = isPlayerGrounded(objects.at(6), dynamicsWorld);

        currentTime = std::chrono::system_clock::now();
        std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
        long currentTimeInSeconds = duration.count();
        if (grounded)
        {
          movementState = MovementState::Ground;
          wallJumpCount = 2;

          if (currentTimeInSeconds - lastDashTime >= 1)
          {
            dashCount = 1;
          }
        }
        else
        {
          movementState = MovementState::Air;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastBroadcast;
        if (elapsed.count() >= 0.1f)
        {
          socketManager.broadcast(objects.at(6).pos);
          lastBroadcast = currentTime;
        }

        if (currentTimeInSeconds - lastSpeedBoostTime < 5)
        {
          speedMultiplier = 1.5;
        }
        else
        {
          speedMultiplier = 1;
        }

        if (currentTimeInSeconds - lastJumpBoostTime < 5)
        {
          jumpMultiplier = 1.5;
        }
        else
        {
          jumpMultiplier = 1;
        }

        for (auto &gameObject : objects)
        {
          if (gameObject.second.tag == GameObjectTags::SpeedPowerup && glm::distance(objects.at(6).pos, gameObject.second.pos) < 2)
          {
            lastSpeedBoostTime = currentTimeInSeconds;
          }
          else if (gameObject.second.tag == GameObjectTags::JumpPowerup && glm::distance(objects.at(6).pos, gameObject.second.pos) < 2)
          {
            lastJumpBoostTime = currentTimeInSeconds;
          }

          gameObject.second.updatePhysics();
        }

        if (camera.type = FirstPerson)
        {
          processPlayerInput(objects.at(6));
          btTransform trans;
          objects.at(6).rigidBody->getMotionState()->getWorldTransform(trans);
          btVector3 origin = trans.getOrigin();
          camera.Position = glm::vec3(origin.x(), origin.y() + objects.at(6).scale.y * 0.4, origin.z());

          btVector3 velocity = objects.at(6).rigidBody->getLinearVelocity();

          btVector3 horizontalVelocity(velocity.x(), 0.0f, velocity.z());

          float speed = horizontalVelocity.length();

          float zoomFactor = 1.0f + speed / (maxSpeed * 4);
          float smoothSpeed = 10.0f;
          camera.Zoom = zoomLerp(camera.Zoom, std::clamp(ZOOM * zoomFactor, 85.0f, 120.0f), smoothSpeed * deltaTime);

          if (speed > maxSpeed * speedMultiplier)
          {
            btVector3 excessVelocity = horizontalVelocity.normalized() * (speed - maxSpeed * speedMultiplier);

            btVector3 dampingForce = -excessVelocity * objects.at(6).rigidBody->getMass() * dampingFactor;

            objects.at(6).rigidBody->applyCentralForce(dampingForce);
            objects.at(6).rigidBody->activate();
          }
        }
        else
        {
          processInput();
        }

        glfwPollEvents();

        renderer.drawFrame();

        updateFPSCounter();
      }
    }

    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
    vkDeviceWaitIdle(renderer.deviceManager.device);

    renderer.swapchainManager.cleanupDepthImages(renderer.deviceManager.device);
    renderer.swapchainManager.cleanupSwapChain(renderer.deviceManager.device);

    renderer.bufferManager.cleanup(renderer.deviceManager.device);

    renderer.descriptorManager.cleanup(renderer.deviceManager.device);

    renderer.pipelineManager.cleanup(renderer.deviceManager.device);

    for (size_t i = 0; i < renderer.MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroySemaphore(renderer.deviceManager.device, renderer.renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(renderer.deviceManager.device, renderer.imageAvailableSemaphores[i], nullptr);
      vkDestroyFence(renderer.deviceManager.device, renderer.inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(renderer.deviceManager.device, renderer.commandPool, nullptr);
    for (auto &gameObject : objects)
    {
      gameObject.second.cleanupPhysics(dynamicsWorld);
      gameObject.second.textureManager.cleanup(renderer.deviceManager.device);
    }
    renderer.cleanup();
    socketManager.cleanup();
  }

  float zoomLerp(float start, float end, float t)
  {
    return start + t * (end - start);
  }

  int addPlayer()
  {
    std::lock_guard<std::mutex> lock(objectsMutex);

    PhysicsConfig config;
    config.collider = ColliderType::Box;
    config.isRigidBody = true;
    config.canMove = false;
    config.canRotateX = false;
    config.canRotateY = false;
    config.canRotateZ = false;
    config.friction = 0.8;
    config.linearDamping = 0.1;
    config.mass = 30;
    config.restitution = 0.8;
    objects.emplace(nextGameObjectId, GameObject(renderer, nextGameObjectId, config, glm::vec3(-5, 5, 0), glm::vec3(1, 3, 1), glm::vec3(0, 0, 0), cubeVertices, cubeIndices, GameObjectTags::NetworkedPlayer));
    objects.at(nextGameObjectId).initGraphics(renderer, "textures/wall.png");
    renderer.drawObjects.emplace(nextGameObjectId, &objects.at(nextGameObjectId));
    objects.at(nextGameObjectId).initPhysics(dynamicsWorld);
    nextGameObjectId++;
    return nextGameObjectId - 1;
  }

  void removePlayer(int id)
  {
    std::lock_guard<std::mutex> lock(objectsMutex);

    auto it = objects.find(id);
    if (it != objects.end())
    {
      it->second.setPosition(glm::vec3(10000, 10000, 10000));
      it->second.cleanupPhysics(dynamicsWorld);
    }
    else
    {
      std::cerr << "Error: Attempted to remove non-existent player with ID " << id << std::endl;
    }
  }

  void setTagged(int id)
  {
    std::lock_guard<std::mutex> lock(objectsMutex);
    auto it = objects.find(id);
    if (it != objects.end())
    {
      if (taggedPlayer != -1)
      {
        auto previousTagger = objects.find(taggedPlayer);
        if (previousTagger != objects.end())
        {
          std::cout << "Editing previous tagger in setTagged: " << taggedPlayer << std::endl;
          previousTagger->second.textureManager.updateTexture("textures/wall.png", renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
        }
      }
      it->second.textureManager.updateTexture("textures/fire.png", renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
      taggedPlayer = id;
    }
    else
    {
      std::cerr << "Error: Attempted to set non-existent player to tagged with ID " << id << std::endl;
    }
  }

  void youAreTagged()
  {
    std::lock_guard<std::mutex> lock(objectsMutex);

    auto previousTagger = objects.find(taggedPlayer);
    if (previousTagger != objects.end())
    {
      std::cout << "Editing previous tagger in youAreTagged: " << taggedPlayer << std::endl;
      previousTagger->second.textureManager.updateTexture("textures/wall.png", renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
    }

    taggedPlayer = -1;
  }

  void editPlayer(PlayerData data)
  {
    std::lock_guard<std::mutex> lock(objectsMutex);

    if (objects.find(data.id) != objects.end())
    {
      objects.at(data.id).setPosition(data.position);
    }
    else
    {
      std::cerr << "Error: Attempted to edit non-existent player with ID " << data.id << std::endl;
    }
  }

  void processInput()
  {
    if (glfwGetKey(renderer.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(renderer.window, true);

    float cameraSpeed = 20.0f * deltaTime;

    if (glfwGetKey(renderer.window, GLFW_KEY_W) == GLFW_PRESS)
      camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(renderer.window, GLFW_KEY_S) == GLFW_PRESS)
      camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(renderer.window, GLFW_KEY_A) == GLFW_PRESS)
      camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(renderer.window, GLFW_KEY_D) == GLFW_PRESS)
      camera.ProcessKeyboard(RIGHT, deltaTime);
  }

  void processPlayerInput(GameObject &player)
  {
    if (glfwGetKey(renderer.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(renderer.window, true);

    if (glfwGetKey(renderer.window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && controlPressed == false)
    {
      player.setScale(glm::vec3(player.scale.x, player.scale.y / 2, player.scale.z));
    }
    else if (glfwGetKey(renderer.window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS && controlPressed == true)
    {
      player.setScale(glm::vec3(player.scale.x, player.scale.y * 2, player.scale.z));
    }

    if (glfwGetKey(renderer.window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
      movementState = MovementState::FallingFast;
      if (!grounded)
      {
        btVector3 force(0, -3, 0);
        player.rigidBody->applyImpulse(force, btVector3(0, 0, 0));
      }
      controlPressed = true;
    }
    else
    {
      controlPressed = false;
    }

    float cameraSpeed = 20.0f * deltaTime;

    if (glfwGetKey(renderer.window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(renderer.window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(renderer.window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(renderer.window, GLFW_KEY_D) == GLFW_PRESS)
    {
      float velocity = 100 * player.rigidBody->getMass();
      if (movementState = MovementState::Air)
      {
        velocity = 80 * player.rigidBody->getMass();
      }
      if (movementState = MovementState::FallingFast)
      {
        currentTime = std::chrono::system_clock::now();
        std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
        long currentTimeInSeconds = duration.count();
        if (currentTimeInSeconds - lastDashTime >= 1)
        {
          velocity = 60 * player.rigidBody->getMass();
        }
      }
      btVector3 force(0, 0, 0);

      glm::vec3 forward = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
      if (glfwGetKey(renderer.window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(renderer.window, GLFW_KEY_S) != GLFW_PRESS)
      {
        force += btVector3(forward.x, 0.0f, forward.z);
      }
      if (glfwGetKey(renderer.window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(renderer.window, GLFW_KEY_W) != GLFW_PRESS)
      {
        force -= btVector3(forward.x, 0.0f, forward.z);
      }
      if (glfwGetKey(renderer.window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(renderer.window, GLFW_KEY_D) != GLFW_PRESS)
      {
        force -= btVector3(camera.Right.x, 0.0f, camera.Right.z);
      }
      if (glfwGetKey(renderer.window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(renderer.window, GLFW_KEY_A) != GLFW_PRESS)
      {
        force += btVector3(camera.Right.x, 0.0f, camera.Right.z);
      }

      force = force.normalize() * velocity * speedMultiplier;
      if (!std::isnan(force.x()) && !std::isnan(force.y()) && !std::isnan(force.z()))
      {
        float playerHeight = 3.0f;
        btVector3 feetPosition = player.rigidBody->getCenterOfMassPosition() - btVector3(0, playerHeight / 2, 0);
        btVector3 headPosition = player.rigidBody->getCenterOfMassPosition() + btVector3(0, playerHeight / 2, 0);

        bool hasHit = false;
        for (float i = feetPosition.getY() + 0.01; i <= headPosition.getY() + 0.01; i += 1.f)
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
    }

    if (glfwGetKey(renderer.window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed)
    {
      if (grounded)
      {
        btVector3 force(0, 500, 0);
        player.rigidBody->applyImpulse(force * jumpMultiplier, btVector3(0, 0, 0));
        dashCount = 1;
      }

      bool canWalljump = wallJumpCount > 0 && isTouchingWall(player, dynamicsWorld);
      if (canWalljump && (!grounded || camera.Pitch > 80))
      {
        btVector3 force(0, 350, 0);
        player.rigidBody->applyImpulse(force * jumpMultiplier, btVector3(0, 0, 0));
        dashCount = 1;
        wallJumpCount--;
      }

      spacePressed = true;
    }
    if (glfwGetKey(renderer.window, GLFW_KEY_SPACE) != GLFW_PRESS)
    {
      spacePressed = false;
    }

    if (glfwGetKey(renderer.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !shiftPressed && dashCount > 0)
    {
      float velocity = 100 * player.rigidBody->getMass();

      glm::vec3 forward = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
      player.rigidBody->applyImpulse(btVector3(forward.x, forward.y, forward.z) * velocity * speedMultiplier, btVector3(0, 0, 0));

      currentTime = std::chrono::system_clock::now();
      std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
      long currentTimeInSeconds = duration.count();
      lastDashTime = currentTimeInSeconds;
      dashCount--;
      shiftPressed = true;
    }
    if (glfwGetKey(renderer.window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS)
    {
      shiftPressed = false;
    }
  }

  bool isPlayerGrounded(GameObject &player, btDynamicsWorld *dynamicsWorld)
  {
    float playerHeight = player.scale.y;
    btVector3 feetPosition = player.rigidBody->getCenterOfMassPosition() - btVector3(0, (playerHeight / 2), 0) + btVector3(0, 1, 0);
    btVector3 rayEnd = feetPosition + btVector3(0, -2, 0);
    btCollisionWorld::AllHitsRayResultCallback rayCallback(feetPosition, rayEnd);
    dynamicsWorld->rayTest(feetPosition, rayEnd, rayCallback);

    for (int i = 0; i < rayCallback.m_hitFractions.size(); i++)
    {
      const btCollisionObject *hitObject = rayCallback.m_collisionObjects[i];
      const btRigidBody *rigidBody = dynamic_cast<const btRigidBody *>(hitObject);

      if (rigidBody)
      {
        GameObject *obj = static_cast<GameObject *>(rigidBody->getUserPointer());

        if (obj->tag == GameObjectTags::Ground)
        {
          return true;
        }
      }
    }

    return false;
  }

  bool isTouchingWall(GameObject &player, btDynamicsWorld *dynamicsWorld)
  {

    float playerHeight = player.scale.y;
    btVector3 feetPosition = player.rigidBody->getCenterOfMassPosition() - btVector3(0, playerHeight / 2, 0);
    btVector3 headPosition = player.rigidBody->getCenterOfMassPosition() + btVector3(0, playerHeight / 2, 0);

    std::vector<btVector3> directions = {
        btVector3(1, 0, 0),
        btVector3(-1, 0, 0),
        btVector3(0, 0, 1),
        btVector3(0, 0, -1)};

    bool hasHit = false;
    for (const btVector3 &dir : directions)
    {
      for (float i = feetPosition.getY() + 0.01; i <= headPosition.getY() + 0.01; i += 1.f)
      {
        btVector3 position(feetPosition.getX(), i, feetPosition.getZ());
        btCollisionWorld::ClosestRayResultCallback rayCallback(position, position + dir);
        dynamicsWorld->rayTest(position, position + dir, rayCallback);
        if (rayCallback.hasHit())
        {
          const btRigidBody *rigidBody = dynamic_cast<const btRigidBody *>(rayCallback.m_collisionObject);
          if (rigidBody)
          {
            btScalar mass = rigidBody->getMass();
            if (mass == 0)
            {
              hasHit = true;
              btVector3 wallNormal = rayCallback.m_hitNormalWorld;
              btVector3 pushDirection = wallNormal.normalized();

              btRigidBody *playerRigidBody = player.rigidBody;
              playerRigidBody->setLinearVelocity(playerRigidBody->getLinearVelocity() + pushDirection * 10);
              break;
            }
          }
        }
      }
    }

    return hasHit;
  }

  void initFPSCounter()
  {
    lastTime = std::chrono::high_resolution_clock::now();
  }

  void updateFPSCounter()
  {
    frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - lastTime;

    if (elapsed.count() >= 1.0f)
    {
      float fps = frameCount;
      std::cout << "FPS: " << fps << std::endl;
      frameCount = 0;
      lastTime = currentTime;
    }
  }
};