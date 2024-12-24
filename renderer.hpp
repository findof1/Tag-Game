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
class DescriptorManager;
class PipelineManager;
class DeviceManager;
struct Vertex;
class GameObject;

class Renderer
{
public:
  const int MAX_FRAMES_IN_FLIGHT = 2;

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  const bool enableValidationLayers = false;
  GLFWwindow *window;

  void initVulkan();

  Renderer(Camera &camera, uint32_t &WIDTH, uint32_t &HEIGHT);

  Camera &camera;
  VkInstance instance;
  VkQueue graphicsQueue;
  VkQueue presentQueue;

  SwapchainManager swapchainManager;
  BufferManager bufferManager;
  DescriptorManager descriptorManager;
  PipelineManager pipelineManager;
  DeviceManager deviceManager;

  std::unordered_map<int, GameObject *> drawObjects;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  bool framebufferResized = false;

  void drawFrame();

  void cleanup();

private:
  uint32_t &WIDTH;
  uint32_t &HEIGHT;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  uint32_t currentFrame = 0;

  void createInstance();
  bool checkValidationLayerSupport();

  void recreateSwapChain();

  void createSyncObjects();

  void createCommandBuffer();
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void createCommandPool();
};