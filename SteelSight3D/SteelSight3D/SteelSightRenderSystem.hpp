#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include "SteelSightModel.hpp"
#include "SteelSightPipeline.hpp"
#include "SteelSightDevice.hpp"
#include "SteelSightSimulationObject.hpp"
#include "SteelSightSwapChain.hpp"
#include "SteelSightCamera.hpp"
#include "SteelSightFrameInfo.hpp"

namespace Voortman {
	class SteelSightRenderSystem {
	public:
		SteelSightRenderSystem(SteelSightDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SteelSightRenderSystem();

		SteelSightRenderSystem(const SteelSightRenderSystem&) = delete;
		SteelSightRenderSystem& operator=(const SteelSightRenderSystem&) = delete;

		void renderSimulationObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		SteelSightDevice& SSDevice;
		std::unique_ptr<SteelSightPipeline> SSPipeline;
		VkPipelineLayout pipelineLayout;
	};
}