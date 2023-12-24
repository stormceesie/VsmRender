#pragma once
#include "SteelSightDevice.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <string>

namespace Voortman {
	class SteelSightSwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		SteelSightSwapChain(SteelSightDevice& deviceRef, VkExtent2D windowExent);
		SteelSightSwapChain(SteelSightDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SteelSightSwapChain> previous);
		~SteelSightSwapChain();

		// SteelSightSwapChain(const SteelSightSwapChain &) = delete;
		SteelSightSwapChain& operator=(const SteelSightSwapChain &) = delete;

		VkFramebuffer getFrameBuffer(int index) { return swapChainFrameBuffers[index]; }
		VkRenderPass getRenderPass() { return renderpass; }
		VkImageView getImageView(int index) { return swapChainImageViews[index]; }
		inline size_t imageCount() { return swapChainImages.size(); }
		inline VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
		inline VkExtent2D getSwapChainExtent() { return swapChainExtent; }
		inline uint32_t width() { return swapChainExtent.width; }
		inline uint32_t height() { return swapChainExtent.height; }

		inline float extentAspectRatio() { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
		VkFormat findDepthFormat();

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		inline bool comparedSwapFormats(const SteelSightSwapChain swapChain) const {
			return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat;
		}

	private:
		void init();
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFrameBuffers();
		void createSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat swapChainImageFormat;
		VkFormat swapChainDepthFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkFramebuffer> swapChainFrameBuffers;
		VkRenderPass renderpass;

		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemory;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;

		SteelSightDevice& device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;
		std::shared_ptr<SteelSightSwapChain> oldSwapChain;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;

		size_t currentFrame{ 0 };
	};
}