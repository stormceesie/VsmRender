#include "SteelSightDescriptor.hpp"

#include <cassert>
#include <stdexcept>

namespace Voortman {
    // *************** Descriptor Set Layout Builder *********************

    SteelSightDescriptorSetLayout::Builder& SteelSightDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<SteelSightDescriptorSetLayout> SteelSightDescriptorSetLayout::Builder::build() const {
        return std::make_unique<SteelSightDescriptorSetLayout>(SSDevice, bindings);
    }

    // *************** Descriptor Set Layout *********************

    SteelSightDescriptorSetLayout::SteelSightDescriptorSetLayout(
        SteelSightDevice& SSDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : SSDevice{ SSDevice }, bindings{ bindings } {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto& kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
            SSDevice.device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    SteelSightDescriptorSetLayout::~SteelSightDescriptorSetLayout() {
        if (descriptorSetLayout) {
            vkDestroyDescriptorSetLayout(SSDevice.device(), descriptorSetLayout, nullptr);
        }
    }

    // *************** Descriptor Pool Builder *********************

    SteelSightDescriptorPool::Builder& SteelSightDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    SteelSightDescriptorPool::Builder& SteelSightDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    SteelSightDescriptorPool::Builder& SteelSightDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<SteelSightDescriptorPool> SteelSightDescriptorPool::Builder::build() const {
        return std::make_unique<SteelSightDescriptorPool>(SSDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    SteelSightDescriptorPool::SteelSightDescriptorPool(
        SteelSightDevice& SSDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : SSDevice{ SSDevice } {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(SSDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    SteelSightDescriptorPool::~SteelSightDescriptorPool() {
        if (descriptorPool) {
            vkDestroyDescriptorPool(SSDevice.device(), descriptorPool, nullptr);
        }
    }

    bool SteelSightDescriptorPool::allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(SSDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void SteelSightDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            SSDevice.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void SteelSightDescriptorPool::resetPool() {
        vkResetDescriptorPool(SSDevice.device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    SteelSightDescriptorWriter::SteelSightDescriptorWriter(SteelSightDescriptorSetLayout& setLayout, SteelSightDescriptorPool& pool)
        : setLayout{ setLayout }, pool{ pool } {}

    SteelSightDescriptorWriter& SteelSightDescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    SteelSightDescriptorWriter& SteelSightDescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool SteelSightDescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void SteelSightDescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.SSDevice.device(), (uint32_t)writes.size(), writes.data(), 0, nullptr);
    }
}