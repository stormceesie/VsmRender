#include "MachineSimulatorPCH.hpp"
#include "MachineSimulatorWindow.hpp"

namespace Voortman {
	MachineSimulatorWindow::MachineSimulatorWindow(const uint32_t width, const uint32_t height, const std::string& windowname) : windowName{ windowname } {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, windowname.c_str(), nullptr, nullptr);
		if (!window) throw std::runtime_error("Could not create a window!");
		std::cout << "Created window!" << std::endl;

		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	MachineSimulatorWindow::~MachineSimulatorWindow() {
		if (window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}

	void MachineSimulatorWindow::createWindowSurface(const VkInstance& instance, VkSurfaceKHR* surface) const {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	void MachineSimulatorWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto SSWindow = reinterpret_cast<MachineSimulatorWindow*>(glfwGetWindowUserPointer(window));
		SSWindow->framebufferResized = true;
		SSWindow->width = width;
		SSWindow->height = height;
	}
}