#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "SteelSightWindow.hpp"

#include "SteelSightApp.hpp"
#include <stdexcept>
#include <chrono>
#include <array>
#include <vector>
#include <numeric>

namespace Voortman {
	SteelSightApp::SteelSightApp() {
		globalPool = SteelSightDescriptorPool::Builder(SSDevice)
			.setMaxSets(SteelSightSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SteelSightSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadSimulationObjects();
	}

	SteelSightApp::~SteelSightApp() {}

	void SteelSightApp::run() {
        std::vector<std::unique_ptr<SteelSightBuffer>> uboBuffers(SteelSightSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<SteelSightBuffer>(
                SSDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = SteelSightDescriptorSetLayout::Builder(SSDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SteelSightSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            SteelSightDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SteelSightRenderSystem RenderSystem{ SSDevice, VSMRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        SteelSightPointLight PointLightSystem{ SSDevice, VSMRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };

        SteelSightCamera Camera{};

        Camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = SteelSightSimulationObject::createSimulationObject();
        SteelSightCameraMovement CameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!SSWindow.ShouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();

            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            CameraController.moveInPlaneXZ(SSWindow.getGLFWwindow(), frameTime, viewerObject);
            Camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            // Make sure the aspect ratio of the 3D model always stays the same
            float aspect = VSMRenderer.getAspectRatio();
            Camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 50.f);

            if (auto commandBuffer = VSMRenderer.beginFrame()) {
                int frameIndex = VSMRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    Camera,
                    globalDescriptorSets[frameIndex],
                    SimulationObjects
                };

                // update

                GlobalUbo ubo{};
                ubo.projection = Camera.getProjection();
                ubo.view = Camera.getView();
                ubo.inverseView = Camera.getInverseView();

                PointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                // render

                VSMRenderer.beginSwapChainRenderPass(commandBuffer);

                // Order matters here because of transperancy
                RenderSystem.renderSimulationObjects(frameInfo);
                PointLightSystem.render(frameInfo);

                VSMRenderer.endSwapChainRenderPass(commandBuffer);
                VSMRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(SSDevice.device());
	}

    void SteelSightApp::loadSimulationObjects() {
        // Aanmaken van het "zon" puntlicht
        auto sunLight = SteelSightSimulationObject::makePointLight(3.f, { 1.f, 1.f, 1.f }, 0.2f, 1000000000.0f);
        sunLight.transform.translation = glm::vec3(1.5f, -3.f, -1.f); // Zon staat in het midden
        SimulationObjects.emplace(sunLight.getId(), std::move(sunLight));

        {
            std::shared_ptr<SteelSightModel> SimulationModel = SteelSightModel::createModelFromFile(SSDevice, "C:/Users/f.kegler/Documents/Stage Files/untitled.obj");

            auto smoothvase = SteelSightSimulationObject::createSimulationObject();
            smoothvase.model = SimulationModel;
            smoothvase.transform.translation = {0.0f, 0.4f, 0.f};
            smoothvase.transform.rotation = {1.57f, 0.0f, 0.0f};
            smoothvase.transform.scale = glm::vec3(0.001f);

            SimulationObjects.emplace(smoothvase.getId(), std::move(smoothvase));
        }
    }
}