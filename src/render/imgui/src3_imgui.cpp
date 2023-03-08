#include "src3_imgui.h"

#define IMGUI_IMPLEMENTATION
#include <imgui/misc/single_file/imgui_single_file.h>
#include <imgui/backends/imgui_impl_vulkan.cpp>
#include <imgui/backends/imgui_impl_glfw.cpp>

// std
#include <cmath>
#include <stdexcept>

namespace src3 {
	SrcImGui::SrcImGui(SrcDevice &device,
		SrcWindow &window,
		SrcRenderer &renderer,
		VkRenderPass renderPass) 
		: srcDevice{device}, srcWindow{window}, srcRenderer{renderer}
	{
		descriptorPool = SrcDescriptorPool::Builder(srcDevice)
			.setMaxSets(1000 * 10 /* the amount of sizes */)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER,1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.build();

		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForVulkan(srcWindow.getGLFWWindow(),true);

		ImGui_ImplVulkan_InitInfo info;
		info.Instance = device.getInstance();
		info.DescriptorPool = descriptorPool->getDescriptorPool();
		info.Device = srcDevice.device();
		info.PhysicalDevice = srcDevice.getPhysicalDevice();
		info.ImageCount = srcRenderer.getImageCount();
		info.MinImageCount = SrcSwapChain::MAX_FRAMES_IN_FLIGHT;
		info.Queue = srcDevice.graphicsQueue();
		info.QueueFamily = srcDevice.findPhysicalQueueFamilies().graphicsFamily;
		info.Subpass = 0;
		info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		info.CheckVkResultFn = check_vk_result;
		info.PipelineCache = VK_NULL_HANDLE;
		info.Allocator = VK_NULL_HANDLE;

		ImGui_ImplVulkan_Init(&info,renderPass);

		VkCommandBuffer commandBuffer = srcDevice.beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		srcDevice.endSingleTimeCommands(commandBuffer);

		vkDeviceWaitIdle(srcDevice.device());
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	SrcImGui::~SrcImGui() {
		ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vkDestroyDescriptorPool(srcDevice.device(), descriptorPool->getDescriptorPool(), nullptr);
	}

	void SrcImGui::newFrame() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void SrcImGui::run() {
		ImGui::ShowDemoWindow();
	}

	void SrcImGui::render(VkCommandBuffer commandBuffer) {
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),commandBuffer);
	}
} 
