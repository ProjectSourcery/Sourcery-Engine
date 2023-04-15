#include "core/device/src3_device.h"
#include "core/buffer/uniform/src3_descriptors.h"
#include "core/swapchain/src3_swap_chain.h"
#include "render/renderer/src3_renderer.h"
#include "game/ecs/entt.hpp"
#include "game/gameobject/src3_game_object.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define IMGUI_IMPLEMENTATION
#include <imgui/misc/single_file/imgui_single_file.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>

// std
#include <memory>
#include <cassert>
#include <stdexcept>

// TODO: TEMPORARY - remove when editor will be completed
// sourcery engine imgui / editor definitions
#define SE_IMGUI_VIEWPORTS
#define SE_EDITOR
//

#include <rttr/type>
using namespace rttr;

namespace src3 {
    [[maybe_unused]] static void check_vk_result(VkResult err) {
        if (err == 0) return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0) abort();
    }

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

    class SrcEditor{
    public:
        SrcEditor(SrcDevice &device, SrcRenderer &renderer)
                : srcDevice{device}, srcRenderer{renderer}, srcSwapChain{renderer.getSwapChain()} {
            createRenderPass();
            srcDevice.createCommandPool(&cmdPool);
            srcRenderer.createCommandBuffers(&commandBuffers,cmdPool);
            createFrameBuffers();
            createTextureSampler();

            std::vector<VkImageView> viewportImageViews = srcSwapChain->getViewportImageViews();
            descriptorSets.resize(viewportImageViews.size());
            for (unsigned int i = 0; i < viewportImageViews.size(); i++) {
                descriptorSets[i] = ImGui_ImplVulkan_AddTexture(texSampler,viewportImageViews[i],VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
        }

        VkRenderPass getRenderPass() const { return renderPass; };
        VkCommandPool getCommandPool() const { return cmdPool; };
        std::vector<VkDescriptorSet> getDescriptorSets() const { return descriptorSets; };
        VkCommandBuffer getCurrentCommandBuffer() const {
            return commandBuffers[getFrameIndex()];
        }

        VkFramebuffer getCurrentFrameBuffer() const {
            return frameBuffers[getFrameIndex()];
        }

        int getFrameIndex() const {
            return srcRenderer.getFrameIndex();
        }

    private:
        inline void createRenderPass(){
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

        inline void createFrameBuffers(){
            frameBuffers.resize(imguiImageViews.size());
            for (size_t i = 0; i < imguiImageViews.size(); i++) {
                VkImageView attachment[1];

                VkExtent2D swapChainExtent = srcSwapChain->getSwapChainExtent();
                VkFramebufferCreateInfo framebufferInfo = {};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachment;
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(
                        srcDevice.device(),
                        &framebufferInfo,
                        nullptr,
                        &frameBuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            }
        }

        inline void createTextureSampler(){
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;

            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = 16.0f;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;

            // these fields useful for percentage close filtering for shadow maps
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = 1;

            if (vkCreateSampler(srcDevice.device(), &samplerInfo, nullptr, &texSampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }

        SrcDevice    &srcDevice;
        SrcRenderer  &srcRenderer;
        SrcSwapChain *srcSwapChain;

        VkRenderPass renderPass;
        VkCommandPool cmdPool;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkFramebuffer> frameBuffers;
        std::vector<VkDescriptorSet> descriptorSets;
        VkSampler texSampler;

        std::vector<VkImage> imguiImages;
        std::vector<VkDeviceMemory> imguiImageMemorys;
        std::vector<VkImageView> imguiImageViews;
    };



    class SrcImGui {
    public:
        SrcImGui(SrcDevice &device,
                 SrcWindow &window,
                 SrcRenderer &renderer,
                 entt::registry &ecs)
                 : srcDevice{device}, srcWindow{window}, srcRenderer{renderer}, ecs{ecs}, im_context{ImGui::CreateContext()}
#ifdef SE_EDITOR
                ,editor{device,renderer}
#endif
        {
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

#ifdef SE_EDITOR
            ImGui_ImplVulkan_Init(&info,editor.getRenderPass());

            VkCommandBuffer commandBuffer = srcDevice.beginSingleTimeCommands(editor.getCommandPool());
            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
            srcDevice.endSingleTimeCommands(commandBuffer);
#else
            ImGui_ImplVulkan_Init(&info, renderer.getSwapChainRenderPass());

        VkCommandBuffer commandBuffer = srcDevice.beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        srcDevice.endSingleTimeCommands(commandBuffer);
#endif

            vkDeviceWaitIdle(srcDevice.device());
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
        ~SrcImGui()
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            vkDestroyDescriptorPool(srcDevice.device(), descriptorPool->getDescriptorPool(), nullptr);
        };

        // delete copy constructors
        SrcImGui(const SrcImGui &) = delete;
        SrcImGui &operator=(const SrcImGui &) = delete;

        inline void newFrame(){
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        };

        inline void run(){
            ImGui::ShowDemoWindow();

#ifdef SE_EDITOR
            ImGui::Begin("Viewport");

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            ImGui::Image(editor.getDescriptorSets()[srcRenderer.getFrameIndex()], ImVec2{viewportPanelSize.x, viewportPanelSize.y});

            ImGui::End();
#endif
        }

        inline void render(VkCommandBuffer commandBuffer) {
            ImGui::Render();
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
#ifdef SE_EDITOR
            auto cmdBuffer = srcRenderer.beginCommandBuffer(editor.getCurrentCommandBuffer());
            {
                std::vector<VkClearValue> clearValues;
                clearValues[0].color = { {0.01f,0.1f,0.1f,1.0f} };
                srcRenderer.beginRenderPass(cmdBuffer,editor.getRenderPass(),editor.getCurrentFrameBuffer(),clearValues);
            }

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);

            srcRenderer.endRenderPass(cmdBuffer);
#else
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
#endif
        }

    private:
        SrcDevice   &srcDevice;
        SrcWindow   &srcWindow;
        SrcRenderer &srcRenderer;
        std::unique_ptr<SrcDescriptorPool> descriptorPool;
        entt::registry &ecs;
        ImGuiContext *im_context;
#ifdef SE_EDITOR
        SrcEditor editor;
#endif
    };
}
