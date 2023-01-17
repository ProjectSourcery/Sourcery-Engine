#pragma once

#include "core/pipeline/src3_pipeline.h"
#include "core/device/src3_device.h"
#include "core/swapchain/src3_swap_chain.h"
#include "core/buffer/uniform/src3_ubo.h"
#include "game/camera/src3_camera.h"
#include "game/gameobject/src3_game_object.h"
#include "game/ecs/src3_ecs.h"
#include "util/src3_frame_info.h"

#include <memory>
#include <vector>

namespace src3 {

	struct TransformUboData {
		glm::mat4 modelMatrix{1.f};
		glm::mat4 normalMatrix{1.f};
	};

	struct TextureUboData {
		std::shared_ptr<SrcTexture> diffuseMap;
	};

	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(SrcDevice& device, EntManager &ecs,VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout );
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		std::unique_ptr<SrcTexture> getDefaultTexture() { return std::make_unique<SrcTexture>(srcDevice,"../textures/missing.png"); };

		void renderGameObjects(FrameInfo& frameInfo);
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		
		SrcDevice& srcDevice;
		SrcUbo<TransformUboData> transformUbo{srcDevice, 1000, false, true};
		SrcUbo<TextureUboData> textureUbo{srcDevice, 1000, false, true};

		std::unique_ptr<SrcPipeline> srcPipeline;
		VkPipelineLayout pipelineLayout;

		std::unique_ptr<SrcDescriptorSetLayout> renderSystemLayout;

		std::unique_ptr<SrcTexture> defaultTexture = std::make_unique<SrcTexture>(srcDevice,"../textures/missing.png");

		EntQueryResult ents;

		friend struct TextureUboData;
	};
}