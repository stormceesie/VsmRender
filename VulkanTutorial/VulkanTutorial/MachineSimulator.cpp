#include "MachineSimulatorPCH.hpp"
#include "MachineSimulator.hpp"

namespace Voortman {
	MachineSimulator::MachineSimulator() {}

	MachineSimulator::~MachineSimulator() {}

	void MachineSimulator::run() {
		mainLoop();
	}

	void MachineSimulator::mainLoop() {
		while (!MSWindow.WindowShouldClose()) {
			glfwPollEvents();
		}
	}
}