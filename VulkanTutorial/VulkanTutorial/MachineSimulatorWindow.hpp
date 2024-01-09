#pragma once
#include "MachineSimulatorPCH.hpp"
#include "MachineSimulatorDefines.hpp"

namespace Voortman {
	static constexpr uint32_t WIDTH{ 800 };
	static constexpr uint32_t HEIGHT{ 600 };

	class MachineSimulatorWindow {
	public:
		MachineSimulatorWindow(const uint32_t width = WIDTH, const uint32_t height = HEIGHT, const std::string& name = "Vulkan tutorial");
		~MachineSimulatorWindow();

		MachineSimulatorWindow(const MachineSimulatorWindow&) = delete;
		MachineSimulatorWindow& operator=(const MachineSimulatorWindow&) = delete;

		inline GLFWwindow* GetWindowPtr() const { return window == nullptr? nullptr : window; }
		inline bool WindowShouldClose() const { return glfwWindowShouldClose(window); }

		void createWindowSurface(const VkInstance& instance, VkSurfaceKHR* surface) const;

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		GLFWwindow* window{ nullptr };

		int width{ 0 };
		int height{ 0 };
		bool framebufferResized = false;

		std::string windowName{};
	};
}