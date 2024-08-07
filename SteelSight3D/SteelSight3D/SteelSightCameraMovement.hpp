#pragma once
#include "SteelSightSimulationObject.hpp"
#include "SteelSightWindow.hpp"

namespace Voortman {
	class SteelSightCameraMovement final {
	public:
		struct KeyMappings final {
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_E;
			int moveDown = GLFW_KEY_Q;
			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
		} keys{};

		void moveInPlaneXZ(GLFWwindow* window, float dt, SteelSightSimulationObject& gameObject);
		const float moveSpeed{ 3.f };
		const float lookSpeed{ 1.5f };
	};
}