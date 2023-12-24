#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string>

#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

namespace Voortman {
	class SteelSightWindow {
	public:
		SteelSightWindow(int w, int h, const std::string& name);
		~SteelSightWindow();

		SteelSightWindow(const SteelSightWindow&) = delete;
		SteelSightWindow& operator=(const SteelSightWindow&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() const { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		bool wasWindowResized() const { return framebufferResized; }
		inline void resetWindowResizedFlag() { framebufferResized = false; }
		GLFWwindow* getGLFWwindow() const { return window; }

	private:
		GLFWwindow* window;
		int width{ 0 };
		int height{ 0 };
		bool framebufferResized = false;
		std::string windowName{};

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();
	};
}