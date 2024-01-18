//#####################################################################################################################################################
//#####################################################################################################################################################
// ___       ___                 ___                                         ____                              ___                                 
// `MMb     dMM'                 `MM      68b                               6MMMMb\68b                         `MM                                 
//  MMM.   ,PMM                   MM      Y89                              6M'    `Y89                          MM           /                     
//  M`Mb   d'MM    ___     ____   MM  __  ___ ___  __     ____             MM      ___ ___  __    __  ___   ___ MM    ___   /M      _____  ___  __ 
//  M YM. ,P MM  6MMMMb   6MMMMb. MM 6MMb `MM `MM 6MMb   6MMMMb            YM.     `MM `MM 6MMb  6MMb `MM    MM MM  6MMMMb /MMMMM  6MMMMMb `MM 6MM 
//  M `Mb d' MM 8M'  `Mb 6M'   Mb MMM9 `Mb MM  MMM9 `Mb 6M'  `Mb            YMMMMb  MM  MM69 `MM69 `Mb MM    MM MM 8M'  `Mb MM    6M'   `Mb MM69 " 
//  M  YM.P  MM     ,oMM MM    `' MM'   MM MM  MM'   MM MM    MM                `Mb MM  MM'   MM'   MM MM    MM MM     ,oMM MM    MM     MM MM'    
//  M  `Mb'  MM ,6MM9'MM MM       MM    MM MM  MM    MM MMMMMMMM                 MM MM  MM    MM    MM MM    MM MM ,6MM9'MM MM    MM     MM MM     
//  M   YP   MM MM'   MM MM       MM    MM MM  MM    MM MM                       MM MM  MM    MM    MM MM    MM MM MM'   MM MM    MM     MM MM     
//  M   `'   MM MM.  ,MM YM.   d9 MM    MM MM  MM    MM YM    d9           L    ,M9 MM  MM    MM    MM YM.   MM MM MM.  ,MM YM.  ,YM.   ,M9 MM     
// _M_      _MM_`YMMM9'Yb.YMMMM9 _MM_  _MM_MM__MM_  _MM_ YMMMM9            MYMMMM9 _MM__MM_  _MM_  _MM_ YMMM9MM_MM_`YMMM9'Yb.YMMM9 YMMMMM9 _MM_   
//                                                                         
// 
//                                                    .oooooo.                                
//                                                   d8P'  `Y8b                               
//                                                  888           .ooooo.  oooo d8b  .ooooo.  
//                                                  888          d88' `88b `888""8P d88' `88b 
//                                                  888          888   888  888     888ooo888 
//                                                  `88b    ooo  888   888  888     888    .o 
//                                                   `Y8bood8P'  `Y8bod8P' d888b    `Y8bod8P' 
//
//#####################################################################################################################################################
//#####################################################################################################################################################
// 
// This is the static library of the machine simulator with this core multiple application of Vulkan can easely be created

#ifndef MACHINESIMULATOR_HPP
#define MACHINESIMULATOR_HPP

#pragma region Includes
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <ShellScalingApi.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <array>
// Better than std::unordered_map<>()
#include "HashMap/unordered_dense.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <numeric>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <string>
#include <iostream>

#include "VulkanDebug.hpp"
#include "VulkanTools.hpp"
#include "CommandLineParser.hpp"
#include "Keycodes.hpp"
#include "MachineSimulatorDevice.hpp"
#include "MachineSimulatorSwapChain.hpp"
#include "MachineSimulatorCamera.hpp"
#include "VulkanUIOverlay.hpp"
#pragma endregion

namespace Voortman {
	class MachineSimulatorCore {
	public:
#pragma region PublicMembers
		bool prepared = false;
		bool resized = false;
		bool viewUpdated = false;
		uint32_t width = 1280;
		uint32_t height = 720;

		vks::UIOverlay UIOverlay;

		static std::vector<const char*> args;
		float timer = 0.0f;
		float timerSpeed = 0.25f;
		float frameTimer = 1.0f;
		bool paused = false;

		MachineSimulatorCamera camera;
		glm::vec2 mousePos;

