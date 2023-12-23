#pragma once
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

#include "SteelSightDevice.hpp"

namespace Voortman {
	constexpr uint32_t WIDTH{ 800 };
	constexpr uint32_t HEIGHT{ 600 };

#ifdef _DEBUG
	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};
#endif

	class SteelSightApp {
	public:
		void run();

		SteelSightApp();
		~SteelSightApp();

		SteelSightApp(const SteelSightApp&) = delete;
		SteelSightApp& operator=(const SteelSightApp&) = delete;

	private:
		void mainLoop();

		SteelSightWindow SSWindow{ WIDTH, HEIGHT, "Voortman SteelSight3D" };
		SteelSightDevice SSDevice{ SSWindow };
	};
}