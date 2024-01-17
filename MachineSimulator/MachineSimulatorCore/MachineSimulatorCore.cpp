#include "MachineSimulatorCore.hpp"

namespace Voortman {
	std::vector<const char*> MachineSimulatorCore::args;

	bool MachineSimulatorCore::initVulkan() {
		VkResult err;

		err = createInstance(settings.validation);
		if (err) {
			vks::tools::exitFatal("Could not create Vulkan instance : \n" + vks::tools::errorString(err), err);
			return false;
		}

		if (settings.validation) {
			vks::debug::setupDebugging(instance);
		}

		uint32_t gpuCount{ 0 };
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
		if (gpuCount == 0) {
			vks::tools::exitFatal("No device with Vulkan support found!", -1);
			return false;
		}

		std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
		err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
		if (err) {
			vks::tools::exitFatal("Could not enumerate physical devices : \n" + vks::tools::errorString(err), err);
			return false;
		}

		uint32_t selectedDevice = 0;

		if (commandLineParser.isSet("gpuselection")) {
			uint32_t index = commandLineParser.getValueAsInt("gpuselection", 0);
			if (index > gpuCount - 1) {
				std::cerr << "Selected device index " << index << "is out of range, reverting to device 0 (use -listgpus to show available Vulkan devices)" << std::endl;
			}
			else {
				selectedDevice = index;
			}
		}
		if (commandLineParser.isSet("gpulist")) {
			std::cout << "Available Vulkan devices" << std::endl;
			for (uint32_t i = 0; i < gpuCount; i++) {
				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
				std::cout << "Device [" << i << "] : " << deviceProperties.deviceName << std::endl;
				std::cout << " Type: " << vks::tools::physicalDeviceTypeString(deviceProperties.deviceType) << std::endl;
				std::cout << " API: " << (deviceProperties.apiVersion >> 22) << "." << ((deviceProperties.apiVersion >> 12) & 0x3FF) << "." << (deviceProperties.apiVersion & 0xFFF) << std::endl;
			}
		}

		physicalDevice = physicalDevices[selectedDevice];

		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

		getEnabledFeatures();

	}

	void MachineSimulatorCore::getEnabledFeatures() {}

	VkResult MachineSimulatorCore::createInstance(bool enableValidation) {
		this->settings.validation = enableValidation;

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name.c_str();
		appInfo.pEngineName = name.c_str();
		appInfo.apiVersion = apiVersion;

		std::vector<const char*> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};

#pragma region PlatformSpecificPushback
#if defined(_WIN32)
		instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
		instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
		instanceExtensions.push_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
		instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
		instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
		instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
		instanceExtensions.push_back(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME);
#endif
#pragma endregion

		uint32_t extCount{ 0 };
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

		if (extCount > 0) {
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
				for (VkExtensionProperties& extension : extensions)
					supportedInstanceExtensions.push_back(extension.extensionName);
		}

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
		// SRS - When running on iOS/macOS with MoltenVK, enable VK_KHR_get_physical_device_properties2 if not already enabled by the example (required by VK_KHR_portability_subset)
		if (std::find(enabledInstanceExtensions.begin(), enabledInstanceExtensions.end(), VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == enabledInstanceExtensions.end())
		{
			enabledInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}
#endif

		if (supportedInstanceExtensions.size() > 0) {
			for (const char* enabledExtension : enabledInstanceExtensions) {
				if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), enabledExtension) == supportedInstanceExtensions.end()) {
					std::cerr << "Enabled instance extensions " << enabledExtension << " is not present at instance level" << std::endl;
				}
				instanceExtensions.push_back(enabledExtension);
			}
		}

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.pApplicationInfo = &appInfo;

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK)) && defined(VK_KHR_portability_enumeration)
		// SRS - When running on iOS/macOS with MoltenVK and VK_KHR_portability_enumeration is defined and supported by the instance, enable the extension and the flag
		if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) != supportedInstanceExtensions.end())
		{
			instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
			instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		}
#endif

		// Enable the debug utils extension if available (e.g. when debugging tools are present)
		if (settings.validation || std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end()) {
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		if (instanceExtensions.size() > 0)
		{
			instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
			instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
		}

		// Vulkan does not support debug feedback because of performance reasons
		// LunarG did create some debug feedback in Vulkan so we can use that
		const char* validationLayerName = "VK_LAYER_KHRONOS_validation";

		if (settings.validation) {
			uint32_t instanceLayerCount;
			vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
			std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
			vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
			bool validationLayerPresent = false;
			for (VkLayerProperties& layer : instanceLayerProperties) {
				if (!strcmp(layer.layerName, validationLayerName)) {
					validationLayerPresent = true;
					break;
				}
			}
			if (validationLayerPresent) {
				instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
				instanceCreateInfo.enabledLayerCount = 1;
			}
			else {
				std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled!" << std::endl;
			}
		}

		VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

		// If the debug util is present -> setup the debugutils
		if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end()) {
			vks::debugutils::setup(instance);
		}

		return result;
	}
}