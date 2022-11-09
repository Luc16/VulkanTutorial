//
// Created by luc on 07/11/22.
//

#include "DescriptorSetLayout.h"

namespace vtt {

    DescriptorSetLayout::DescriptorSetLayout(const Device &device, const std::vector<VkDescriptorSetLayoutBinding>& bindings): m_deviceRef(device) {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS){
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(m_deviceRef.device(), m_layout, nullptr);
    }

    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(VkDescriptorSetLayoutBinding binding) {
        for (const auto &item: bindings) {
            if (item.binding == binding.binding){
                throw std::runtime_error("Trying to add two VkDescriptorSetLayoutBinding with the same binding");
            }
        }
        bindings.push_back(binding);
        return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return std::make_unique<DescriptorSetLayout>(m_deviceRef, bindings);
    }
}