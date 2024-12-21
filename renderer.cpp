#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderer.hpp"
#include "utils.h"
#include <cstring>

Renderer::Renderer(Camera &camera, uint32_t &WIDTH, uint32_t &HEIGHT)
    : bufferManager(), swapchainManager(), deviceManager(swapchainManager), textureManager(bufferManager), couchTextureManager(bufferManager), descriptorManager(bufferManager), pipelineManager(swapchainManager, descriptorManager), camera(camera), WIDTH(WIDTH), HEIGHT(HEIGHT)
{
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
  textureManager.createTextureImage("textures/wall.png", deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  textureManager.createTextureImageView(deviceManager.device);
  textureManager.createTextureSampler(deviceManager.device, deviceManager.physicalDevice);

  couchTextureManager.createTextureImage("models/gray.png", deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  couchTextureManager.createTextureImageView(deviceManager.device);
  couchTextureManager.createTextureSampler(deviceManager.device, deviceManager.physicalDevice);

  // bufferManager.createVertexBuffer(vertices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  // bufferManager.createIndexBuffer(indices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, //graphicsQueue);
  // bufferManager.createUniformBuffers(MAX_FRAMES_IN_FLIGHT, deviceManager.device, deviceManager.physicalDevice, 2);
  descriptorManager.createDescriptorPool(deviceManager.device, MAX_FRAMES_IN_FLIGHT, 4);
  // descriptorManager.createDescriptorSets(deviceManager.device, MAX_FRAMES_IN_FLIGHT, 1);
  // descriptorManager.addDescriptorSets(deviceManager.device, MAX_FRAMES_IN_FLIGHT, 1);
  createCommandBuffer();
  createSyncObjects();
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
    drawObject.second->draw(this, currentFrame, camera.GetViewMatrix(), glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f), commandBuffer);
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

  couchTextureManager.cleanup(deviceManager.device);

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