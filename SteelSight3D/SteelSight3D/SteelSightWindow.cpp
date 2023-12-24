#include "SteelSightWindow.hpp"
#include <stdexcept>
#include "imgui.h"
#include <stdio.h>

namespace Voortman {
	SteelSightWindow::SteelSightWindow(int w, int h, const std::string& name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	SteelSightWindow::~SteelSightWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void SteelSightWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		if (!window) throw std::runtime_error("failed to create a window!");

		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void SteelSightWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	void SteelSightWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto SSWindow = reinterpret_cast<SteelSightWindow*>(glfwGetWindowUserPointer(window));
		SSWindow->framebufferResized = true;
		SSWindow->width = width;
		SSWindow->height = height;
	}
}