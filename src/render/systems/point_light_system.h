#pragma once

#include "core/swapchain/src3_swap_chain.h"
#include "core/device/src3_device.h"
#include "core/pipeline/src3_pipeline.h"
#include "game/camera/src3_camera.h"
#include "game/gameobject/src3_game_object.h"
#include "util/src3_frame_info.h"

#include <memory>
#include <vector>

namespace src3 {
	class PointLightSystem {
	public:
		PointLightSystem(SrcDevice& device, VkRenderPass renderPass,VkRenderPass viewportRenderPass, VkDescriptorSetLayout globalSetLayout );
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo,GlobalUbo &ubo);
		void render(FrameInfo& frameInfo);
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass,VkRenderPass viewportRenderPass);
		
		SrcDevice& srcDevice;
		std::unique_ptr<SrcSwapChain> srcSwapChain;
		std::unique_ptr<SrcPipeline> srcPipeline;
		VkPipelineLayout pipelineLayout;
	};
}