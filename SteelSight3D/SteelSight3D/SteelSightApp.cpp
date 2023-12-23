#include "SteelSightApp.hpp"
#include "SteelSightWindow.hpp"
#include <vector>

namespace Voortman {
	SteelSightApp::SteelSightApp() {
	}

	SteelSightApp::~SteelSightApp() {
	}

	void SteelSightApp::mainLoop() {
		while (!SSWindow.ShouldClose()) {
			glfwPollEvents();
		}
	}

	void SteelSightApp::run() {
		mainLoop();
	}
}