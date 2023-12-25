#include "point_light_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "..\lve_game_object.hpp"

#include <stdexcept>
#include <array>

namespace lve {

	struct PointLightPushConstants {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	point_light_system::point_light_system(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice{ device } {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	point_light_system::~point_light_system() {
		if (pipelineLayout) {
			vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
		}
	}

	void point_light_system::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void point_light_system::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"shaders\\point_light.vert.spv",
			"shaders\\point_light.frag.spv",
			pipelineConfig);
	}

	void point_light_system::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
		// Constanten
		const float G = 6.67430e-10f; // Gravitatieconstante
		const float timestep = static_cast<float>(frameInfo.frameTime);

		// Verzamel alle puntlichten
		std::vector<LveGameObject*> pointLights;
		for (auto& kv : frameInfo.gameObjects) {
			if (kv.second.pointLight != nullptr) {
				pointLights.push_back(&(kv.second));
			}
		}

		if (pointLights.size() < 2) return; // Er zijn minstens twee lichten nodig voor een zinvolle simulatie

		// Bereken de krachten tussen alle paren van lichten
		for (size_t i = 0; i < pointLights.size(); i++) {
			for (size_t j = i + 1; j < pointLights.size(); j++) {
				LveGameObject* light1 = pointLights[i];
				LveGameObject* light2 = pointLights[j];

				glm::vec3 distanceVec = light2->transform.translation - light1->transform.translation;
				float distance = glm::length(distanceVec);
				float forceMagnitude = (float)(G * (light1->pointLight->mass * light2->pointLight->mass) / ((distance * distance) + 0.0001));
				glm::vec3 forceDirection = glm::normalize(distanceVec);

				// Bereken de versnelling
				glm::vec3 acceleration1 = forceDirection * (forceMagnitude / light1->pointLight->mass);
				glm::vec3 acceleration2 = -forceDirection * (forceMagnitude / light2->pointLight->mass);

				// Update snelheden
				light1->transform.velocity += acceleration1 * timestep;
				light2->transform.velocity += acceleration2 * timestep;
			}
		}

		int lightIndex = 0;
		for (auto& light : pointLights) {
			light->transform.translation += light->transform.velocity * timestep;

			// Zorg ervoor dat je niet meer lichten verwerkt dan MAX_LIGHTS
			if (lightIndex >= MAX_LIGHTS) break;

			// Update UBO
			ubo.pointLights[lightIndex].position = glm::vec4(light->transform.translation, 1.f);
			ubo.pointLights[lightIndex].color = glm::vec4(light->color, light->pointLight->lightIntensity);
			lightIndex++;
		}

		ubo.numLights = static_cast<uint32_t>(lightIndex);
	}

	void point_light_system::render(FrameInfo& frameInfo) {
		lvePipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			PointLightPushConstants push{};
			push.position = glm::vec4(obj.transform.translation, 1.f);
			push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			push.radius = obj.transform.scale.x;

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);

			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
	}
}