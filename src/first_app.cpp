#include "first_app.h"

#include "keyboard_movement_controller.h"
#include "src3_camera.h"
#include "buffer/src3_buffer.h"
#include "systems/simple_render_system.h"
#include "systems/point_light_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <chrono>
#include <cassert>
#include <array>
#include <numeric>

namespace src3 {

	

	FirstApp::FirstApp() {
		globalPool = SrcDescriptorPool::Builder(srcDevice)
			.setMaxSets(SrcSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SrcSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		framePools.resize(SrcSwapChain::MAX_FRAMES_IN_FLIGHT);
		auto framePoolBuilder = SrcDescriptorPool::Builder(srcDevice)
			.setMaxSets(1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		for (int i = 0; i < framePools.size(); i++) {
			framePools[i] = framePoolBuilder.build();
		}

		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		
		std::vector<std::unique_ptr<SrcBuffer>> uboBuffers(SrcSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<SrcBuffer>(
				srcDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = SrcDescriptorSetLayout::Builder(srcDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(SrcSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			SrcDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{srcDevice, srcRenderer.getSwapChainRenderPass(),globalSetLayout->getDescriptorSetLayout()};
		PointLightSystem pointLightSystem{srcDevice, srcRenderer.getSwapChainRenderPass(),globalSetLayout->getDescriptorSetLayout()};
        SrcCamera camera{};
        camera.setViewTarget(glm::vec3{ -1.f,-2.f,-2.f }, glm::vec3{ 0.f,0.f,2.5f });

        auto& viewerObject = gameObjectManager.createGameObject();
		viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!srcWindow.shouldClose()) {
			glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(srcWindow.getGLFWWindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = srcRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect,0.1f,100.0f);

			if (auto commandBuffer = srcRenderer.beginFrame()) {
				int frameIndex = srcRenderer.getFrameIndex();
				framePools[frameIndex]->resetPool();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,camera,
					globalDescriptorSets[frameIndex],
					*framePools[frameIndex],
					gameObjectManager.gameObjects
				};

				// update
				GlobalUbo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getInverseView();
				pointLightSystem.update(frameInfo,ubo);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				gameObjectManager.updateBuffer(frameIndex);

				// render
				srcRenderer.beginSwapChainRenderPass(commandBuffer);

				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);
				
				srcRenderer.endSwapChainRenderPass(commandBuffer);
				srcRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(srcDevice.device());
	}

	void FirstApp::loadGameObjects()
	{
        std::shared_ptr<SrcModel> srcModel = SrcModel::createModelFromFile(srcDevice, "models/flat_vase.obj");

        auto& fVase = gameObjectManager.createGameObject();
		fVase.model = srcModel;
		fVase.transform.translation = { -.5f, .5f, 0.f };
		fVase.transform.scale = {3.f,3.f,3.f};

		srcModel = SrcModel::createModelFromFile(srcDevice, "models/smooth_vase.obj");
		auto& sVase = gameObjectManager.createGameObject();
		sVase.model = srcModel;
		sVase.transform.translation = { .5f, .5f, 0.f };
		sVase.transform.scale = { 3.f,1.5f,3.f };

		srcModel = SrcModel::createModelFromFile(srcDevice, "models/quad.obj");
		std::shared_ptr<SrcTexture> marbleTexture = SrcTexture::createTextureFromFile(srcDevice, "../textures/missing.png");
		auto& floor = gameObjectManager.createGameObject();
		floor.model = srcModel;
		floor.diffuseMap = marbleTexture;
		floor.transform.translation = { 0.f, .5f, 0.f };
		floor.transform.scale = { 3.f,1.f,3.f };

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}  
		};

		for (int i = 0; i < lightColors.size(); i++) {
			auto& pointLight = gameObjectManager.makePointLight(0.2f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{0.f, -1.f, 0.f});
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
		}
	}

}