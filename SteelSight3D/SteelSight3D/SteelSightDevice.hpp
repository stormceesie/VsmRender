#pragma once
#include "SteelSightWindow.hpp"

#include <optional>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

// Here define if you want to use MAILBOX_MODE mode or IMMEDIATE_MODE
// Code will try to choose if this is available
// #define MAILBOX_MODE
// #define IMMEDIATE_MODE

namespace Voortman {
	struct SwapChainSupportDetails final {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats{};
		std::vector<VkPresentModeKHR> presentModes{};
	};

	struct QueueFamilyIndices final {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	class SteelSightDevice final {
	public:
		SteelSightDevice(SteelSightWindow& SSwindow);
		~SteelSightDevice();

		SteelSightDevice(const SteelSightDevice&) = delete;
		void operator=(const SteelSightDevice&) = delete;
		SteelSightDevice(SteelSightDevice&&) = delete;
		SteelSightDevice& operator=(SteelSightDevice&&) = delete;

		_NODISCARD const inline VkCommandPool getCommandPool()                 const noexcept { return commandPool; }
		_NODISCARD const inline VkDevice device()                              const noexcept { return device_; }
		_NODISCARD const inline VkSurfaceKHR surface()                         const noexcept { return surface_; }
		_NODISCARD const inline VkQueue graphicsQueue()                        const noexcept { return graphicsQueue_; }
		_NODISCARD const inline VkQueue presentQueue()                         const noexcept { return presentQueue_; }
		_NODISCARD const inline VkSampleCountFlagBits GetSampleCountFlagBits() const noexcept { return msaaSamples; }

		SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		void createImageWithInfo(
			const VkImageCreateInfo& imageInfo,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory);

		void createBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory);

		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	private:
		// Don't build any functions or variables related to validation layers
#ifdef _DEBUG
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		VkDebugUtilsMessengerEXT debugMessenger{};
		const std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };
		bool CheckValidationLayerSupport();
#endif
		const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkInstance instance_;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkCommandPool commandPool;

		VkDevice device_;

		VkSurfaceKHR surface_;

		VkQueue graphicsQueue_;
		VkQueue presentQueue_;
		VkSampleCountFlagBits msaaSamples;

		// Reference to the window
		SteelSightWindow& window;

		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

		inline void createSurface();
		void CreateInstance();
		void CreateLogicalDevice();
		void setupDebugMessenger();
		void pickPhysicalDevice();
		void createCommandPool();

		static std::vector<const char*> getRequiredExtensions();
	};
}