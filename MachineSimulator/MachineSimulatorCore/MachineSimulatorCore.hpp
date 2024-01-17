//#####################################################################################################################################################
//#####################################################################################################################################################
// ___       ___                 ___                                         ____                              ___                                 
// `MMb     dMM'                 `MM      68b                               6MMMMb\68b                         `MM                                 
//  MMM.   ,PMM                   MM      Y89                              6M'    `Y89                          MM           /                     
//  M`Mb   d'MM    ___     ____   MM  __  ___ ___  __     ____             MM      ___ ___  __    __  ___   ___ MM    ___   /M      _____  ___  __ 
//  M YM. ,P MM  6MMMMb   6MMMMb. MM 6MMb `MM `MM 6MMb   6MMMMb            YM.     `MM `MM 6MMb  6MMb `MM    MM MM  6MMMMb /MMMMM  6MMMMMb `MM 6MM 
//  M `Mb d' MM 8M'  `Mb 6M'   Mb MMM9 `Mb MM  MMM9 `Mb 6M'  `Mb            YMMMMb  MM  MM69 `MM69 `Mb MM    MM MM 8M'  `Mb MM    6M'   `Mb MM69 " 
//  M  YM.P  MM     ,oMM MM    `' MM'   MM MM  MM'   MM MM    MM                `Mb MM  MM'   MM'   MM MM    MM MM     ,oMM MM    MM     MM MM'    
//  M  `Mb'  MM ,6MM9'MM MM       MM    MM MM  MM    MM MMMMMMMM                 MM MM  MM    MM    MM MM    MM MM ,6MM9'MM MM    MM     MM MM     
//  M   YP   MM MM'   MM MM       MM    MM MM  MM    MM MM                       MM MM  MM    MM    MM MM    MM MM MM'   MM MM    MM     MM MM     
//  M   `'   MM MM.  ,MM YM.   d9 MM    MM MM  MM    MM YM    d9           L    ,M9 MM  MM    MM    MM YM.   MM MM MM.  ,MM YM.  ,YM.   ,M9 MM     
// _M_      _MM_`YMMM9'Yb.YMMMM9 _MM_  _MM_MM__MM_  _MM_ YMMMM9            MYMMMM9 _MM__MM_  _MM_  _MM_ YMMM9MM_MM_`YMMM9'Yb.YMMM9 YMMMMM9 _MM_   
//                                                                         
// 
//                                                    .oooooo.                                
//                                                   d8P'  `Y8b                               
//                                                  888           .ooooo.  oooo d8b  .ooooo.  
//                                                  888          d88' `88b `888""8P d88' `88b 
//                                                  888          888   888  888     888ooo888 
//                                                  `88b    ooo  888   888  888     888    .o 
//                                                   `Y8bood8P'  `Y8bod8P' d888b    `Y8bod8P' 
//
//#####################################################################################################################################################
//#####################################################################################################################################################
// 
// This is the static library of the machine simulator with this core multiple application of Vulkan can easely be created

#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace Voortman {
	class MachineSimulatorCore {
	public:
		static std::vector<const char*> args;

		MachineSimulatorCore();
		virtual ~MachineSimulatorCore();

		bool initVulkan();

		virtual VkResult createInstance(bool enableValidation);

		struct Settings {
			bool validation = false;
			bool fullscreen = false;
			bool vsync = false;
			bool overlay = true;
		} settings;


	protected:


	private:

	};

#if defined(_WIN32)
#define MachineSimulatorCoreMain()
	MachineSimulatorCore* machineSimulator;
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (!machineSimulator) {

		}
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}

	int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
		for (int32_t i = 0; i < __argc; i++) 
			MachineSimulatorCore::args.push_back(__argv[i]);

		machineSimulator = new MachineSimulatorCore();
		machineSimulator->initVulkan();

		return 0;
	}

#endif
}
