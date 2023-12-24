#include "SteelSightRenderer.hpp"

#include <stdexcept>
#include <array>
#include <vector>
#include <memory>
#include <cassert>
#include <iostream>
#include "SteelSightApp.hpp"

namespace Voortman {

	SteelSightRenderer::SteelSightRenderer(SteelSightWindow& window, SteelSightDevice& device) : SSWindow{ window }, SSDevice{ device } {
		recreateSwapChain();
		createCommandBuffers();
	}

	SteelSightRenderer::~SteelSightRenderer() {
		freeCommandBuffers();
	}

	void SteelSightRenderer::createCommandBuffers() {
		commandBuffers.resize(SteelSightSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = SSDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(SSDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Could not allocate command buffers");
		}
	}

	void SteelSightRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			SSDevice.device(),
			SSDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	void SteelSightRenderer::recreateSwapChain() {
		auto extent = SSWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = SSWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(SSDevice.device());

		if (SSSwapChain == nullptr) {
			SSSwapChain = std::make_unique<SteelSightSwapChain>(SSDevice, extent);
		}
		else {
			std::shared_ptr<SteelSightSwapChain> oldSwapChain = std::move(SSSwapChain);
			SSSwapChain = std::make_unique<SteelSightSwapChain>(SSDevice, extent, oldSwapChain);

			if (!oldSwapChain->comparedSwapFormats(*SSSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}
	}

	VkCommandBuffer SteelSightRenderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginframe while already in progress");

		uint32_t imageIndex{ 0 };

		auto result = SSSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;
		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffers");
		}
		return commandBuffer;
	}

	void SteelSightRenderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = SSSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			SSWindow.wasWindowResized()) {
			SSWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % SteelSightSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void SteelSightRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = SSSwapChain->getRenderPass();
		renderPassInfo.framebuffer = SSSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = SSSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.f, 0.f, 0.f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(SSSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(SSSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, SSSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void SteelSightRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}