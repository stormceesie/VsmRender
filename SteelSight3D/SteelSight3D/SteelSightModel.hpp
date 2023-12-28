#pragma once
#include "SteelSightDevice.hpp"
#include "SteelSightBuffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace Voortman {
	class SteelSightModel {
	public:

		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {return position == other.position && color == other.color && normal == other.normal && uv == other.uv;}
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);
		};

		SteelSightModel(SteelSightDevice& device, const SteelSightModel::Builder& builder);
		~SteelSightModel();

		SteelSightModel(const SteelSightModel&) = delete;
		SteelSightModel& operator=(const SteelSightModel&) = delete;

		static std::unique_ptr<SteelSightModel> createModelFromFile(SteelSightDevice& device, const std::string& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& verteces);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		SteelSightDevice& SSDevice;

		std::unique_ptr<SteelSightBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;

		std::unique_ptr<SteelSightBuffer> indexBuffer;
		uint32_t indexCount;
	};
}