#pragma once

#include "core/device/src3_device.h"
#include "core/buffer/uniform/src3_descriptors.h"
#include "core/swapchain/src3_swap_chain.h"
#include "core/pipeline/src3_pipeline.h"
#include "render/renderer/src3_renderer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "game/ecs/entt.hpp"
#include "game/gameobject/src3_game_object.h"

// std
#include <memory>
#include <string>
#include <cassert>

// TODO: TEMPORARY - remove when editor will be completed
// sourcery engine imgui / editor definitions
#define SE_IMGUI_VIEWPORTS
#define SE_EDITOR
//

namespace src3 {
  [[maybe_unused]] static void check_vk_result(VkResult err) {
    if (err == 0) return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0) abort();
  }

  class SrcEditor{
  public:
      SrcEditor(SrcDevice &device, SrcRenderer &renderer);


  private:
      void createRenderPass();

      SrcDevice    &srcDevice;
      SrcRenderer  &srcRenderer;
      SrcSwapChain *srcSwapChain;

      VkRenderPass renderPass;
      VkCommandPool cmdPool;
      std::vector<VkCommandBuffer> commandBuffers;
  };

  class SrcImGui {
  public:
    SrcImGui(SrcDevice &device,
      SrcWindow &window,
      SrcRenderer &renderer,
      entt::registry &ecs);
    ~SrcImGui();

    // delete copy constructors
    SrcImGui(const SrcImGui &) = delete;
    SrcImGui &operator=(const SrcImGui &) = delete;

    void newFrame();

    void run();

    void render(VkCommandBuffer commandBuffer);

  private:
    SrcDevice   &srcDevice;
    SrcWindow   &srcWindow;
    SrcRenderer &srcRenderer;
    std::unique_ptr<SrcDescriptorPool> descriptorPool;
    entt::registry &ecs;
  };

}