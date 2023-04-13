#include "src3_imgui.h"

#define IMGUI_IMPLEMENTATION
#define IMGUI_DEFINE_MATH_OPERATIONS

#include <imgui/misc/single_file/imgui_single_file.h>
#include <imgui/backends/imgui_impl_vulkan.cpp>
#include <imgui/backends/imgui_impl_glfw.cpp>

#include <rttr/type>

// std
#include <stdexcept>

using namespace rttr;

namespace src3 {

    namespace imgui {
        // thanks to https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353 for this :)

        static ImVector<ImRect> s_GroupPanelLabelStack;

        [[maybe_unused]] void BeginGroupPanel(const char* name, const ImVec2& size)
        {
            ImGui::BeginGroup();

            auto cursorPos = ImGui::GetCursorScreenPos();
            auto itemSpacing = ImGui::GetStyle().ItemSpacing;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

            auto frameHeight = ImGui::GetFrameHeight();
            ImGui::BeginGroup();

            ImVec2 effectiveSize = size;
            if (size.x < 0.0f)
                effectiveSize.x = ImGui::GetContentRegionAvail().x;
            else
                effectiveSize.x = size.x;
            ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

            ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::BeginGroup();
            ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::TextUnformatted(name);
            auto labelMin = ImGui::GetItemRectMin();
            auto labelMax = ImGui::GetItemRectMax();
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
            ImGui::BeginGroup();

            //ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

            ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
            ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
            ImGui::GetCurrentWindow()->WorkRect.Max.x          -= frameHeight * 0.5f;
            ImGui::GetCurrentWindow()->InnerRect.Max.x         -= frameHeight * 0.5f;
#else
            ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
            ImGui::GetCurrentWindow()->Size.x                   -= frameHeight;

            auto itemWidth = ImGui::CalcItemWidth();
            ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

            s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
        }

        [[maybe_unused]] void EndGroupPanel()
        {
            ImGui::PopItemWidth();

            auto itemSpacing = ImGui::GetStyle().ItemSpacing;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

            auto frameHeight = ImGui::GetFrameHeight();

            ImGui::EndGroup();

            //ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

            ImGui::EndGroup();

            ImGui::SameLine(0.0f, 0.0f);
            ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
            ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

            ImGui::EndGroup();

            auto itemMin = ImGui::GetItemRectMin();
            auto itemMax = ImGui::GetItemRectMax();
            //ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

            auto labelRect = s_GroupPanelLabelStack.back();
            s_GroupPanelLabelStack.pop_back();

            ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
            ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
            labelRect.Min.x -= itemSpacing.x;
            labelRect.Max.x += itemSpacing.x;
            for (int i = 0; i < 4; ++i)
            {
                switch (i)
                {
                    // left half-plane
                    case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
                        // right half-plane
                    case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
                        // top
                    case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
                        // bottom
                    case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
                }

                ImGui::GetWindowDrawList()->AddRect(
                        frameRect.Min, frameRect.Max,
                        ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
                        halfFrame.x);

                ImGui::PopClipRect();
            }

            ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
            ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
            ImGui::GetCurrentWindow()->WorkRect.Max.x          += frameHeight * 0.5f;
            ImGui::GetCurrentWindow()->InnerRect.Max.x         += frameHeight * 0.5f;
#else
            ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
            ImGui::GetCurrentWindow()->Size.x                   += frameHeight;

            ImGui::Dummy(ImVec2(0.0f, 0.0f));

            ImGui::EndGroup();
        }
    }

    namespace SourceryEngine_Editor {
        bool open_window = true;

        class PropertiesWindow {
        public:

            PropertiesWindow(){

            }

            void ShowPropertiesWindow() {
                ImGui::SetNextWindowSize(ImVec2(400, 500));

                if (ImGui::Begin("Properties", &open_window)) {
                    ImGui::Text("TransformComponent");

                    ImGui::BeginChild(7, ImVec2(355, 124), true);

                    ImGui::Text("translation");

                    ImGui::SameLine();

                    ImGui::PushItemWidth(200);
                    static float vec4a12[4] = {0.10f, 0.20f, 0.30f, 0.44f};
                    ImGui::InputFloat3("##", vec4a12);
                    ImGui::PopItemWidth();

                    ImGui::Text("scale");

                    ImGui::SameLine();

                    ImGui::PushItemWidth(200);
                    static float vec4a15[4] = {0.10f, 0.20f, 0.30f, 0.44f};
                    ImGui::InputFloat3("##", vec4a15);
                    ImGui::PopItemWidth();

                    ImGui::Text("rotation");

                    ImGui::SameLine();

                    ImGui::PushItemWidth(200);
                    static float vec4a18[4] = {0.10f, 0.20f, 0.30f, 0.44f};
                    ImGui::InputFloat3("##", vec4a18);
                    ImGui::PopItemWidth();

                    ImGui::EndChild();

                }
                ImGui::End();
            }
        private:
            void addAllComponentsOfEntity(entt::registry &ecs, entt::entity entity) {
                for (auto &&curr: ecs.storage()) {
                    if (auto &storage = curr.second; storage.contains(entity)) {
                        auto comp = storage.value(entity);
                        type compType = rttr::type::get(comp);
                        addComponentToPropWindow(comp,compType);
                    }
                }
            }

            void addComponentToPropWindow(entt::any comp, type compType) {
                for (auto& prop: compType.get_properties()) {

                }
            }

            ImGuiIO &io = ImGui::GetIO();
        };

        class GraphicsViewport {
        public:
            GraphicsViewport(){

            }

        private:
            ImGuiIO &io = ImGui::GetIO();
        };
    };

    SrcImGui::SrcImGui(SrcDevice &device,
                       SrcWindow &window,
                       SrcRenderer &renderer,
                       entt::registry &ecs)
            : srcDevice{device}, srcWindow{window}, srcRenderer{renderer}, ecs{ecs} {
        descriptorPool = SrcDescriptorPool::Builder(srcDevice)
                .setMaxSets(1000 * 10 /* the amount of sizes */)
                .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
                .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                .build();

        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#ifdef SE_IMGUI_VIEWPORTS
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

        ImGui_ImplGlfw_InitForVulkan(srcWindow.getGLFWWindow(), true);

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

        ImGui_ImplVulkan_Init(&info, renderer.getSwapChainRenderPass());

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
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    SrcEditor::SrcEditor(SrcDevice &device, SrcRenderer &renderer)
        : srcDevice{device}, srcRenderer{renderer}, srcSwapChain{renderer.getSwapChain()} {
        createRenderPass();
        srcDevice.createCommandPool(&cmdPool);
        srcRenderer.createCommandBuffers(&commandBuffers,cmdPool);

    }

    void SrcEditor::createRenderPass() {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = srcSwapChain->getSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstSubpass = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(srcDevice.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
}
