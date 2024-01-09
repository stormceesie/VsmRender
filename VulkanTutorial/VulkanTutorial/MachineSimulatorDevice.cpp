#include "MachineSimulatorPCH.hpp"
#include "MachineSimulatorDevice.hpp"

namespace Voortman {
#ifdef VALIDATION_LAYERS
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}
#endif

//########################## Class implementation ############################################################
	MachineSimulatorDevice::MachineSimulatorDevice(MachineSimulatorWindow& window) : MSWindow{ window } {
		createInstance();
#ifdef VALIDATION_LAYERS
		setupDebugMessenger();
#endif
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
	}

	MachineSimulatorDevice::~MachineSimulatorDevice() {
#ifdef VALIDATION_LAYERS
		if (debugMessenger) {
			DestroyDebugUtilsMessengerEXT(instance_, debugMessenger, nullptr);
		}
#endif
		if (swapChain) {
			vkDestroySwapchainKHR(device_, swapChain, nullptr);
		}

		if (surface) {
			vkDestroySurfaceKHR(instance_, surface, nullptr);
		}

		if (device_) {
			vkDestroyDevice(device_, nullptr);
		}
		
		if (instance_) {
			vkDestroyInstance(instance_, nullptr);
		}
	}

	void MachineSimulatorDevice::createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// The imageCount can be increased you can play with this what will work best
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 2;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain!");
		}
#ifdef CONSOLE_OUTPUT
		std::cout << "Swap chain is created !" << std::endl;
#endif

		vkGetSwapchainImagesKHR(device_, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device_, swapChain, &imageCount, swapChainImages.data());
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	void MachineSimulatorDevice::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
		if (deviceCount == 0) throw std::runtime_error("failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

		// If the user does not select a GPU the code will choose the best one
#ifndef SELECTED_GPU
		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) {
				int score = rateDeviceSuitability(device);
				candidates.insert(std::make_pair(score, device));
			}
		}

		// All devices will be sorted and the best one will be chosen by default
		if (candidates.rbegin()->first > 0) {
			physicalDevice = candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
#else
		// Pick the device that is chosen by the user
		if (devices.size() - 1 >= SELECTED_GPU) {
			if (IsDeviceSuitable(devices[SELECTED_GPU])) {
				physicalDevice = devices[SELECTED_GPU];

				// Show the user which GPU he chose
#ifdef CONSOLE_OUTPUT
				std::cout << std::endl << "GPU " << SELECTED_GPU << " was chosen as GPU!" << std::endl;
				rateDeviceSuitability(physicalDevice);
#endif
			}
			else {
				throw std::runtime_error("This GPU is not suitable!");
			}
		}
		else {
			throw std::runtime_error("This GPU does not exist!");
		}

#endif
	}

	bool MachineSimulatorDevice::IsDeviceSuitable(const VkPhysicalDevice& device) const {
		QueueFamilyIndices indices = findQueueFamilies(device);
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}
		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	// Function to return the VRAM in MB for a particular device
	void MachineSimulatorDevice::GetMemoryProperty(const VkPhysicalDevice& device, unsigned int& HeapSize) const {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
				// VRAM size
				size_t heapIndex = memProperties.memoryTypes[i].heapIndex;
				VkDeviceSize heapSize = memProperties.memoryHeaps[heapIndex].size;

				// Return the heapsize in MB
				HeapSize = static_cast<unsigned int>(heapSize / (1024 * 1024));
			}
		}
	}

	MachineSimulatorDevice::QueueFamilyIndices MachineSimulatorDevice::findQueueFamilies(const VkPhysicalDevice& device) const {
		QueueFamilyIndices indices{};

		uint32_t queueFamilyCount{ 0 };
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphicsFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}
		return indices;
	}

	void MachineSimulatorDevice::createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		std::set<uint32_t> uniqueQueueFamilies{
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef VALIDATION_LAYERS
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif
		// Create logical device
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
	}

	// Check if every used format is also supported by the physical device
	MachineSimulatorDevice::SwapChainSupportDetails MachineSimulatorDevice::querySwapChainSupport(const VkPhysicalDevice& device) const {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR MachineSimulatorDevice::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR MachineSimulatorDevice::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
#ifdef MAILBOX_MODE
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
#elif defined(IMMEDIATE_MODE)
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				return availablePresentMode;
			}
		}
