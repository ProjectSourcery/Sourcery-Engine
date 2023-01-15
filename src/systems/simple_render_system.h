#pragma once

#include "src3_camera.h"
#include "src3_pipeline.h"
#include "src3_device.h"
#include "src3_game_object.h"
#include "src3_frame_info.h"

#include <memory>
#include <vector>
#include "src3_swap_chain.h"

namespace src3 {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(SrcDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout );
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		
		SrcDevice& srcDevice;
		std::unique_ptr<SrcSwapChain> srcSwapChain;
		std::unique_ptr<SrcPipeline> srcPipeline;
		VkPipelineLayout pipelineLayout;
	};
}