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
	class SteelSightPointLight final {
	public:
		SteelSightPointLight(SteelSightDevice& device, VkRenderPass renderpass, VkDescriptorSetLayout globalSetLayout);
		~SteelSightPointLight();

		SteelSightPointLight(const SteelSightPointLight&) = delete;
		SteelSightPointLight& operator=(const SteelSightPointLight&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderpass);

		SteelSightDevice& SSDevice;
		std::unique_ptr<SteelSightPipeline> SSPipeline;
		VkPipelineLayout pipelineLayout;
	};
}