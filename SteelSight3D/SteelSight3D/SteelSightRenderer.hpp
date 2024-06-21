#pragma once
#include <cassert>
#include <vector>
#include <iostream>
#include <memory>

#include "SteelSightWindow.hpp"
#include "SteelSightDevice.hpp"
#include "SteelSightSwapChain.hpp"

namespace Voortman {
	class SteelSightRenderer final {
	public:

		SteelSightRenderer(SteelSightWindow& Window, SteelSightDevice& Device);
		~SteelSightRenderer();

		SteelSightRenderer(const SteelSightRenderer&) = delete;
		SteelSightRenderer& operator=(const SteelSightRenderer&) = delete;

		_NODISCARD inline VkRenderPass getSwapChainRenderPass() const noexcept { return SSSwapChain->getRenderPass(); }

		_NODISCARD inline float getAspectRatio() const noexcept { return SSSwapChain->extentAspectRatio(); }

		_NODISCARD inline bool isFrameInProgress() const noexcept { return isFrameStarted; }

		_NODISCARD inline VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame is in progress");
			return commandBuffers[currentFrameIndex];
		}

		_NODISCARD inline int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		SteelSightWindow& SSWindow;
		SteelSightDevice& SSDevice;
		std::unique_ptr<SteelSightSwapChain> SSSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex{ 0 };
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	};
}