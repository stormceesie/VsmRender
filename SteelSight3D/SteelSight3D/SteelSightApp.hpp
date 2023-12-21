#pragma once
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/vec4.hpp>
#include <glm/glm/mat4x4.hpp>

#include <iostream>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

namespace Voortman {
	class SteelSightApp {
	public:
		void run();
		SteelSightApp();
		~SteelSightApp();

	private:
		void mainLoop();
		void initVulkan();
		void cleanup();
	};
}