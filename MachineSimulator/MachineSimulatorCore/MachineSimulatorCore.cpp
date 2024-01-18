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

		vulkanDevice = new MachineSimulatorDevice(physicalDevice);

		getEnabledExtensions();

		VkResult res = vulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, deviceCreatepNextChain);
		if (res != VK_SUCCESS) {
			vks::tools::exitFatal("Could not create Vulkan device: \n" + vks::tools::errorString(res), res);
			return false;
		}

		device = vulkanDevice->logicalDevice;

		vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.graphics, 0, &queue);

		VkBool32 validFormat{ false };

		if (requiresStencil) {
			validFormat = vks::tools::getSupportedDepthStencilFormat(physicalDevice, &depthFormat);
		}
		else {
			validFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
		}

		assert(validFormat);

		swapChain.connect(instance, physicalDevice, device);

		VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();

		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));

		submitInfo = vks::initializers::submitInfo();
		submitInfo.pWaitDstStageMask = &submitPipelineStages;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &semaphores.presentComplete;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &semaphores.renderComplete;

		return true;
	}

	// Functions that can be overwritten bij the app
	void MachineSimulatorCore::getEnabledFeatures() {}
	void MachineSimulatorCore::getEnabledExtensions() {}

	void MachineSimulatorCore::keyPressed(uint32_t) {}
	void MachineSimulatorCore::mouseMoved(double x, double y, bool& handled) {}

	void MachineSimulatorCore::handleMouseMove(int32_t x, int32_t y) {
		int32_t dx = (int32_t)mousePos.x - x;
		int32_t dy = (int32_t)mousePos.y - y;

		bool handled = false;

		if (settings.overlay) {
			ImGuiIO& io = ImGui::GetIO();
			handled = io.WantCaptureMouse && UIOverlay.visible;
		}
		mouseMoved((float)x, (float)y, handled);

		if (handled) {
			mousePos = glm::vec2((float)x, (float)y);
			return;
		}

		if (mouseButtons.left) {
			camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
			viewUpdated = true;
		}
		if (mouseButtons.right) {
			camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
			viewUpdated = true;
		}
		if (mouseButtons.middle) {
			camera.translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
			viewUpdated = true;
		}
		mousePos = glm::vec2((float)x, (float)y);
	}

	void MachineSimulatorCore::setupSwapChain() {
		swapChain.create(&width, &height, settings.vsync, settings.fullscreen);
	}

	void MachineSimulatorCore::initSwapChain() {
		swapChain.initSurface(windowInstance, window);
	}

	void MachineSimulatorCore::setupDepthStencil() {
		VkImageCreateInfo imageCI{};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = depthFormat;
		imageCI.extent = { width, height, 1 };
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = 1;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &depthStencil.image));
		VkMemoryRequirements memReqs{};
		vkGetImageMemoryRequirements(device, depthStencil.image, &memReqs);

		VkMemoryAllocateInfo memAllloc{};
		memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllloc.allocationSize = memReqs.size;
		memAllloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllloc, nullptr, &depthStencil.mem));
		VK_CHECK_RESULT(vkBindImageMemory(device, depthStencil.image, depthStencil.mem, 0));

		VkImageViewCreateInfo imageViewCI{};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.image = depthStencil.image;
		imageViewCI.format = depthFormat;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
		if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
			imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCI, nullptr, &depthStencil.view));
	}

	void MachineSimulatorCore::buildCommandBuffers() {}

	void MachineSimulatorCore::windowResized() {}

	void MachineSimulatorCore::windowResize() {
		if (!prepared)
		{
			return;
		}
		prepared = false;
		resized = true;

		// Ensure all operations on the device have been finished before destroying resources
		vkDeviceWaitIdle(device);

		// Recreate swap chain
		width = destWidth;
		height = destHeight;
		setupSwapChain();

		// Recreate the frame buffers
		vkDestroyImageView(device, depthStencil.view, nullptr);
		vkDestroyImage(device, depthStencil.image, nullptr);
		vkFreeMemory(device, depthStencil.mem, nullptr);
		setupDepthStencil();
		for (uint32_t i = 0; i < frameBuffers.size(); i++) {
			vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
		}
		setupFrameBuffer();

		if ((width > 0.0f) && (height > 0.0f)) {
			if (settings.overlay) {
				UIOverlay.resize(width, height);
			}
		}

		// Command buffers need to be recreated as they may store
		// references to the recreated frame buffer
		destroyCommandBuffers();
		createCommandBuffers();
		buildCommandBuffers();

		// SRS - Recreate fences in case number of swapchain images has changed on resize
		for (auto& fence : waitFences) {
			vkDestroyFence(device, fence, nullptr);
		}
		createSynchronizationPrimitives();

		vkDeviceWaitIdle(device);

		if ((width > 0.0f) && (height > 0.0f)) {
			camera.updateAspectRatio((float)width / (float)height);
		}

		// Notify derived class
		windowResized();
		viewChanged();

		prepared = true;
	}

	void MachineSimulatorCore::createCommandPool() {
		VkCommandPoolCreateInfo cmdPoolInfo{};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool));
	}

	// Broken function needs to be fixed first
	void MachineSimulatorCore::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg)
		{
		case WM_CLOSE:
			prepared = false;
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		case WM_PAINT:
			ValidateRect(window, NULL);
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case KEY_P:
				paused = !paused;
				break;
			case KEY_F1:
				UIOverlay.visible = !UIOverlay.visible;
				UIOverlay.updated = true;
				break;
			case KEY_ESCAPE:
				PostQuitMessage(0);
				break;
			}

			if (camera.type == MachineSimulatorCamera::firstperson)
			{
				switch (wParam)
				{
				case KEY_W:
					camera.keys.up = true;
					break;
				case KEY_S:
					camera.keys.down = true;
					break;
				case KEY_A:
					camera.keys.left = true;
					break;
				case KEY_D:
					camera.keys.right = true;
					break;
				}
			}

			keyPressed((uint32_t)wParam);
			break;
		case WM_KEYUP:
			if (camera.type == MachineSimulatorCamera::firstperson)
			{
				switch (wParam)
				{
				case KEY_W:
					camera.keys.up = false;
					break;
				case KEY_S:
					camera.keys.down = false;
					break;
				case KEY_A:
					camera.keys.left = false;
					break;
				case KEY_D:
					camera.keys.right = false;
					break;
				}
			}
			break;
		case WM_LBUTTONDOWN:
			mousePos = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
			mouseButtons.left = true;
			break;
		case WM_RBUTTONDOWN:
			mousePos = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
			mouseButtons.right = true;
			break;
		case WM_MBUTTONDOWN:
			mousePos = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
			mouseButtons.middle = true;
			break;
		case WM_LBUTTONUP:
			mouseButtons.left = false;
			break;
		case WM_RBUTTONUP:
			mouseButtons.right = false;
			break;
		case WM_MBUTTONUP:
			mouseButtons.middle = false;
			break;
		case WM_MOUSEWHEEL:
		{
			short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			camera.translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f));
			viewUpdated = true;
			break;
		}
		case WM_MOUSEMOVE:
		{
			handleMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;
		}
		case WM_SIZE:
			if ((prepared) && (wParam != SIZE_MINIMIZED))
			{
				if ((resizing) || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
				{
					destWidth = LOWORD(lParam);
					destHeight = HIWORD(lParam);
					windowResize();
				}
			}
			break;
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO minMaxInfo = (LPMINMAXINFO)lParam;
			minMaxInfo->ptMinTrackSize.x = 64;
			minMaxInfo->ptMinTrackSize.y = 64;
			break;
		}
		case WM_ENTERSIZEMOVE:
			resizing = true;
			break;
		case WM_EXITSIZEMOVE:
			resizing = false;
			break;
		}

		OnHandleMessage(hWnd, uMsg, wParam, lParam);
	}

	VkResult MachineSimulatorCore::createInstance(bool enableValidation) {
		this->settings.validation = enableValidation;

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name.c_str();
		appInfo.pEngineName = name.c_str();
		appInfo.apiVersion = apiVersion;

		std::vector<const char*> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};

		instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		uint32_t extCount{ 0 };
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

		if (extCount > 0) {
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
				for (VkExtensionProperties& extension : extensions)
					supportedInstanceExtensions.push_back(extension.extensionName);
		}

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