		MachineSimulatorDevice* vulkanDevice;

		std::string name = "Machine Simulator";
		std::string title = "Machine Simulator";
		uint32_t apiVersion = VK_API_VERSION_1_3;

		VkClearColorValue defaultClearColor = { {0.025f, 0.025f, 0.025f, 1.0f} };

		CommandLineParser commandLineParser;

#pragma region GlobalStructs
		struct Settings {
			bool validation = false;
			bool fullscreen = false;
			bool vsync = false;
			bool overlay = true;
		} settings;

		struct {
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		} depthStencil;

		struct {
			bool left = false;
			bool right = false;
			bool middle = false;
		} mouseButtons;
#pragma endregion

		HWND window;
		HINSTANCE windowInstance;

		MachineSimulatorCore();
		virtual ~MachineSimulatorCore();

		bool initVulkan();

		void setupConsole(std::string title);
		void setupDPIAwareness();
		HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
		void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void windowResize();

#pragma region PublicVirtual
		virtual VkResult createInstance(bool enableValidation);
		virtual void getEnabledFeatures();
		virtual void getEnabledExtensions();
		virtual void keyPressed(uint32_t);

		virtual void OnHandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual void buildCommandBuffers();
		virtual void windowResized();

		// Pure virtual render function
		virtual void render() = 0;

		virtual void viewChanged();
		virtual void mouseMoved(double x, double y, bool& handled);
		virtual void setupRenderPass();
		virtual void setupFrameBuffer();
		virtual void setupDepthStencil();
		virtual void prepare();
#pragma endregion
#pragma endregion
	protected:
#pragma region ProtectedMembers
		std::string getShadersPath() const;

		uint32_t frameCounter{ 0 };
		uint32_t lastFPS{ 0 };
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;
		std::chrono::time_point<std::chrono::high_resolution_clock> tPrevEnd;

		VkInstance instance{ VK_NULL_HANDLE };

		std::vector<std::string> supportedInstanceExtensions;
		std::vector<const char*> enabledDeviceExtensions;
		std::vector<const char*> enabledInstanceExtensions;

		void* deviceCreatepNextChain = nullptr;
		VkDevice device{ VK_NULL_HANDLE };
		VkQueue queue{ VK_NULL_HANDLE };

		// Can also be a vector of physical devices if the work load is spreaded efficiently
		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		VkPhysicalDeviceProperties deviceProperties{};
		VkPhysicalDeviceFeatures deviceFeatures{};
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
		VkPhysicalDeviceFeatures enabledFeatures{};
		std::vector<VkFramebuffer> frameBuffers;

		MachineSimulatorSwapChain swapChain;

		VkFormat depthFormat;
		VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo;
		VkCommandPool cmdPool{ VK_NULL_HANDLE };

		struct {
			VkSemaphore presentComplete;
			VkSemaphore renderComplete;
		} semaphores;
		std::vector<VkFence> waitFences;

		bool requiresStencil{ false };
#pragma endregion
	private:
#pragma region PrivateMembers
		void handleMouseMove(int32_t x, int32_t y);
		void setupSwapChain();
		void createCommandBuffers();
		void destroyCommandBuffers();
		void nextFrame();
		void updateOverlay();
		void createPipelineCache();
		void createCommandPool();
		void createSynchronizationPrimitives();
		void initSwapChain();

		uint32_t destWidth;
		uint32_t destHeight;
		bool resizing = false;

		std::string shaderDir = "glsl";
#pragma endregion
	};
}

#pragma region MainFunction
#define MachineSimulatorCoreMain()
Voortman::MachineSimulatorApp* machineSimulator;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (!machineSimulator) {
		machineSimulator->handleMessages(hWnd, uMsg, wParam, lParam);
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	for (int32_t i = 0; i < __argc; i++)
		Voortman::MachineSimulatorCore::args.push_back(__argv[i]);

	machineSimulator = new Voortman::MachineSimulatorApp();
	machineSimulator->initVulkan();

	delete(machineSimulator);
	return 0;
}
#pragma endregion

#endif // MACHINESIMULATORCORE_HPP