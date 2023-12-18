#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include "lve_model.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_Game_Object.hpp"
#include "lve_swap_chain.hpp"
#include "Lve_Camera.hpp"


namespace lve {
	class LveRenderSystem {
	public:
		LveRenderSystem(LveDevice &device, VkRenderPass renderPass);
		~LveRenderSystem();

		LveRenderSystem(const LveRenderSystem&) = delete;
		LveRenderSystem& operator=(const LveRenderSystem&) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects, const LveCamera &camera);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);


		LveDevice& lveDevice;
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}