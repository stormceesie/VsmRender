#pragma once
#include "SteelSightModel.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <memory>

namespace Voortman {
	struct TransformComponent final {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		// Optional for planet simulation example
		glm::vec3 velocity{};
		float mass{};

		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	struct PointLightComponent {
		float lightIntensity = 1.0f;
		float mass = 100.f;
	};

	class SteelSightSimulationObject final {
	public:
		using id_t = unsigned int;
		// Unordered map is used here this is fine because all the objects should fit inside cache
		using map = ankerl::unordered_dense::map<id_t, SteelSightSimulationObject>;

		_NODISCARD static inline const SteelSightSimulationObject createSimulationObject() noexcept { 
			static id_t currentId = 0;
			return SteelSightSimulationObject{currentId++};
		}

		static SteelSightSimulationObject makePointLight(float intensity = 2.f, glm::vec3 color = glm::vec3(1.f), float radius = 0.05f, float mass = 0.f);

		SteelSightSimulationObject(const SteelSightSimulationObject&) = delete;
		SteelSightSimulationObject& operator=(const SteelSightSimulationObject&) = delete;

		SteelSightSimulationObject(SteelSightSimulationObject&&) = default;
		SteelSightSimulationObject& operator=(SteelSightSimulationObject&&) = default;

		id_t getId() { return id; }

		std::shared_ptr<SteelSightModel> model{};
		glm::vec3 color{};
		TransformComponent transform{};

		std::unique_ptr<PointLightComponent> pointLight{ nullptr };

	private:
		SteelSightSimulationObject(id_t objId) : id{ objId } {}
		id_t id;
	};
}