#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>
#include <iostream>

namespace src3 {

	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	SimpleRenderSystem::SimpleRenderSystem(SrcDevice& device, entt::registry &ecs,VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : srcDevice{device}, ents{ecs} {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem() {
		vkDestroyPipelineLayout(srcDevice.device(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstRange{};
		pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstRange.offset = 0;
		pushConstRange.size = sizeof(SimplePushConstantData);

		renderSystemLayout = SrcDescriptorSetLayout::Builder(srcDevice)
			.addBinding(
				0,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout, renderSystemLayout->getDescriptorSetLayout() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstRange;

		if (vkCreatePipelineLayout(srcDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		SrcPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		srcPipeline = std::make_unique<SrcPipeline>(
			srcDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
			);
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo)
	{
		srcPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

		int index = 0;
		for (auto [entity, transform] : ents.group<TransformComponent>({},entt::exclude<PointLightComponent>).each()) {
			TransformUboData& transformData = transformUbo.get(frameInfo.frameIndex,index);
			transformData.modelMatrix = transform.mat4();
			transformData.normalMatrix = transform.normalMatrix();

			index += 1;
		}
		transformUbo.flushRegion(frameInfo.frameIndex);

		index = 0;
		for (auto [entity, model] : ents.group<ModelComponent>({},entt::exclude<PointLightComponent>).each()){
			TextureUboData& textureData = textureUbo.get(frameInfo.frameIndex,index);
			if (model.texture) 
			{
				textureData.diffuseMap = model.texture;
			} else {
				#pragma GCC diagnostic push
				#pragma GCC diagnostic ignored "-Wpessimizing-move"
				textureData.diffuseMap = std::move(std::make_unique<SrcTexture>(srcDevice,"../textures/missing.png"));
				#pragma GCC diagnostic pop
			}
			
			index += 1;
		}
		textureUbo.flushRegion(frameInfo.frameIndex);

		index = 0;
		for (auto [entity, transform, model] : ents.group<TransformComponent, ModelComponent>({},entt::exclude<PointLightComponent>).each()) {
			auto bufferInfo = transformUbo.bufferInfoForElement(frameInfo.frameIndex,index);
			auto imageInfo = textureUbo.get(frameInfo.frameIndex).diffuseMap->getImageInfo();
			VkDescriptorSet transformDescriptorSet;
			SrcDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1,&imageInfo) // TODO: Fix this
				.build(transformDescriptorSet);
			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
				1,  // binding 1 more set
				&transformDescriptorSet,
				0,
				nullptr);

			SimplePushConstantData push{};
			push.modelMatrix = transform.mat4();
			push.normalMatrix = transform.normalMatrix();

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
			model.model->bind(frameInfo.commandBuffer);
			model.model->draw(frameInfo.commandBuffer);

			index += 1;
		}
	}
}