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
		VkDescriptorPool descriptorPool,
		VkRenderPass renderPass) 
		: srcDevice{device}, srcWindow{window}
	{
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForVulkan(srcWindow.getGLFWWindow(),true);

		ImGui_ImplVulkan_InitInfo info;
		info.Instance = device.getInstance();
		info.DescriptorPool = descriptorPool;
		info.Device = srcDevice.device();
		info.PhysicalDevice = srcDevice.getPhysicalDevice();
		info.ImageCount = SrcSwapChain::MAX_FRAMES_IN_FLIGHT;
		info.MinImageCount = SrcSwapChain::MAX_FRAMES_IN_FLIGHT;
		info.Queue = srcDevice.graphicsQueue();
		info.QueueFamily = srcDevice.findPhysicalQueueFamilies().graphicsFamily;
		info.Subpass = 0;
		info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

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
	}

	void SrcImGui::render(VkPipeline pipeline,VkCommandBuffer commandBuffer) {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		// TODO: add a render system of some sorts
		ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),commandBuffer,pipeline);
	}
} 
