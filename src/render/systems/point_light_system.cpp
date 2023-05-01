#include "point_light_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>
#include <map>
#include <iostream>

namespace src3 {

	struct PointLightPushConstants {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	PointLightSystem::PointLightSystem(SrcDevice& device, VkRenderPass renderPass,VkRenderPass viewportRenderPass, VkDescriptorSetLayout globalSetLayout) : srcDevice{device} {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass,viewportRenderPass);
	}

	PointLightSystem::~PointLightSystem() {
		vkDestroyPipelineLayout(srcDevice.device(), pipelineLayout, nullptr);
	}

	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(srcDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void PointLightSystem::createPipeline(VkRenderPass renderPass,VkRenderPass viewportRenderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		SrcPipeline::defaultPipelineConfigInfo(pipelineConfig);
		SrcPipeline::enableAlphaBlending(pipelineConfig);
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptors.clear();
		pipelineConfig.renderPass = renderPass;
        pipelineConfig.viewportRenderPass = viewportRenderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		srcPipeline = std::make_unique<SrcPipeline>(
			srcDevice,
			"shaders/point_light.vert.spv",
			"shaders/point_light.frag.spv",
			pipelineConfig
			);
	}

	void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
		auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, {0.f, -1.f, 0.f});
		int lightIndex = 0;
		for (auto [entity,pointLight,lightTransform]: frameInfo.ecs.view<PointLightComponent,TransformComponent>().each()) {
			assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

			// update light position
			frameInfo.ecs.emplace_or_replace<TransformComponent>(entity,glm::vec3(rotateLight * glm::vec4(lightTransform.translation, 1.f)),lightTransform.scale);

			// copy light to ubo
			ubo.pointLights[lightIndex].position = glm::vec4(lightTransform.translation, 1.f);
			ubo.pointLights[lightIndex].color = glm::vec4(frameInfo.ecs.get<ColorComponent>(entity).color, frameInfo.ecs.get<PointLightComponent>(entity).lightIntensity);

			lightIndex += 1;
		}
		ubo.numLights = lightIndex;
	}

	void PointLightSystem::render(FrameInfo& frameInfo)
	{
		// sort lights
		std::map<float,entt::entity> sorted;
		for (auto [lightId,plc,lightTransform]: frameInfo.ecs.view<PointLightComponent,TransformComponent>().each()) {
			auto offset = frameInfo.camera.getPosition() - lightTransform.translation;
			float disSquared = glm::dot(offset,offset);
			sorted[disSquared] = lightId;
		}

        srcPipeline->bindViewport(frameInfo.viewportCommandBuffer);

		vkCmdBindDescriptorSets(frameInfo.viewportCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

		for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
			auto lightId = it->second;
			auto lightTransform = frameInfo.ecs.get<TransformComponent>(lightId);
    		auto lightColor = frameInfo.ecs.get<ColorComponent>(lightId);
    		auto lightComp = frameInfo.ecs.get<PointLightComponent>(lightId);

			PointLightPushConstants push{};
			push.position = glm::vec4(lightTransform.translation, 1.f);
			push.color = glm::vec4(lightColor.color, lightComp.lightIntensity);
			push.radius = lightTransform.scale.x;

			vkCmdPushConstants(
				frameInfo.viewportCommandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push);
			vkCmdDraw(frameInfo.viewportCommandBuffer, 6, 1, 0, 0);
		}
	}
}