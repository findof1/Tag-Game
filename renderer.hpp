#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <chrono>
#include <GLFW/glfw3.h>
#include <memory>
#include "swapchainManager.hpp"
#include "deviceManager.hpp"
#include "textureManager.hpp"
#include "descriptorManager.hpp"
#include "pipelineManager.hpp"
#include "camera.h"
#include "gameObject.hpp"
#include "bufferManager.hpp"
#include "vertex.h"

class Camera;
class SwapchainManager;
class BufferManager;
class TextureManager;
class DescriptorManager;
class PipelineManager;
class DeviceManager;
struct Vertex;
class GameObject;

class Renderer
{
public:
  uint32_t WIDTH = 1600;
  uint32_t HEIGHT = 1200;
  const int MAX_FRAMES_IN_FLIGHT = 2;
  const std::string MODEL_PATH = "models/couch1.obj";

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  const bool enableValidationLayers = false;
  GLFWwindow *window;

  void run();

  Renderer();

private:
  Camera camera;
  VkInstance instance;
  VkQueue graphicsQueue;
  VkQueue presentQueue;

  SwapchainManager swapchainManager;
  BufferManager bufferManager;
  TextureManager textureManager;
  DescriptorManager descriptorManager;
  PipelineManager pipelineManager;
  DeviceManager deviceManager;

  std::unordered_map<int, std::shared_ptr<GameObject>> drawObjects;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  bool firstMouse = true;
  float lastX = WIDTH / 2, lastY = HEIGHT / 2;
  float deltaTime = 0.0f;
  float lastFrame = 0.0f;

  bool framebufferResized = false;

  uint32_t currentFrame = 0;

  void initWindow();

  std::chrono::high_resolution_clock::time_point lastTime;
  int frameCount = 0;

  void initFPSCounter();
  void updateFPSCounter();

  void processInput();
  static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
  static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

  void initVulkan();

  void createInstance();
  bool checkValidationLayerSupport();

  void loadModel();

  void recreateSwapChain();

  void createSyncObjects();

  void createCommandBuffer();
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void createCommandPool();

  void mainLoop();
  void drawFrame();

  void cleanup();
};