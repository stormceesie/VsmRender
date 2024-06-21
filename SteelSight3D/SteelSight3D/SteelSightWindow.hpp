#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string>

namespace Voortman {
	/// <summary>
	/// Class that handles all the interactions with the window
	/// </summary>
	class SteelSightWindow final {
	public:
		/// <summary>
		/// The SteelSightWindow constructor
		/// </summary>
		/// <param name="w">The initial width of the window.</param>
		/// <param name="h">The initial height of the window.</param>
		/// <param name="name">The name of the window.</param>
		SteelSightWindow(const int w, const int h, const std::string& name);

		/// <summary>
		/// The deconstructor of the SteelSightWindow class.
		/// </summary>
		~SteelSightWindow();

		/// <summary>
		/// Delete this constructor
		/// </summary>
		/// <param name=""></param>
		SteelSightWindow(const SteelSightWindow&) = delete;

		/// <summary>
		/// Delete this operator
		/// </summary>
		/// <param name=""></param>
		/// <returns></returns>
		SteelSightWindow& operator=(const SteelSightWindow&) = delete;

		/// <summary>
		/// Check if the window should close return the result
		/// </summary>
		/// <returns>If the window should close</returns>
		_NODISCARD const inline bool ShouldClose() const noexcept { return glfwWindowShouldClose(window); }

		/// <summary>
		/// Function that gets the VkExtent2D of the window
		/// </summary>
		/// <returns>The VkExtent2D based on the width and the height of the window</returns>
		_NODISCARD const inline VkExtent2D getExtent() const noexcept { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

		/// <summary>
		/// Function to check if the window was resized
		/// </summary>
		/// <returns>True if the window was resized</returns>
		_NODISCARD const inline bool wasWindowResized() const noexcept { return framebufferResized; }

		/// <summary>
		/// Function to reset the window resize flag.
		/// </summary>
		inline void resetWindowResizedFlag() noexcept { framebufferResized = false; }

		/// <summary>
		/// Getter of the GLFWwindow pointer.
		/// </summary>
		/// <returns>The GLFWwindow pointer</returns>
		_NODISCARD inline GLFWwindow* getGLFWwindow() const noexcept { return window; }

		/// <summary>
		/// Function that creates a window surface with the current VkInstance
		/// </summary>
		/// <param name="instance">The vulkan instance</param>
		/// <param name="surface">The surface pointer</param>
		void createWindowSurface(const VkInstance& instance, VkSurfaceKHR* surface);

	private:
		/// <summary>
		/// Static callback function for when the frame was resized
		/// </summary>
		/// <param name="window">The GLFWwindow pointer</param>
		/// <param name="width">The new width of the window</param>
		/// <param name="height">The new height of the window</param>
		static inline void framebufferResizeCallback(GLFWwindow* window, const int width, const int height);

		/// <summary>
		/// Function to initialize the window.
		/// </summary>
		void initWindow();

		/// <summary>
		/// The GLFWwindow pointer this is a pointer to the window.
		/// </summary>
		GLFWwindow* window{ nullptr };

		/// <summary>
		/// The width of the window.
		/// </summary>
		int width{ 0 };

		/// <summary>
		/// The height of the window.
		/// </summary>
		int height{ 0 };

		/// <summary>
		/// Boolean value for checking if the framebuffer was resized.
		/// </summary>
		bool framebufferResized{ false };

		/// <summary>
		/// The name of the window.
		/// </summary>
		std::string windowName{};
	};
}