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

// Region for system specific includes
#pragma region SystemSpecificInclude
#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <ShellScalingApi.h>
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
#pragma endregion

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

namespace Voortman {
	class MachineSimulatorCore {
	public:
		bool prepared = false;
		bool resized = false;
		bool viewUpdated = false;
		uint32_t width = 1280;
		uint32_t height = 720;

		static std::vector<const char*> args;
		float timer = 0.0f;
		float timerSpeed = 0.25f;
		float frameTimer = 1.0f;
		bool paused = false;

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
			bool middel = false;
		} mouseButtons;
#pragma endregion

#pragma region PlatformSpecificVariables
#if defined(_WIN32)
		HWND window;
		HINSTANCE windowInstance;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		// true if application has focused, false if moved to background
		bool focused = false;
		struct TouchPos {
			int32_t x;
			int32_t y;
		} touchPos;
		bool touchDown = false;
		double touchTimer = 0.0;
		int64_t lastTapTime = 0;
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
		void* view;
#if defined(VK_EXAMPLE_XCODE_GENERATED)
		bool quit = false;
#endif
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
		bool quit = false;
		IDirectFB* dfb = nullptr;
		IDirectFBDisplayLayer* layer = nullptr;
		IDirectFBWindow* window = nullptr;
		IDirectFBSurface* surface = nullptr;
		IDirectFBEventBuffer* event_buffer = nullptr;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		wl_display* display = nullptr;
		wl_registry* registry = nullptr;
		wl_compositor* compositor = nullptr;
		struct xdg_wm_base* shell = nullptr;
		wl_seat* seat = nullptr;
		wl_pointer* pointer = nullptr;
		wl_keyboard* keyboard = nullptr;
		wl_surface* surface = nullptr;
		struct xdg_surface* xdg_surface;
		struct xdg_toplevel* xdg_toplevel;
		bool quit = false;
		bool configured = false;

#elif defined(_DIRECT2DISPLAY)
		bool quit = false;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		bool quit = false;
		xcb_connection_t* connection;
		xcb_screen_t* screen;
		xcb_window_t window;
		xcb_intern_atom_reply_t* atom_wm_delete_window;
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
		bool quit = false;
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
		screen_context_t screen_context = nullptr;
		screen_window_t screen_window = nullptr;
		screen_event_t screen_event = nullptr;
		bool quit = false;
#endif
#pragma endregion

		MachineSimulatorCore();
		virtual ~MachineSimulatorCore();

