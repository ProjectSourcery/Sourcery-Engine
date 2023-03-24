#include "first_app.h"

#include "game/keyboard_controller/keyboard_movement_controller.h"
#include "game/camera/src3_camera.h"
#include "core/buffer/src3_buffer.h"
#include "core/buffer/uniform/src3_ubo.h"
#include "render/systems/simple_render_system.h"
#include "render/systems/point_light_system.h"
#include "render/imgui/src3_imgui.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

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
		for (long long unsigned int i = 0; i < framePools.size(); i++) {
			framePools[i] = framePoolBuilder.build();
		}

		JPH::RegisterDefaultAllocator(); //Jolt Physics -- cant think of anything else
		physicsSystem = std::make_unique<SrcPhysicsSystem>(ecs);
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		SrcUbo<GlobalUbo> globalUbo{srcDevice,1,false,false};
		auto globalSetLayout = SrcDescriptorSetLayout::Builder(srcDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(SrcSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (long long unsigned int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = globalUbo.bufferInfoForRegion(i);
			SrcDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		// render systems
		SrcImGui imGui{srcDevice,srcWindow,srcRenderer,ecs};
		SimpleRenderSystem simpleRenderSystem{srcDevice,ecs, srcRenderer.getSwapChainRenderPass(),globalSetLayout->getDescriptorSetLayout()};
		PointLightSystem pointLightSystem{srcDevice, srcRenderer.getSwapChainRenderPass(),globalSetLayout->getDescriptorSetLayout()};

		// game systems
		
		///////////////

        SrcCamera camera{};
        camera.setViewTarget(glm::vec3{ -1.f,-2.f,-2.f }, glm::vec3{ 0.f,0.f,2.5f });

        TransformComponent viewerTransform{};
  		viewerTransform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!srcWindow.shouldClose()) {
			glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(srcWindow.getGLFWWindow(), frameTime, viewerTransform);
            camera.setViewYXZ(viewerTransform.translation, viewerTransform.rotation);

            float aspect = srcRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(70.f), aspect,0.1f,100.0f);

			physicsSystem->update();

			if (auto commandBuffer = srcRenderer.beginFrame()) {
				int frameIndex = srcRenderer.getFrameIndex();
				framePools[frameIndex]->resetPool();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,camera,
					globalDescriptorSets[frameIndex],
					*framePools[frameIndex],
					ecs
				};

				// update
				GlobalUbo &ubo = globalUbo.get(frameIndex);
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getInverseView();
				pointLightSystem.update(frameInfo,ubo);
				globalUbo.flushRegion(frameIndex);

				// render
				imGui.newFrame();

				srcRenderer.beginSwapChainRenderPass(commandBuffer);

				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				imGui.run();

				imGui.render(commandBuffer);
				
				srcRenderer.endSwapChainRenderPass(commandBuffer);
				srcRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(srcDevice.device());
	}

	void FirstApp::loadGameObjects()
	{
        std::shared_ptr<SrcModel> srcModel = SrcModel::createModelFromFile(srcDevice, "models/flat_vase.obj");

        auto fVase = ecs.create();
		ecs.emplace<TransformComponent>(fVase,glm::vec3( -.5f, -1.f, 0.f ),glm::vec3(3.f,3.f,3.f));
		ecs.emplace<ModelComponent>(fVase,srcModel);
		physicsSystem->registerPhysicsBody(fVase, new SphereShape(1.f), {}, EActivation::Activate);
		
		srcModel = SrcModel::createModelFromFile(srcDevice, "models/smooth_vase.obj");
		auto smoothVase = ecs.create();
		ecs.emplace<TransformComponent>(smoothVase,glm::vec3(.5f, .5f, 0.f),glm::vec3(3.f, 1.5f, 3.f));
		ecs.emplace<ModelComponent>(smoothVase,srcModel);

		srcModel = SrcModel::createModelFromFile(srcDevice, "models/quad.obj");
		auto floor = ecs.create();
		ecs.emplace<TransformComponent>(floor,glm::vec3(0.f, .5f, 0.f),glm::vec3(3.f, 1.f, 3.f));
		ecs.emplace<ModelComponent>(floor,srcModel);
		{
			PhysicsComponent phys{};
			phys.motionType = EMotionType::Static;
			phys.objectLayer = Layers::NON_MOVING;

			physicsSystem->registerPhysicsBody(floor,new BoxShape(Vec3(3.f,1.f,3.f)),phys);
		}

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}  
		};

		for (long long unsigned int i = 0; i < lightColors.size(); i++) {
			auto pointLight = makePointLight(ecs, 0.2f);
			ecs.replace<ColorComponent>(pointLight,lightColors[i]);

			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{0.f, -1.f, 0.f});
			ecs.patch<TransformComponent>(pointLight,[&](TransformComponent &transform){ transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));});
		}
	}

}