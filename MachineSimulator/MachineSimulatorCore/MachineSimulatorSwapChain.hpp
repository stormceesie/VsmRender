#ifndef MACHINESIMULATORSWAPCHAIN_HPP
#define MACHINESIMULATORSWAPCHAIN_HPP

#include <stdlib.h>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <vector>

#include <vulkan/vulkan.h>
#include "VulkanTools.hpp"

namespace Voortman {
	typedef struct _SwapChainBuffers {
		VkImage image;
		VkImageView view;
	} SwapChainBuffer;

	class MachineSimulatorSwapChain {
	public:
		VkFormat colorFormat;
		VkColorSpaceKHR colorSpace;
		VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
		uint32_t imageCount;
		std::vector<VkImage> images;
		std::vector<SwapChainBuffer> buffers;
		uint32_t queueNodeIndex = UINT32_MAX;

		// Input is void pointer because 
		void initSurface(void* platformHandle, void* platformWindow);

		void connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
		void create(uint32_t* width, uint32_t* height, bool vsync = false, bool fullscreen = false);
		VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
		VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
		void cleanup();

	private:
		VkInstance instance;
		VkDevice device;
		VkPhysicalDevice physicalDevice;
		VkSurfaceKHR surface;
	};
}
#endif