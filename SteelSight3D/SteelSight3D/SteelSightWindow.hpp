#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string>

namespace Voortman {
	class SteelSightWindow final {
	public:
		SteelSightWindow(const int w, const int h, const std::string& name);
		~SteelSightWindow();

		SteelSightWindow(const SteelSightWindow&) = delete;
		SteelSightWindow& operator=(const SteelSightWindow&) = delete;

		inline bool ShouldClose() const noexcept { return glfwWindowShouldClose(window); }
		inline VkExtent2D getExtent() const noexcept { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		inline bool wasWindowResized() const noexcept { return framebufferResized; }
		inline void resetWindowResizedFlag() noexcept { framebufferResized = false; }
		inline GLFWwindow* getGLFWwindow() const noexcept { return window; }

		void createWindowSurface(const VkInstance& instance, VkSurfaceKHR* surface);

	private:
		// Static can come with performance advantage when static
		static inline void framebufferResizeCallback(GLFWwindow* window, const int width, const int height);
		void initWindow();

		GLFWwindow* window{ nullptr };

		int width{ 0 };
		int height{ 0 };
		bool framebufferResized{ false };

		std::string windowName{};
	};
}