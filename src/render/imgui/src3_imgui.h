#pragma once

#include "core/device/src3_device.h"
#include "core/buffer/uniform/src3_descriptors.h"
#include "core/swapchain/src3_swap_chain.h"
#include "render/renderer/src3_renderer.h"

// std
#include <memory>
#include <string>

namespace src3 {
  class SrcImGui {
  public:
    SrcImGui(SrcDevice &device,
      SrcWindow &window,
      SrcRenderer &renderer,
      VkDescriptorPool descriptorPool,
      VkRenderPass renderPass);
    ~SrcImGui();

    // delete copy constructors
    SrcImGui(const SrcImGui &) = delete;
    SrcImGui &operator=(const SrcImGui &) = delete;

    void render(VkPipeline pipeline,VkCommandBuffer commandBuffer);

  private:
    SrcDevice   &srcDevice;
    SrcWindow   &srcWindow;
    SrcRenderer &srcRenderer;
  };

}