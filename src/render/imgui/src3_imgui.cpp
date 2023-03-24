#include "src3_imgui.h"

#define IMGUI_IMPLEMENTATION

#include <imgui/misc/single_file/imgui_single_file.h>
#include <imgui/backends/imgui_impl_vulkan.cpp>
#include <imgui/backends/imgui_impl_glfw.cpp>

#include <rttr/type>

// std
#include <stdexcept>

namespace src3 {
    class ImGuiWindowHelpers {
    public:
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
        void addAllComponentsOfEntity(entt::registry &ecs, entt::entity entity, ComponentHashClass componentHashClass) {
            std::vector<entt::any> components;
            std::map<entt::id_type,std::string> compHashMap = componentHashClass.getComponentHashMap();
            for (auto &&curr: ecs.storage()) {
                if (auto &storage = curr.second; storage.contains(entity)) {
                    entt::id_type id = curr.first;

                    // TODO: do something with this: storage.value(entity);

                    components.emplace_back(compHashMap[id]);
                }
            }
        }

        bool open_window = true;
        ImGuiIO &io = ImGui::GetIO();
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
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }
} 
