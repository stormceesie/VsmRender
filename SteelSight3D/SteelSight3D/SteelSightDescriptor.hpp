#pragma once

#include "SteelSightDevice.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace Voortman {

    class SteelSightDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(SteelSightDevice& lveDevice) : SSDevice{ lveDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<SteelSightDescriptorSetLayout> build() const;

        private:
            SteelSightDevice& SSDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        SteelSightDescriptorSetLayout(
            SteelSightDevice& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~SteelSightDescriptorSetLayout();
        SteelSightDescriptorSetLayout(const SteelSightDescriptorSetLayout&) = delete;
        SteelSightDescriptorSetLayout& operator=(const SteelSightDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        SteelSightDevice& SSDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class SteelSightDescriptorWriter;
    };

    class SteelSightDescriptorPool {
    public:
        class Builder {
        public:
            Builder(SteelSightDevice& SSDevice) : SSDevice{ SSDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<SteelSightDescriptorPool> build() const;

        private:
            SteelSightDevice& SSDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        SteelSightDescriptorPool(
            SteelSightDevice& lveDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~SteelSightDescriptorPool();
        SteelSightDescriptorPool(const SteelSightDescriptorPool&) = delete;
        SteelSightDescriptorPool& operator=(const SteelSightDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        SteelSightDevice& SSDevice;
        VkDescriptorPool descriptorPool;

        friend class SteelSightDescriptorWriter;
    };

    class SteelSightDescriptorWriter {
    public:
        SteelSightDescriptorWriter(SteelSightDescriptorSetLayout& setLayout, SteelSightDescriptorPool& pool);

        SteelSightDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        SteelSightDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        SteelSightDescriptorSetLayout& setLayout;
        SteelSightDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace lve