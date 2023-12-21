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
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "Models/flat_vase.obj");

        {
            lveModel = LveModel::createModelFromFile(lveDevice, "Models/smooth_vase.obj");

            auto smoothvase = LveGameObject::createGameObject();
            smoothvase.model = lveModel;
            smoothvase.transform.translation = {0.0f, .5f, 0.0f };
            smoothvase.transform.scale = glm::vec3(3.f);

            gameObjects.emplace(smoothvase.getId(), std::move(smoothvase));
        }

        {
            lveModel = LveModel::createModelFromFile(lveDevice, "Models/quad.obj");

            auto floor = LveGameObject::createGameObject();
            floor.model = lveModel;
            floor.transform.translation = { .0f, .5f, 0.0f };
            floor.transform.scale = glm::vec3(20.f, 1.f, 20.f);

            gameObjects.emplace(floor.getId(), std::move(floor));
        }

        {
            std::vector<glm::vec3> lightColors{ {1.f, .1f, .1f},
                                                {.1f, .1f, 1.f},
                                                {.1f, 1.f, .1f},
                                                {1.f, 1.f, .1f},
                                                {.1f, 1.f, 1.f},
                                                {1.f, 1.f, 1.f}
            };

            for (int i = 0; i < lightColors.size(); i++) {
                auto pointLight = LveGameObject::makePointLight(0.5f);
                pointLight.color = lightColors[i];
                auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f,-1.f,0.f });
                pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
                gameObjects.emplace(pointLight.getId(), std::move(pointLight));
            }
        }
    }
}