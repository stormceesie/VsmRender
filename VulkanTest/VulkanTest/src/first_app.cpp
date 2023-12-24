#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "systems/lve_rendersystem.hpp"
#include "systems/point_light_system.hpp"
#include "lve_Camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_buffer.hpp"

#include "first_app.hpp"
#include <stdexcept>
#include <chrono>
#include <array>
#include <numeric>

namespace lve {

    

    FirstApp::FirstApp() {
        globalPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();

        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {

        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        LveRenderSystem RenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        point_light_system PointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };

        LveCamera camera{};

        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = LveGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!lveWindow.ShouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();

            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            // Make sure the aspect ratio of the 3D model always stays the same
            float aspect = lveRenderer.getAspectRatio();
            camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 50.f);

            if (auto commandBuffer = lveRenderer.beginFrame()) {
                int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects
                };

                // update

                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                PointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();


                // render

                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                RenderSystem.renderGameObjects(frameInfo);
                PointLightSystem.render(frameInfo);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::loadGameObjects() {
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "Models/quad.obj");

        auto floor = LveGameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = glm::vec3(0.0f, 0.5f, 0.0f);
        floor.transform.scale = glm::vec3(4.f);
        gameObjects.emplace(floor.getId(), std::move(floor));

        // Aanmaken van het "zon" puntlicht
        auto sunLight = LveGameObject::makePointLight(2.f, { 1.f, 1.f, 0.8f }, 0.2f, 1000000000.0f);
        sunLight.transform.translation = glm::vec3(1.5f, -1.f, 0.f); // Zon staat in het midden
        gameObjects.emplace(sunLight.getId(), std::move(sunLight));

        // Aanmaken van Mercurius
        auto mercuryLight = LveGameObject::makePointLight(1.f, { 0.5f, 0.1f, 0.1f }, 0.03f, 550000.0f);
        mercuryLight.transform.translation = glm::vec3(2.3f, -1.f, 0.4f);
        mercuryLight.transform.velocity = glm::vec3(0.4f, 0.5f, 0.3f);
        gameObjects.emplace(mercuryLight.getId(), std::move(mercuryLight));

        auto mercuryLight2 = LveGameObject::makePointLight(1.f, { 0.1f, 0.1f, 0.5f }, 0.03f, 400000.0f);
        mercuryLight2.transform.translation = glm::vec3(0.7f, -1.f, -0.4f);
        mercuryLight2.transform.velocity = glm::vec3(-0.4f, -0.5f, -0.5f);
        gameObjects.emplace(mercuryLight2.getId(), std::move(mercuryLight2));

        {
            lveModel = LveModel::createModelFromFile(lveDevice, "Models/smooth_vase.obj");

            auto smoothvase = LveGameObject::createGameObject();
            smoothvase.model = lveModel;
            smoothvase.transform.translation = { 0.0f, .5f, 0.0f };
            smoothvase.transform.scale = glm::vec3(3.f);

            gameObjects.emplace(smoothvase.getId(), std::move(smoothvase));
        }
    }
}