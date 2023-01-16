#pragma once

#include "core/device/src3_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace src3 {

    class SrcDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(SrcDevice& srcDevice) : srcDevice{ srcDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<SrcDescriptorSetLayout> build() const;

        private:
            SrcDevice& srcDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        SrcDescriptorSetLayout(
            SrcDevice& srcDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~SrcDescriptorSetLayout();
        SrcDescriptorSetLayout(const SrcDescriptorSetLayout&) = delete;
        SrcDescriptorSetLayout& operator=(const SrcDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        SrcDevice& srcDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class SrcDescriptorWriter;
    };

    class SrcDescriptorPool {
    public:
        class Builder {
        public:
            Builder(SrcDevice& srcDevice) : srcDevice{ srcDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<SrcDescriptorPool> build() const;

        private:
            SrcDevice& srcDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        SrcDescriptorPool(
            SrcDevice& srcDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~SrcDescriptorPool();
        SrcDescriptorPool(const SrcDescriptorPool&) = delete;
        SrcDescriptorPool& operator=(const SrcDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        SrcDevice& srcDevice;
        VkDescriptorPool descriptorPool;

        friend class SrcDescriptorWriter;
    };

    class SrcDescriptorWriter {
    public:
        SrcDescriptorWriter(SrcDescriptorSetLayout& setLayout, SrcDescriptorPool& pool);

        SrcDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        SrcDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        SrcDescriptorSetLayout& setLayout;
        SrcDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}