		bool initVulkan();

#pragma region PlatformSpecificFunctions
#if defined(_WIN32)
		void setupConsole(std::string title);
		void setupDPIAwareness();
		HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
		void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		static int32_t handleAppInput(struct android_app* app, AInputEvent* event);
		static void handleAppCommand(android_app* app, int32_t cmd);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
		void* setupWindow(void* view);
		void displayLinkOutputCb();
		void mouseDragged(float x, float y);
		void windowWillResize(float x, float y);
		void windowDidResize();
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
		IDirectFBSurface* setupWindow();
		void handleEvent(const DFBWindowEvent* event);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		struct xdg_surface* setupWindow();
		void initWaylandConnection();
		void setSize(int width, int height);
		static void registryGlobalCb(void* data, struct wl_registry* registry,
			uint32_t name, const char* interface, uint32_t version);
		void registryGlobal(struct wl_registry* registry, uint32_t name,
			const char* interface, uint32_t version);
		static void registryGlobalRemoveCb(void* data, struct wl_registry* registry,
			uint32_t name);
		static void seatCapabilitiesCb(void* data, wl_seat* seat, uint32_t caps);
		void seatCapabilities(wl_seat* seat, uint32_t caps);
		static void pointerEnterCb(void* data, struct wl_pointer* pointer,
			uint32_t serial, struct wl_surface* surface, wl_fixed_t sx,
			wl_fixed_t sy);
		static void pointerLeaveCb(void* data, struct wl_pointer* pointer,
			uint32_t serial, struct wl_surface* surface);
		static void pointerMotionCb(void* data, struct wl_pointer* pointer,
			uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
		void pointerMotion(struct wl_pointer* pointer,
			uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
		static void pointerButtonCb(void* data, struct wl_pointer* wl_pointer,
			uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
		void pointerButton(struct wl_pointer* wl_pointer,
			uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
		static void pointerAxisCb(void* data, struct wl_pointer* wl_pointer,
			uint32_t time, uint32_t axis, wl_fixed_t value);
		void pointerAxis(struct wl_pointer* wl_pointer,
			uint32_t time, uint32_t axis, wl_fixed_t value);
		static void keyboardKeymapCb(void* data, struct wl_keyboard* keyboard,
			uint32_t format, int fd, uint32_t size);
		static void keyboardEnterCb(void* data, struct wl_keyboard* keyboard,
			uint32_t serial, struct wl_surface* surface, struct wl_array* keys);
		static void keyboardLeaveCb(void* data, struct wl_keyboard* keyboard,
			uint32_t serial, struct wl_surface* surface);
		static void keyboardKeyCb(void* data, struct wl_keyboard* keyboard,
			uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
		void keyboardKey(struct wl_keyboard* keyboard,
			uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
		static void keyboardModifiersCb(void* data, struct wl_keyboard* keyboard,
			uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
			uint32_t mods_locked, uint32_t group);

#elif defined(_DIRECT2DISPLAY)
		//
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		xcb_window_t setupWindow();
		void initxcbConnection();
		void handleEvent(const xcb_generic_event_t* event);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
		void setupWindow();
		void handleEvent();
#else
		void setupWindow();
#endif
#pragma endregion

		virtual VkResult createInstance(bool enableValidation);
		virtual void getEnabledFeatures();

#ifdef _WIN32
		virtual void OnHandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

	protected:
		std::string getShadersPath() const;

		uint32_t frameCounter{ 0 };
		uint32_t lastFPS{ 0 };
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;
		std::chrono::time_point<std::chrono::high_resolution_clock> tPrevEnd;

		VkInstance instance{ VK_NULL_HANDLE };

		std::vector<std::string> supportedInstanceExtensions;
		std::vector<const char*> enabledDeviceExtensions;
		std::vector<const char*> enabledInstanceExtensions;

		// Can also be a vector of physical devices if the work load is spreaded efficiently
		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		VkPhysicalDeviceProperties deviceProperties{};
		VkPhysicalDeviceFeatures deviceFeatures{};
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
		VkPhysicalDeviceFeatures enabledFeatures{};

	private:
		inline constexpr std::string getWindowTitle() const noexcept;

		uint32_t destWidth;
		uint32_t destHeight;
		bool resizing = false;

		std::string shaderDir = "glsl";
	};

#if defined(_WIN32)
#define MachineSimulatorCoreMain()
	MachineSimulatorCore* machineSimulator;
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (!machineSimulator) {
			machineSimulator->handleMessages(hWnd, uMsg, wParam, lParam);
		}
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}

	int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
		for (int32_t i = 0; i < __argc; i++) 
			MachineSimulatorCore::args.push_back(__argv[i]);

		machineSimulator = new MachineSimulatorCore();
		machineSimulator->initVulkan();

		delete(machineSimulator);
		return 0;
	}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	// Android entry point
#define VULKAN_EXAMPLE_MAIN()																		\
VulkanExample *vulkanExample;																		\
void android_main(android_app* state)																\
{																									\
	vulkanExample = new VulkanExample();															\
	state->userData = vulkanExample;																\
	state->onAppCmd = VulkanExample::handleAppCommand;												\
	state->onInputEvent = VulkanExample::handleAppInput;											\
	androidApp = state;																				\
	vks::android::getDeviceConfig();																\
	vulkanExample->renderLoop();																	\
	delete(vulkanExample);																			\
}
#elif defined(_DIRECT2DISPLAY)
	// Linux entry point with direct to display wsi
#define VULKAN_EXAMPLE_MAIN()																		\
VulkanExample *vulkanExample;																		\
static void handleEvent()                                											\
{																									\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { VulkanExample::args.push_back(argv[i]); };  				\
	vulkanExample = new VulkanExample();															\
	vulkanExample->initVulkan();																	\
	vulkanExample->prepare();																		\
	vulkanExample->renderLoop();																	\
	delete(vulkanExample);																			\
	return 0;																						\
}
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
#define VULKAN_EXAMPLE_MAIN()																		\
VulkanExample *vulkanExample;																		\
static void handleEvent(const DFBWindowEvent *event)												\
{																									\
	if (vulkanExample != NULL)																		\
	{																								\
		vulkanExample->handleEvent(event);															\
	}																								\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { VulkanExample::args.push_back(argv[i]); };  				\
	vulkanExample = new VulkanExample();															\
	vulkanExample->initVulkan();																	\
	vulkanExample->setupWindow();					 												\
	vulkanExample->prepare();																		\
	vulkanExample->renderLoop();																	\
	delete(vulkanExample);																			\
	return 0;																						\
}
#elif (defined(VK_USE_PLATFORM_WAYLAND_KHR) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
#define VULKAN_EXAMPLE_MAIN()																		\
VulkanExample *vulkanExample;																		\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { VulkanExample::args.push_back(argv[i]); };  				\
	vulkanExample = new VulkanExample();															\
	vulkanExample->initVulkan();																	\
	vulkanExample->setupWindow();					 												\
	vulkanExample->prepare();																		\
	vulkanExample->renderLoop();																	\
	delete(vulkanExample);																			\
	return 0;																						\
}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#define VULKAN_EXAMPLE_MAIN()																		\
VulkanExample *vulkanExample;																		\
static void handleEvent(const xcb_generic_event_t *event)											\
{																									\
	if (vulkanExample != NULL)																		\
	{																								\
		vulkanExample->handleEvent(event);															\
	}																								\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { VulkanExample::args.push_back(argv[i]); };  				\
	vulkanExample = new VulkanExample();															\
	vulkanExample->initVulkan();																	\
	vulkanExample->setupWindow();					 												\
	vulkanExample->prepare();																		\
	vulkanExample->renderLoop();																	\
	delete(vulkanExample);																			\
	return 0;																						\
}
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
#if defined(VK_EXAMPLE_XCODE_GENERATED)
#define VULKAN_EXAMPLE_MAIN()																		\
VulkanExample *vulkanExample;																		\
int main(const int argc, const char *argv[])														\
{																									\
	@autoreleasepool																				\
	{																								\
		for (size_t i = 0; i < argc; i++) { VulkanExample::args.push_back(argv[i]); };				\
		vulkanExample = new VulkanExample();														\
		vulkanExample->initVulkan();																\
		vulkanExample->setupWindow(nullptr);														\
		vulkanExample->prepare();																	\
		vulkanExample->renderLoop();																\
		delete(vulkanExample);																		\
	}																								\
	return 0;																						\
}
#else
#define VULKAN_EXAMPLE_MAIN()
#endif

#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
#define VULKAN_EXAMPLE_MAIN()												\
VulkanExample *vulkanExample;												\
int main(const int argc, const char *argv[])										\
{															\
	for (int i = 0; i < argc; i++) { VulkanExample::args.push_back(argv[i]); };					\
	vulkanExample = new VulkanExample();										\
	vulkanExample->initVulkan();											\
	vulkanExample->setupWindow();											\
	vulkanExample->prepare();											\
	vulkanExample->renderLoop();											\
	delete(vulkanExample);												\
	return 0;													\
}
#endif
}

#endif