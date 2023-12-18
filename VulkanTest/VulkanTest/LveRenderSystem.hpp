#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include "lve_model.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_Game_Object.hpp"
#include "lve_swap_chain.hpp"


namespace lve {
	class LveRenderSystem {
	public:
		LveRenderSystem(LveDevice &device, VkRenderPass renderPass);
		~LveRenderSystem();

		LveRenderSystem(const LveRenderSystem&) = delete;
		LveRenderSystem& operator=(const LveRenderSystem&) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);


		LveDevice& lveDevice;
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}