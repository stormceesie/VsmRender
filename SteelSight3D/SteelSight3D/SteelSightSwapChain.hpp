#pragma once
#include "SteelSightDevice.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <string>

namespace Voortman {
	class SteelSightSwapChain final {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT{ 2 };

		SteelSightSwapChain(SteelSightDevice& deviceRef, VkExtent2D windowExent);
		SteelSightSwapChain(SteelSightDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SteelSightSwapChain> previous);
		~SteelSightSwapChain();

		SteelSightSwapChain(const SteelSightSwapChain &) = delete;
		SteelSightSwapChain& operator=(const SteelSightSwapChain &) = delete;

		// inline const noexcept to reduce overhead
		_NODISCARD const inline VkFramebuffer getFrameBuffer(int index) const noexcept { return swapChainFrameBuffers[index]; }
		_NODISCARD const inline VkRenderPass getRenderPass()            const noexcept { return renderpass; }
		_NODISCARD const inline VkImageView getImageView(int index)     const noexcept { return swapChainImageViews[index]; }
		_NODISCARD const inline size_t imageCount()                     const noexcept { return swapChainImages.size(); }
		_NODISCARD const inline VkFormat getSwapChainImageFormat()      const noexcept { return swapChainImageFormat; }
		_NODISCARD const inline VkExtent2D getSwapChainExtent()         const noexcept { return swapChainExtent; }
		_NODISCARD const inline uint32_t width()                        const noexcept { return swapChainExtent.width; }
		_NODISCARD const inline uint32_t height()                       const noexcept { return swapChainExtent.height; }

		inline float extentAspectRatio() const noexcept { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
		inline bool comparedSwapFormats(const SteelSightSwapChain& swapChain) const noexcept { return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat; }

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
		static inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static inline VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
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