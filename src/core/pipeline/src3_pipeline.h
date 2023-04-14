#pragma once

#include "core/device/src3_device.h"

#include <string>
#include <vector>

namespace src3 {

	struct PipelineConfigInfo {
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo() = default;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptors{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
        VkRenderPass viewportRenderPass = nullptr;
		uint32_t subpass = 0;
	};

	class SrcPipeline {
	public:
		SrcPipeline(SrcDevice &device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
		~SrcPipeline();

		SrcPipeline(const SrcPipeline&) = delete;
		SrcPipeline operator=(const SrcPipeline) = delete;

		void bindGraphics(VkCommandBuffer commandBuffer);
        void bindViewport(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);

		VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }

	private:
		static std::vector<char> readFile(const std::string& filepath);
		void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		SrcDevice& srcDevice;
		VkPipeline graphicsPipeline;
        VkPipeline viewportPipeline;
	};
}