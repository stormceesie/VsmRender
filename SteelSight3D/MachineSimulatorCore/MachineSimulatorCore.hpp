#pragma once
#include "pch.h"

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <ShellScalingAPI.h>
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <sys/system_properties.h>
#include "VulkanAndroid.h"
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
#include <directfb.h>
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#elif defined(_DIRECT2DISPLAY)
//
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#include <xcb/xcb.h>
#endif

#include "HelperFunctions/CommandLineParser.hpp"
#include "HelperFunctions/keycodes.hpp"
#include "HelperFunctions/VulkanTools.hpp"
#include "HelperFunctions/VulkanDebugger.hpp"
#include "VulkanUIOverlay.h"
#include "MachineSimulatorSwapChain.hpp"
#include "HelperFunctions/VulkanBuffer.hpp"
#include "HelperFunctions/VulkanDevice.hpp"
#include "VulkanTexture.h"

#include "HelperFunctions/VulkanInitializers.hpp"
#include "camera.hpp"
#include "HelperFunctions/benchmark.hpp"

namespace Voortman {
	class MachineSimulatorCode {
	public:

	protected:
		std::string sGetShaderPath() const;
		uint32_t nFrameCounter{ 0 };
		uint32_t nLastFPS{ 0 };
		std::chrono::time_point<std::chrono::high_resolution_clock> tLastTimestamp{};
		std::chrono::time_point<std::chrono::high_resolution_clock> tPrevEnd{};

		std::vector<std::string> vSupportedInstanceExtensions{};

		std::vector<const char*> vEnabledDeviceExtensions{};
		std::vector<const char*> vEnabledInstanceExtensions{};

		// Vulkan variables order can matter because of the deletion when the object goes out of scope
		VkInstance Instance{ VK_NULL_HANDLE };
		VkPhysicalDevice PhysicalDevice{ VK_NULL_HANDLE }; // Physical GPU device can even be extended to a vector of GPU devices to devide the CommandBuffer over multiple devices
		VkPhysicalDeviceMemoryProperties DeviceMemoryProperties{};
		VkPhysicalDeviceFeatures DeviceFeatures{};
		VkPhysicalDeviceMemoryProperties DeviceMemoryProperties{};
		VkPhysicalDeviceFeatures EnabledFeatures{};

		void* DeviceCreateNextChain{ nullptr };

		VkDevice Device{ VK_NULL_HANDLE };
		VkQueue Queue{ VK_NULL_HANDLE };
		VkFormat DepthFormat{};
		VkCommandPool CommandPool{ VK_NULL_HANDLE };
		VkPipelineStageFlags SubmitPipelineStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_BIT };
		VkSubmitInfo SubmitInfo{};
		std::vector<VkCommandBuffer> DrawCmdBuffers{};
		VkRenderPass RenderPass{ VK_NULL_HANDLE };
		std::vector<VkFramebuffer> FrameBuffers{};
		uint32_t CurrentBuffer{ 0 };
		VkDescriptorPool DescriptorPool{ VK_NULL_HANDLE };
		std::vector<VkShaderModule> ShaderModules{};

		VkPipelineCache PipelineCache{ VK_NULL_HANDLE };


	private:
		// Private functions
		std::string GetWindowTitle();
		void HandleMouseMove(uint32_t x, int32_t y);
		void NextFrame();
		void UpdateOverlay();
		void CreatePipelineCache();
		void CreateCommandPool();
		void CreateSynchronizationPrimitives();
		void InitSwapChain();
		void SetupSwapChain();
		void CreateCommandBuffers();
		void DestroyCommandBuffers();

		// Member variables
		uint32_t nDestinationWidth{0};
		uint32_t nDestinationHeight{0};
		bool bWindowResizing{ false };
		std::string sShaderDir{"glsl"};
	};
}