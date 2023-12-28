#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "SteelSightApp.hpp"
#include "SteelSightWindow.hpp"
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

        SteelSightCamera camera{};

        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = SteelSightSimulationObject::createSimulationObject();
        SteelSightCameraMovement cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!SSWindow.ShouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();

            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(SSWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            // Make sure the aspect ratio of the 3D model always stays the same
            float aspect = VSMRenderer.getAspectRatio();
            camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 50.f);

            if (auto commandBuffer = VSMRenderer.beginFrame()) {
                int frameIndex = VSMRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    SimulationObjects
                };

                // update

                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();

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
        std::shared_ptr<SteelSightModel> SimulationModel = SteelSightModel::createModelFromFile(SSDevice, "Models/quad.obj");

        auto floor = SteelSightSimulationObject::createSimulationObject();
        floor.model = SimulationModel;
        floor.transform.translation = glm::vec3(0.0f, 0.5f, 0.0f);
        floor.transform.scale = glm::vec3(4.f);
        SimulationObjects.emplace(floor.getId(), std::move(floor));

        // Aanmaken van het "zon" puntlicht
        auto sunLight = SteelSightSimulationObject::makePointLight(0.f, { 0.f, 0.f, 0.f }, 0.2f, 1000000000.0f);
        sunLight.transform.translation = glm::vec3(1.5f, -1.f, 0.f); // Zon staat in het midden
        SimulationObjects.emplace(sunLight.getId(), std::move(sunLight));

        // Aanmaken van Mercurius
        auto mercuryLight = SteelSightSimulationObject::makePointLight(1.f, { 0.5f, 0.1f, 0.1f }, 0.03f, 550000.0f);
        mercuryLight.transform.translation = glm::vec3(2.3f, -1.f, 0.4f);
        mercuryLight.transform.velocity = glm::vec3(0.4f, 0.5f, 0.3f);
        SimulationObjects.emplace(mercuryLight.getId(), std::move(mercuryLight));

        auto mercuryLight2 = SteelSightSimulationObject::makePointLight(1.f, { 0.1f, 0.1f, 0.5f }, 0.03f, 400000.0f);
        mercuryLight2.transform.translation = glm::vec3(0.7f, -1.f, -0.4f);
        mercuryLight2.transform.velocity = glm::vec3(-0.4f, -0.5f, -0.5f);
        SimulationObjects.emplace(mercuryLight2.getId(), std::move(mercuryLight2));

        auto mercuryLight3 = SteelSightSimulationObject::makePointLight(0.5f, { 0.1f, 0.5f, 0.1f }, 0.03f, 400000.0f);
        mercuryLight3.transform.translation = glm::vec3(0.8f, -1.f, 0.6f);
        mercuryLight3.transform.velocity = glm::vec3(-0.4f, 0.6f, 0.2f);
        SimulationObjects.emplace(mercuryLight3.getId(), std::move(mercuryLight3));

        {
            SimulationModel = SteelSightModel::createModelFromFile(SSDevice, "Models/Voortman3D.obj");

            auto smoothvase = SteelSightSimulationObject::createSimulationObject();
            smoothvase.model = SimulationModel;
            smoothvase.transform.translation = { 1.0f, -0.85f, -0.5f };
            smoothvase.transform.rotation = {3.14f + 1.57f, 3.14f, 0.f};
            smoothvase.transform.scale = glm::vec3(0.001f);

            SimulationObjects.emplace(smoothvase.getId(), std::move(smoothvase));
        }
    }
}