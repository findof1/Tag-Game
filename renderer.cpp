#include <tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderer.hpp"
#include "utils.h"
#include <cstring>

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

void Renderer::run()
{
  initWindow();
  initVulkan();
  mainLoop();
  cleanup();
}

Renderer::Renderer()
    : bufferManager(), swapchainManager(), deviceManager(swapchainManager), textureManager(bufferManager), descriptorManager(bufferManager, textureManager), pipelineManager(swapchainManager, descriptorManager)
{
}

void Renderer::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
  auto app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  app->framebufferResized = true;
}

void Renderer::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  auto app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  app->camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void Renderer::initWindow()
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
  swapchainManager.setWindow(window);
}

void Renderer::initVulkan()
{
  createInstance();
  swapchainManager.createSurface(instance);
  deviceManager.pickPhysicalDevice(instance, deviceExtensions);
  deviceManager.createLogicalDevice(enableValidationLayers, deviceExtensions, validationLayers, &presentQueue, &graphicsQueue);
  swapchainManager.createSwapChain(deviceManager.device, deviceManager.physicalDevice);
  swapchainManager.createImageViews(deviceManager.device);
  pipelineManager.createRenderPass(deviceManager.device, deviceManager.physicalDevice);
  descriptorManager.createDescriptorSetLayout(deviceManager.device);
  pipelineManager.createGraphicsPipeline(deviceManager.device);
  createCommandPool();
  swapchainManager.createDepthResources(deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  swapchainManager.createFramebuffers(deviceManager.device, pipelineManager.renderPass);
  textureManager.createTextureImage("textures/awesomeface.png", deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  textureManager.createTextureImageView(deviceManager.device);
  textureManager.createTextureSampler(deviceManager.device, deviceManager.physicalDevice);
  loadModel();
  // bufferManager.createVertexBuffer(vertices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  // bufferManager.createIndexBuffer(indices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, //graphicsQueue);
  // bufferManager.createUniformBuffers(MAX_FRAMES_IN_FLIGHT, deviceManager.device, deviceManager.physicalDevice, 2);
  descriptorManager.createDescriptorPool(deviceManager.device, MAX_FRAMES_IN_FLIGHT, 2);
  // descriptorManager.createDescriptorSets(deviceManager.device, MAX_FRAMES_IN_FLIGHT, 1);
  // descriptorManager.addDescriptorSets(deviceManager.device, MAX_FRAMES_IN_FLIGHT, 1);
  createCommandBuffer();
  createSyncObjects();

  drawObjects.emplace(0, std::make_shared<GameObject>(bufferManager, descriptorManager, 0, MAX_FRAMES_IN_FLIGHT, deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue, glm::vec3(0, 5, 0), glm::vec3(0.1, 0.1, 0.1), 0, 0, vertices, indices));

  drawObjects.emplace(1, std::make_shared<GameObject>(bufferManager, descriptorManager, 1, MAX_FRAMES_IN_FLIGHT, deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue, glm::vec3(0, -15, 0), glm::vec3(2, 2, 2), 0, 0, cubeVertices, cubeIndices));
}

void Renderer::loadModel()
{
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

void Renderer::recreateSwapChain()
{
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0)
  {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
  WIDTH = width;
  HEIGHT = height;
  vkDeviceWaitIdle(deviceManager.device);

  swapchainManager.cleanupSwapChain(deviceManager.device);

  swapchainManager.createSwapChain(deviceManager.device, deviceManager.physicalDevice);
  swapchainManager.createImageViews(deviceManager.device);
  swapchainManager.createDepthResources(deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  swapchainManager.createFramebuffers(deviceManager.device, pipelineManager.renderPass);
}

void Renderer::createSyncObjects()
{
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    if (vkCreateSemaphore(deviceManager.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(deviceManager.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(deviceManager.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create synchronization objects!");
    }
  }
}

void Renderer::createCommandBuffer()
{
  commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

  if (vkAllocateCommandBuffers(deviceManager.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.image = swapchainManager.swapChainImages[imageIndex];
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = pipelineManager.renderPass;
  renderPassInfo.framebuffer = swapchainManager.swapChainFramebuffers[imageIndex];

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swapchainManager.swapChainExtent;

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager.graphicsPipeline);

  VkViewport viewport{};

  viewport.x = 0.0f;
  viewport.y = 0.0f;

  viewport.width = static_cast<float>(swapchainManager.swapChainExtent.width);
  viewport.height = static_cast<float>(swapchainManager.swapChainExtent.height);

  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapchainManager.swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  for (auto &drawObject : drawObjects)
  {
    drawObject.second->draw(currentFrame, MAX_FRAMES_IN_FLIGHT, camera.GetViewMatrix(), glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f), commandBuffer, pipelineManager.pipelineLayout);
  }

  /*
          VkBuffer vertexBuffersArray2[] = {vertexBuffers[1]};
          VkDeviceSize offsets2[] = {0};
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersArray2, offsets2);

          vkCmdBindIndexBuffer(commandBuffer, indexBuffers[1], 0, VK_INDEX_TYPE_UINT32);

          vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

          updateUniformBuffer(currentFrame, glm::vec3(0.f, -1.5f, 0.f));

          vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices2.size()), 1, 0, 0, 0);

          */

  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void Renderer::createCommandPool()
{
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(deviceManager.physicalDevice, swapchainManager.surface);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(deviceManager.device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create command pool!");
  }
}

void Renderer::mainLoop()
{
  initFPSCounter();
  while (!glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput();
    glfwPollEvents();
    drawFrame();
    updateFPSCounter();
  }

  vkDeviceWaitIdle(deviceManager.device);
}

void Renderer::initFPSCounter()
{
  lastTime = std::chrono::high_resolution_clock::now();
}

void Renderer::updateFPSCounter()
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

void Renderer::mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
  auto app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
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

void Renderer::processInput()
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
  {
    vkQueueWaitIdle(graphicsQueue);
    bufferManager.freeVertexBuffer(0, deviceManager.device);
    bufferManager.freeIndexBuffer(0, deviceManager.device);
    vertices.clear();
    vertices = cubeVertices;
    indices.clear();
    indices = cubeIndices;
    bufferManager.createVertexBuffer(vertices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
    bufferManager.createIndexBuffer(indices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  }

  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
  {
    vkQueueWaitIdle(graphicsQueue);
    bufferManager.freeVertexBuffer(0, deviceManager.device);
    bufferManager.freeIndexBuffer(0, deviceManager.device);
    vertices.clear();
    indices.clear();
    loadModel();
    bufferManager.createVertexBuffer(vertices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
    bufferManager.createIndexBuffer(indices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  }

  float cameraSpeed = 20.0f * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

void Renderer::drawFrame()
{
  vkWaitForFences(deviceManager.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(deviceManager.device, swapchainManager.swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    recreateSwapChain();
    return;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vkResetFences(deviceManager.device, 1, &inFlightFences[currentFrame]);

  vkResetCommandBuffer(commandBuffers[currentFrame], 0);

  recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapchainManager.swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;

  result = vkQueuePresentKHR(presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
  {
    framebufferResized = false;
    recreateSwapChain();
  }
  else if (result != VK_SUCCESS)
  {
    throw std::runtime_error("failed to present swap chain image!");
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::cleanup()
{
  swapchainManager.cleanupDepthImages(deviceManager.device);
  swapchainManager.cleanupSwapChain(deviceManager.device);

  textureManager.cleanup(deviceManager.device);

  bufferManager.cleanup(deviceManager.device);

  descriptorManager.cleanup(deviceManager.device);

  pipelineManager.cleanup(deviceManager.device);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    vkDestroySemaphore(deviceManager.device, renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(deviceManager.device, imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(deviceManager.device, inFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(deviceManager.device, commandPool, nullptr);

  vkDestroyDevice(deviceManager.device, nullptr);
  vkDestroySurfaceKHR(instance, swapchainManager.surface, nullptr);
  vkDestroyInstance(instance, nullptr);

  glfwDestroyWindow(window);

  glfwTerminate();
}

void Renderer::createInstance()
{
  if (enableValidationLayers && !checkValidationLayerSupport())
  {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan";
  appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;

  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if (result != VK_SUCCESS)
  {
    std::cerr << "failed to create instance!" << result << std::endl;
    exit(EXIT_FAILURE);
  }
}

bool Renderer::checkValidationLayerSupport()
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers)
  {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers)
    {
      if (strcmp(layerName, layerProperties.layerName) == 0)
      {
        layerFound = true;
        break;
      }
    }

    if (!layerFound)
    {
      return false;
    }
  }
  return true;
}