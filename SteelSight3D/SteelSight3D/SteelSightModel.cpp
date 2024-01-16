#include "SteelSightModel.hpp"
#include <stddef.h>
#include "SteelSightUtils.hpp"

#include <rapidobj.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cstring>
#include <cassert>

#include <iostream>
#include <chrono>

namespace std {
	// Hash function to use hash vertexes using an unordered map
	template <>
	struct hash<Voortman::SteelSightModel::Vertex> {
		inline size_t operator()(Voortman::SteelSightModel::Vertex const& vertex) const noexcept {
			size_t seed = 0;
			Voortman::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace Voortman {
	SteelSightModel::SteelSightModel(SteelSightDevice& device, const SteelSightModel::Builder& builder) : SSDevice{ device } {
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	SteelSightModel::~SteelSightModel() {}

	std::unique_ptr<SteelSightModel> SteelSightModel::createModelFromFile(SteelSightDevice& device, const std::string& filepath) {
		Builder builder{};
		builder.loadModel(filepath);

		// show the amount of vertices into the console
		// std::cout << "Vertex count: " << builder.vertices.size() << "\n";

		return std::make_unique<SteelSightModel>(device, builder);
	}

	void SteelSightModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			// UINT32 is used because models can have easy have more than 65000 triangles
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void SteelSightModel::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
	}

	void SteelSightModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		SteelSightBuffer stagingBuffer{
			SSDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<SteelSightBuffer>(
			SSDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		SSDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void SteelSightModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		SteelSightBuffer stagingBuffer{
			SSDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<SteelSightBuffer>(
			SSDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		SSDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	std::vector<VkVertexInputBindingDescription> SteelSightModel::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> SteelSightModel::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

		return attributeDescriptions;
	}

	void SteelSightModel::Builder::loadModel(const std::string& filepath) {
		auto start = std::chrono::high_resolution_clock::now();

		// With rapidobj the .obj files will be loaded asynchronous with multi threaded parsing if the file is bigger than 1 MB
		// rapidobj is not the most memory efficient but this should not be a big problem because loading the files takes just a few seconds at max
		// Requires C++17 or above compiler
		rapidobj::Result result = rapidobj::ParseFile(filepath);

		if (result.error) throw std::runtime_error(result.error.code.message());

		bool succes = rapidobj::Triangulate(result);

		if (!succes) throw std::runtime_error(result.error.code.message());

		auto stop = std::chrono::high_resolution_clock::now();
		std::cout << "rapidobj parse time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start) << std::endl << std::endl;

		vertices.clear();
		indices.clear();

		// Significantly faster than std::unordered_map
		ankerl::unordered_dense::map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : result.shapes) {
			for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
				Vertex vertex{};

				const auto& index = shape.mesh.indices[i];
				int material_index = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[i / 3]; // Assuming each face is a triangle

				// Position
				if (index.position_index >= 0) {
					vertex.position = {
						result.attributes.positions[3 * index.position_index + 0],
						result.attributes.positions[3 * index.position_index + 1],
						result.attributes.positions[3 * index.position_index + 2]
					};
				}

				// Assign color based on material
				// Rapidobj does not support vertex colors instead rapidobj uses the more efficient .mtl files to set each vertex color
				if (material_index >= 0) {
					vertex.color = {
						result.materials[material_index].diffuse[0],
						result.materials[material_index].diffuse[1],
						result.materials[material_index].diffuse[2]
					};
				}

				// Normals
				if (index.normal_index >= 0) {
					vertex.normal = {
						result.attributes.normals[3 * index.normal_index + 0],
						result.attributes.normals[3 * index.normal_index + 1],
						result.attributes.normals[3 * index.normal_index + 2]
					};
				}

				// Texture Coordinates
				if (index.texcoord_index >= 0) {
					vertex.uv = {
						result.attributes.texcoords[2 * index.texcoord_index + 0],
						result.attributes.texcoords[2 * index.texcoord_index + 1]
					};
				}

				// Check for unique vertex and update indices
				if (!uniqueVertices.contains(vertex)) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}

		// Information about the 3D models
		stop = std::chrono::high_resolution_clock::now();
		std::cout << filepath << std::endl;
		std::cout << "Vertices: " << vertices.size() << std::endl;
		std::cout << "Indices: " << indices.size() << std::endl;
		std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start) << std::endl << std::endl;
	}
}