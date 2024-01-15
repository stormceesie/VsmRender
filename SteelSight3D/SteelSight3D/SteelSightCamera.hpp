#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Voortman {
	class SteelSightCamera final {
	public:
		void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
		void setViewTarget(glm::vec3 position, glm::vec3 Target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		inline const glm::mat4& getProjection() const { return projectionMatrix; }
		inline const glm::mat4& getView() const { return viewMatrix; }
		inline const glm::mat4& getInverseView() const { return inverseViewMatrix; }
		inline const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); }

	private:
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 inverseViewMatrix{ 1.f };
	};
}