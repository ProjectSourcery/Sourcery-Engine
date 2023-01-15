#pragma once

#include "src3_device.h"
#include "src3_window.h"
#include "src3_swap_chain.h"

#include <memory>
#include <vector>
#include <cassert>

namespace src3 {
	class SrcRenderer {
	public:
		SrcRenderer(SrcWindow &window, SrcDevice &device);
		~SrcRenderer();

		SrcRenderer(const SrcRenderer&) = delete;
		SrcRenderer& operator=(const SrcRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return srcSwapChain->getRenderPass(); }
		float getAspectRatio() const { return srcSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; };

		VkCommandBuffer getCurrentCommandBuffer() const { 
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex]; 
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		SrcWindow& srcWindow;
		SrcDevice& srcDevice;
		std::unique_ptr<SrcSwapChain> srcSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{false};
	};
}