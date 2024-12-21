#include "renderer.hpp"
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

    objects.emplace(0, GameObject(renderer, 0, glm::vec3(0, 10, 0), glm::vec3(0.1, 0.1, 0.1), 0, 0, {}, {}));
    objects.emplace(1, GameObject(renderer, 1, glm::vec3(0, 5.5, 0), glm::vec3(50, 2, 50), 0, 0, cubeVertices, cubeIndices));

    objects.at(0).loadModel("models/couch1.obj");
    objects.at(0).initGraphics(renderer, renderer.couchTextureManager);
    objects.at(1).initGraphics(renderer, renderer.textureManager);

    renderer.drawObjects.emplace(0, &objects.at(0));
    renderer.drawObjects.emplace(1, &objects.at(1));
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
    while (!glfwWindowShouldClose(renderer.window))
    {
      float currentFrame = static_cast<float>(glfwGetTime());
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      float time = glfwGetTime();

      float oscillationY = 2.0f * sin(time * 2);
      float oscillationX = 2.0f * cos(time);
      float oscillationZ = 2.0f * sin(time * 2) * cos(time);

      objects.at(0).pos.y = 10.0f + oscillationY;
      objects.at(0).pos.x = oscillationX;
      objects.at(0).pos.z = oscillationZ;
      processInput();
      glfwPollEvents();
      renderer.drawFrame();
      updateFPSCounter();
    }

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
    { // If 1 second has passed
      float fps = frameCount / elapsed.count();
      std::cout << "FPS: " << fps << std::endl; // Output FPS
      frameCount = 0;                           // Reset frame counter
      lastTime = currentTime;                   // Reset timing
    }
  }
};