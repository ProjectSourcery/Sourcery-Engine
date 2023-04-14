#pragma once

#include "core/device/src3_device.h"
#include "core/window/src3_window.h"
#include "core/swapchain/src3_swap_chain.h"

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
        VkRenderPass getViewportRenderPass() const { return srcSwapChain->getViewportRenderPass(); }
		float getAspectRatio() const { return srcSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; };
		uint32_t getImageCount() const { return srcSwapChain->imageCount(); }
        SrcSwapChain* getSwapChain() const { return srcSwapChain.get(); }

		VkCommandBuffer getCurrentCommandBuffer() const { 
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex]; 
		}

        VkCommandBuffer getCurrentViewportCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return viewportCommandBuffers[currentFrameIndex];
        }

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

        void createCommandBuffers(std::vector<VkCommandBuffer> *cmdBuffers,VkCommandPool cmdPool);

		VkCommandBuffer beginFrame();
		void endFrame();

        VkCommandBuffer beginCommandBuffer(VkCommandBuffer commandBuffer,VkBufferUsageFlagBits usageFlagBits = (VkBufferUsageFlagBits)0);

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
        std::vector<VkCommandBuffer> viewportCommandBuffers;

        std::vector<VkCommandBuffer> submitingBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{false};
	};
}