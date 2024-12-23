#include "renderer.hpp"
#include <btBulletDynamicsCommon.h>
#include "gameObjectPhysicsConfig.hpp"
#include "camera.h"
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
  Camera camera;
  GLFWwindow *window;

  uint32_t WIDTH = 1600;
  uint32_t HEIGHT = 1200;

  std::chrono::high_resolution_clock::time_point lastTime;
  int frameCount = 0;
  bool firstMouse = true;
  float lastX = WIDTH / 2, lastY = HEIGHT / 2;
  float deltaTime = 0.0f;
  float lastFrame = 0.0f;
  std::unordered_map<int, GameObject> objects;

  Application() : camera(), renderer(camera, WIDTH, HEIGHT)
  {
  }

  void run()
  {
    initWindow();
    renderer.initVulkan();

    PhysicsConfig config0;
    config0.collider = ColliderType::Mesh;
    config0.isRigidBody = true;

    PhysicsConfig config1;
    config1.collider = ColliderType::Box;
    config1.isRigidBody = true;
    config1.canMove = false;
    config1.canRotateX = false;
    config1.canRotateY = false;
    config1.canRotateZ = false;

    PhysicsConfig config2;
    config2.collider = ColliderType::Box;
    config2.isRigidBody = true;

    PhysicsConfig config3;
    config3.collider = ColliderType::Box;
    config3.isRigidBody = true;

    PhysicsConfig config4;
    config4.collider = ColliderType::Box;
    config4.interactable = false;

    PhysicsConfig config5;
    config5.collider = ColliderType::None;

    objects.emplace(0, GameObject(renderer, 0, config0, glm::vec3(0, 20, 0), glm::vec3(0.1, 0.1, 0.1), glm::vec3(10, 40, 50), 74, {}, {}));
    objects.emplace(1, GameObject(renderer, 1, config1, glm::vec3(0, 0, 0), glm::vec3(50, 2, 50), glm::vec3(0, 0, 0), 0, cubeVertices, cubeIndices));
    objects.emplace(2, GameObject(renderer, 2, config2, glm::vec3(0, 30, 0), glm::vec3(1, 1, 1), glm::vec3(0, 30, 45), 1, cubeVertices, cubeIndices));
    objects.emplace(3, GameObject(renderer, 3, config3, glm::vec3(5.1, 15, 0), glm::vec3(2, 1, 1), glm::vec3(0, 10, 45), 1, cubeVertices, cubeIndices));

    objects.emplace(4, GameObject(renderer, 4, config4, glm::vec3(5, 5, 0), glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), 1, cubeVertices, cubeIndices));

    objects.emplace(5, GameObject(renderer, 5, config5, glm::vec3(0, 0, 0), glm::vec3(500, 500, 500), glm::vec3(0, 0, 0), 0, cubeVertices, skyBoxIndices));

    objects.at(0).loadModel("models/couch1.obj");
    objects.at(0).initGraphics(renderer, "models/gray.png");
    objects.at(1).initGraphics(renderer, "textures/wood.png");
    objects.at(2).initGraphics(renderer, "textures/metal.png");
    objects.at(3).initGraphics(renderer, "textures/wall.png");
    objects.at(4).initGraphics(renderer, "textures/wood.png");
    objects.at(5).initGraphics(renderer, "textures/sky.png");

    renderer.drawObjects.emplace(0, &objects.at(0));
    renderer.drawObjects.emplace(1, &objects.at(1));
    renderer.drawObjects.emplace(2, &objects.at(2));
    renderer.drawObjects.emplace(3, &objects.at(3));
    renderer.drawObjects.emplace(4, &objects.at(4));
    renderer.drawObjects.emplace(5, &objects.at(5));
    mainLoop();
    renderer.cleanup();
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
    btDiscreteDynamicsWorld *dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

    for (auto &gameObject : objects)
    {
      gameObject.second.initPhysics(dynamicsWorld);
    }

    while (!glfwWindowShouldClose(renderer.window))
    {
      float currentFrame = static_cast<float>(glfwGetTime());
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      float time = glfwGetTime();

      dynamicsWorld->stepSimulation(deltaTime);

      for (auto &gameObject : objects)
      {
        gameObject.second.updatePhysics();
      }

      processInput();
      glfwPollEvents();
      renderer.drawFrame();
      updateFPSCounter();
    }

    for (auto &gameObject : objects)
    {
      gameObject.second.cleanupPhysics(dynamicsWorld);
      gameObject.second.textureManager.cleanup(renderer.deviceManager.device);
    }
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
    vkDeviceWaitIdle(renderer.deviceManager.device);
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
      float fps = frameCount / elapsed.count();
      std::cout << "FPS: " << fps << std::endl;
      frameCount = 0;
      lastTime = currentTime;
    }
  }
};