#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include "..\lve_model.hpp"
#include "..\lve_pipeline.hpp"
#include "..\lve_device.hpp"
#include "..\lve_Game_Object.hpp"
#include "..\lve_swap_chain.hpp"
#include "..\Lve_Camera.hpp"
#include "..\lve_frame_info.hpp"

namespace lve {
	class point_light_system {
	public:
		point_light_system(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~point_light_system();

		point_light_system(const point_light_system&) = delete;
		point_light_system& operator=(const point_light_system&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);


		LveDevice& lveDevice;
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}