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

#include "SteelSightWindow.hpp"
#include "SteelSightModel.hpp"
#include "SteelSightRenderer.hpp"
#include "SteelSightDevice.hpp"
#include "SteelSightPipeline.hpp"
#include "SteelSightSwapChain.hpp"
#include "SteelSightSimulationObject.hpp"
#include "SteelSightDescriptor.hpp"
#include "SteelSightFrameInfo.hpp"
#include "SteelSightRenderSystem.hpp"
#include "SteelSightCameraMovement.hpp"
#include "SteelSightPointLight.hpp"

namespace Voortman {
	class SteelSightApp final {
	public:
		static constexpr uint32_t WIDTH{ 800 };
		static constexpr uint32_t HEIGHT{ 600 };

		SteelSightApp();
		~SteelSightApp();

		SteelSightApp(const SteelSightApp&) = delete;
		SteelSightApp& operator=(const SteelSightApp&) = delete;

		void run();
	private:
		void loadSimulationObjects();

		SteelSightWindow SSWindow{ WIDTH, HEIGHT, "Voortman SteelSight3D" };
		SteelSightDevice SSDevice{ SSWindow };
		SteelSightRenderer VSMRenderer{ SSWindow, SSDevice };

		// Order matters !
		std::unique_ptr<SteelSightDescriptorPool> globalPool{};
		SteelSightSimulationObject::map SimulationObjects;
	};
}