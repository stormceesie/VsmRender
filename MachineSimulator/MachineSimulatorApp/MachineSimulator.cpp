#include "MachineSimulatorCore.hpp"

namespace Voortman {
	class MachineSimulatorApp : MachineSimulatorCore {
		MachineSimulatorApp() : MachineSimulatorCore() {
			title = "MachineSimulatorApp";
		}

		~MachineSimulatorApp() {
		}
	};

	MachineSimulatorCoreMain();
}
