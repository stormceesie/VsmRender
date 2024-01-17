#include "MachineSimulatorCore.hpp"

namespace Voortman {
	std::vector<const char*> MachineSimulatorCore::args;

	bool MachineSimulatorCore::initVulkan() {
		VkResult err;

		err = createInstance(settings.validation);
	}

	VkResult MachineSimulatorCore::createInstance(bool enableValidation) {

	}
}