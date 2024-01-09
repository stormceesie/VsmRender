#pragma once
#include "MachineSimulatorDefines.hpp"
#include "MachineSimulatorPCH.hpp"
#include "MachineSimulatorWindow.hpp"

namespace Voortman {
	class MachineSimulatorDevice {
	public:
		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			inline bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
		};

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		MachineSimulatorDevice(MachineSimulatorWindow& window);
		~MachineSimulatorDevice();

		MachineSimulatorDevice(const MachineSimulatorDevice&) = delete;

		// Inline short functions inline can reduces overhead for short functions
		inline VkPhysicalDevice* GetPhysicalDevice() { return &physicalDevice; }
		inline VkDevice* Device() { return &device_; }

	private:
		void createInstance();
		std::vector<const char*> getRequiredExtensions();
		void pickPhysicalDevice();
		unsigned int rateDeviceSuitability(const VkPhysicalDevice& device) const;
		void createSwapChain();

		QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device) const;
		bool IsDeviceSuitable(const VkPhysicalDevice& device) const;
		void createLogicalDevice();
		bool checkDeviceExtensionSupport(const VkPhysicalDevice& device) const;
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device) const;

#ifdef CONSOLE_OUTPUT
		void PrintGPUStats(unsigned int VRAM, unsigned int MSAASampleRate, const std::string& deviceName) const;
#endif

		void GetMemoryProperty(const VkPhysicalDevice& device, unsigned int& HeapSize) const;
		void getMaxUsableSampleCount(const VkPhysicalDevice& device, VkSampleCountFlagBits& MSAASampleCount) const;

#ifdef VALIDATION_LAYERS
		void setupDebugMessenger();
		bool checkValidationLayerSupport();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		VkDebugUtilsMessengerEXT debugMessenger;
		static const inline std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };
#endif

		static const inline std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		VkQueue presentQueue_;
		VkQueue graphicsQueue;

		VkInstance instance_;
		VkDevice device_;
		MachineSimulatorWindow& MSWindow;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		VkSurfaceKHR surface;
	};
}