#include "src3_renderer.h"

#include <stdexcept>
#include <cassert>
#include <array>

namespace src3 {

	SrcRenderer::SrcRenderer(SrcWindow& window, SrcDevice& device) : srcWindow{ window }, srcDevice{device} {
		recreateSwapChain();
		createCommandBuffers();
        createCommandBuffers(&viewportCommandBuffers,device.getViewportCommandPool());
	}

	SrcRenderer::~SrcRenderer() {freeCommandBuffers();}

	void SrcRenderer::recreateSwapChain()
	{
		auto extent = srcWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = srcWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(srcDevice.device());

		if (srcSwapChain == nullptr) {
			srcSwapChain = std::make_unique<SrcSwapChain>(srcDevice, extent);
		} else {
			std::shared_ptr<SrcSwapChain> oldSwapChain = std::move(srcSwapChain);
			srcSwapChain = std::make_unique<SrcSwapChain>(srcDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*srcSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}

		// TODO: do this
	}

	void SrcRenderer::createCommandBuffers() {
		commandBuffers.resize(SrcSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = srcDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(srcDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
	};

    void SrcRenderer::createCommandBuffers(std::vector<VkCommandBuffer> *cmdBuffers, VkCommandPool cmdPool) {
        cmdBuffers->resize(SrcSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = cmdPool;
        allocInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers->size());

        if (vkAllocateCommandBuffers(srcDevice.device(), &allocInfo, cmdBuffers->data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }
    };

	void SrcRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			srcDevice.device(),
			srcDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data()
		);
		commandBuffers.clear();
	}

	VkCommandBuffer SrcRenderer::beginFrame()
	{
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");
		auto result = srcSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

        beginCommandBuffer(commandBuffer);
		return commandBuffer;
	}

	void SrcRenderer::endFrame()
	{
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        for (const auto &buffer: submitingBuffers) {
            if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer");
            }
        }

		auto result = srcSwapChain->submitCommandBuffers(&submitingBuffers, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || srcWindow.wasWindowResized()) {
			srcWindow.resetWindowResizedFlag();
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % SrcSwapChain::MAX_FRAMES_IN_FLIGHT;
        submitingBuffers.clear();
	}

    VkCommandBuffer SrcRenderer::beginCommandBuffer(VkCommandBuffer commandBuffer,VkBufferUsageFlagBits usageFlagBits) {
        if (std::find(submitingBuffers.begin(), submitingBuffers.end(),commandBuffer) == std::end(submitingBuffers))
            submitingBuffers.push_back(commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = usageFlagBits;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording the command buffer");
        }
        return commandBuffer;
    }

	void SrcRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = srcSwapChain->getRenderPass();
		renderPassInfo.framebuffer = srcSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = srcSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.01f,0.1f,0.1f,1.0f} };
		clearValues[1].depthStencil = { 1.0f,0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(srcSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(srcSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, srcSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void SrcRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call endSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}
}