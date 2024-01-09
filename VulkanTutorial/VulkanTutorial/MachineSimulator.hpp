#pragma once
#include "MachineSimulatorPCH.hpp"
#include "MachineSimulatorDefines.hpp"
#include "MachineSimulatorWindow.hpp"
#include "MachineSimulatorDevice.hpp"

namespace Voortman {
	class MachineSimulator{
	public:
		MachineSimulator();
		~MachineSimulator();
		void run();

	private:
		void mainLoop();

		// Order matter because they are dependent to eachother
		MachineSimulatorWindow MSWindow{};
		MachineSimulatorDevice MSDevice{MSWindow};
	};
}
