#include "SteelSightWindow.hpp"
#include <stdexcept>
#include <stdio.h>
#include "Windows.h"

namespace Voortman {
	SteelSightWindow::SteelSightWindow(const int w, const int h, const std::string& name) 
		: width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	SteelSightWindow::~SteelSightWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void SteelSightWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

		if (!window) _UNLIKELY throw std::runtime_error("failed to create a window!");
		
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void SteelSightWindow::createWindowSurface(const VkInstance& instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) _UNLIKELY {
			throw std::runtime_error("failed to create window surface");
		}
	}

	void SteelSightWindow::framebufferResizeCallback(GLFWwindow* window, const int width, const int height) {
		auto SSWindow = reinterpret_cast<SteelSightWindow*>(glfwGetWindowUserPointer(window));
		SSWindow->framebufferResized = true;
		SSWindow->width = width;
		SSWindow->height = height;
	}
}