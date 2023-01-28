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

// std
#include <memory>
#include <string>
#include <cassert>

namespace src3 {
  static void check_vk_result(VkResult err) {
    if (err == 0) return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0) abort();
  }

  class SrcImGui {
  public:
    SrcImGui(SrcDevice &device,
      SrcWindow &window,
      SrcRenderer &renderer,
      VkRenderPass renderPass);
    ~SrcImGui();

    // delete copy constructors
    SrcImGui(const SrcImGui &) = delete;
    SrcImGui &operator=(const SrcImGui &) = delete;

    struct Vertex {
			glm::vec3 position{};
      glm::vec2 uv{};
			glm::vec3 color{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && uv  == other.uv;
			}
		};

    void newFrame();

    void run();

    void render(VkCommandBuffer commandBuffer);

  private:
    SrcDevice   &srcDevice;
    SrcWindow   &srcWindow;
    SrcRenderer &srcRenderer;
    std::unique_ptr<SrcDescriptorPool> descriptorPool;
  };

}