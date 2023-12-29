#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include <string>

namespace lve {
	class LveWindow {
	public:
		LveWindow(int w, int h, std::string name);
		~LveWindow();

		LveWindow(const LveWindow&) = delete;
		LveWindow& operator=(const LveWindow&) = delete;

		inline bool ShouldClose() { return glfwWindowShouldClose(window); }
		inline VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		inline bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		inline GLFWwindow* getGLFWwindow() const { return window; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private :
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();
		GLFWwindow* window;

		int width;
		int height;
		bool framebufferResized = false;

		std::string windowName;
	};
}