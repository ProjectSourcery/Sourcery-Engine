#pragma once

#include "core/device/src3_device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <memory>
#include <vector>

namespace src3 {

    class SrcSwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SrcSwapChain(SrcDevice& deviceRef, VkExtent2D windowExtent);
        SrcSwapChain(SrcDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SrcSwapChain> previous);
        ~SrcSwapChain();

        SrcSwapChain(const SrcSwapChain&) = delete;
        SrcSwapChain operator=(const SrcSwapChain&) = delete;

        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkFramebuffer getViewportFrameBuffer(int index) { return swapChainViewportFramebuffers[index]; }
        VkRenderPass getRenderPass() { return renderPass; }
        VkRenderPass getViewportRenderPass() const { return viewportRenderPass; }
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }
        size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }

        std::vector<VkImageView> getViewportImageViews() const { return viewportImageViews; }
        std::vector<VkImageView> getSwapchainImageViews() const { return swapChainImageViews; }

        float extentAspectRatio() {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t* imageIndex);
        VkResult submitCommandBuffers(const std::vector<VkCommandBuffer>* buffers, uint32_t* imageIndex);

        bool compareSwapFormats(const SrcSwapChain& swapChain) const {
            return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat;
        }

    private:
        void init();
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createViewportResources();
        void createRenderPass();
        void createViewportRenderPass();
        void createFramebuffers();
        void createViewportFramebuffers();
        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat swapChainImageFormat;
        VkFormat swapChainDepthFormat;
        VkFormat swapChainViewportImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        std::vector<VkFramebuffer> swapChainViewportFramebuffers;

        VkRenderPass renderPass;
        VkRenderPass viewportRenderPass;


        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;

        std::vector<VkImage> viewportImages;
        std::vector<VkDeviceMemory> viewportImageMemorys;
        std::vector<VkImageView> viewportImageViews;

        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;


        SrcDevice& device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;
        std::shared_ptr<SrcSwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };

}