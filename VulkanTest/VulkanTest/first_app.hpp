#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include "lve_Window.hpp"
#include "lve_model.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_Game_Object.hpp"
#include "Lve_Renderer.hpp"

namespace lve {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void run();
	private:
		void loadGameObjects();

		LveWindow lveWindow{ WIDTH, HEIGHT, "Voortman 3D renderer" };
		LveDevice lveDevice{ lveWindow };
		LveRenderer lveRenderer{ lveWindow, lveDevice };
		std::vector<LveGameObject> gameObjects;
	};
}