#endif

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D MachineSimulatorDevice::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(MSWindow.GetWindowPtr(), &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	// Gives a GPU a score
	unsigned int MachineSimulatorDevice::rateDeviceSuitability(const VkPhysicalDevice& PhysicalDevice) const {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		VkSampleCountFlagBits MSAASampleRate;
		unsigned int VRAM;
		int score{ 0 };

		vkGetPhysicalDeviceProperties(PhysicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(PhysicalDevice, &deviceFeatures);
		GetMemoryProperty(PhysicalDevice, VRAM);
		getMaxUsableSampleCount(PhysicalDevice, MSAASampleRate);

		// Discrete GPU is better
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 10;
		}

		// 1 GB is 1 point
		score += VRAM / 1000;

#ifdef CONSOLE_OUTPUT
		PrintGPUStats(VRAM, MSAASampleRate, deviceProperties.deviceName);
#endif

		return score;
	}

	bool MachineSimulatorDevice::checkDeviceExtensionSupport(const VkPhysicalDevice& device) const {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

#ifdef CONSOLE_OUTPUT
	void MachineSimulatorDevice::PrintGPUStats(unsigned int VRAM, unsigned int MSAASampleRate, const std::string& deviceName) const {
		std::cout << std::endl;
		std::cout << "################################################################################################################" << std::endl;
		std::cout << deviceName << std::endl;
		std::cout << "Accesible memory: " << VRAM << " MB" << std::endl;
		std::cout << "MSAA: X" << MSAASampleRate << std::endl;
		std::cout << "################################################################################################################" << std::endl;
	}
#endif

	// Get the maximum MSAA sample count this function can be enabled for smoother graphics
	void MachineSimulatorDevice::getMaxUsableSampleCount(const VkPhysicalDevice& device, VkSampleCountFlagBits& MSAASampleCount) const {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { MSAASampleCount = VK_SAMPLE_COUNT_64_BIT; return; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { MSAASampleCount = VK_SAMPLE_COUNT_32_BIT; return; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { MSAASampleCount = VK_SAMPLE_COUNT_16_BIT; return; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { MSAASampleCount = VK_SAMPLE_COUNT_8_BIT; return; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { MSAASampleCount = VK_SAMPLE_COUNT_4_BIT; return; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { MSAASampleCount = VK_SAMPLE_COUNT_2_BIT; return; }

		MSAASampleCount = VK_SAMPLE_COUNT_1_BIT;
	}

	std::vector<const char*> MachineSimulatorDevice::getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef VALIDATION_LAYERS
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return extensions;
	}

	void MachineSimulatorDevice::createInstance() {
#ifdef VALIDATION_LAYERS
		if (!checkValidationLayerSupport()) throw std::runtime_error("Validation layer requested, but not available!");
#endif
		// Application information is optional but can be usefull for the machine
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "MachineSimulator";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		// Instance createinfo is not optional!
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto requiredExtensions = getRequiredExtensions();

		requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifdef VALIDATION_LAYERS
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
#endif

		if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) throw std::runtime_error("Failed to create instance!");
		std::cout << "Created vulkan instance !" << std::endl;

		// Vulkan has a poor error called VK_ERROR_EXTENSION_NOT_PRESENT
		// In debug modus it can be pleasent to get a list of the available extensions to see which one is missing
#ifdef VALIDATION_LAYERS
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		std::cout << "available extensions:" << std::endl;
		for (const auto& extension : extensions)
			std::cout << '\t' << extension.extensionName << std::endl;

		this->MSWindow.createWindowSurface(instance_, &surface);
#endif
	}

#ifdef VALIDATION_LAYERS

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void MachineSimulatorDevice::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void MachineSimulatorDevice::setupDebugMessenger() {
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) 
			throw std::runtime_error("failed to set up debug messenger!");
	}

	bool MachineSimulatorDevice::checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}
		return true;
	}
#endif
}