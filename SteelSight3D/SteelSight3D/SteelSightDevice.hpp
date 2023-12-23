#pragma once
#include "SteelSightWindow.hpp"

#include <optional>
#include <string>
#include <vector>

namespace Voortman {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	class SteelSightDevice {
	public:
		SteelSightDevice(SteelSightWindow& SSwindow);
		~SteelSightDevice();

		SteelSightDevice(const SteelSightDevice&) = delete;
		void operator=(const SteelSightDevice&) = delete;
		SteelSightDevice(SteelSightDevice&&) = delete;
		SteelSightDevice& operator=(SteelSightDevice&&) = delete;

		inline VkCommandPool getCommandPool() { return commandPool; }
		inline VkDevice device() { return device_; }

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

		// Reference to the window
		SteelSightWindow& window;

		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		void createSurface();
		void CreateInstance();
		void CreateLogicalDevice();
		void setupDebugMessenger();
		void pickPhysicalDevice();
		std::vector<const char*> getRequiredExtensions();
	};
}