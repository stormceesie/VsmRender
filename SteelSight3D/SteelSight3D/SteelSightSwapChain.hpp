#pragma once
#include "SteelSightDevice.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <string>

namespace Voortman {
	class SteelSightSwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT{ 2 };

		SteelSightSwapChain(SteelSightDevice& deviceRef, VkExtent2D windowExent);
		SteelSightSwapChain(SteelSightDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SteelSightSwapChain> previous);
		~SteelSightSwapChain();

		SteelSightSwapChain(const SteelSightSwapChain &) = delete;
		SteelSightSwapChain& operator=(const SteelSightSwapChain &) = delete;

		// inline const to reduce overhead
		inline VkFramebuffer getFrameBuffer(int index) const { return swapChainFrameBuffers[index]; }
		inline VkRenderPass getRenderPass() const { return renderpass; }
		inline VkImageView getImageView(int index) const { return swapChainImageViews[index]; }
		inline size_t imageCount() const { return swapChainImages.size(); }
		inline VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
		inline VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
		inline uint32_t width() const { return swapChainExtent.width; }
		inline uint32_t height() const { return swapChainExtent.height; }

		inline float extentAspectRatio() const { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
		inline bool comparedSwapFormats(const SteelSightSwapChain& swapChain) const { return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat; }

		VkFormat findDepthFormat();
		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
	private:
		void init();
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFrameBuffers();
		void createSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
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

		SteelSightDevice &device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;
		std::shared_ptr<SteelSightSwapChain> oldSwapChain;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;

		size_t currentFrame{0};
	};
}