#include "SteelSightPointLight.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "SteelSightSimulationObject.hpp"

#include <stdexcept>
#include <array>
#include <map>
#include <iterator>

namespace Voortman {
	struct PointLightPushConstants {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	SteelSightPointLight::SteelSightPointLight(SteelSightDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : SSDevice{ device } {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	SteelSightPointLight::~SteelSightPointLight() {
		if (pipelineLayout) {
			vkDestroyPipelineLayout(SSDevice.device(), pipelineLayout, nullptr);
		}
	}

	void SteelSightPointLight::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

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

		if (vkCreatePipelineLayout(SSDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SteelSightPointLight::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		SteelSightPipeline::defaultPipelineConfigInfo(pipelineConfig);
		SteelSightPipeline::enableAlphaBlending(pipelineConfig);
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		SSPipeline = std::make_unique<SteelSightPipeline>(
			SSDevice,
			"shaders\\point_light.vert.spv",
			"shaders\\point_light.frag.spv",
			pipelineConfig);
	}

	void SteelSightPointLight::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
		// Constanten
		const float G = 6.67430e-10f; // Gravitatieconstante
		const float timestep = static_cast<float>(frameInfo.frameTime);

		// Verzamel alle puntlichten
		std::vector<SteelSightSimulationObject*> pointLights;
		for (auto& kv : frameInfo.simulationObjects) {
			if (kv.second.pointLight != nullptr) {
				pointLights.push_back(&(kv.second));
			}
		}

		if (pointLights.size() < 2) return; // Er zijn minstens twee lichten nodig voor een zinvolle simulatie

		// Bereken de krachten tussen alle paren van lichten
		for (size_t i = 0; i < pointLights.size(); i++) {
			for (size_t j = i + 1; j < pointLights.size(); j++) {
				SteelSightSimulationObject* light1 = pointLights[i];
				SteelSightSimulationObject* light2 = pointLights[j];

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

	void SteelSightPointLight::render(FrameInfo& frameInfo) {
		std::map<float, SteelSightSimulationObject::id_t> sorted;
		for (auto& kv : frameInfo.simulationObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
			float disSquared = glm::dot(offset, offset);
			sorted[disSquared] = obj.getId();
		}

		SSPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);

		// iterate through sorted lights in reverse order (from back to front)
		for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
			auto& obj = frameInfo.simulationObjects.at(it->second);

			PointLightPushConstants push{};
			push.position = glm::vec4(obj.transform.translation, 1.f);
			push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			push.radius = obj.transform.scale.x;

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);

			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
	}
}