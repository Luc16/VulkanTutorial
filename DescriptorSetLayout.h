//
// Created by luc on 07/11/22.
//

#ifndef VULKANTUTORIAL_DESCRIPTORSETLAYOUT_H
#define VULKANTUTORIAL_DESCRIPTORSETLAYOUT_H

#include "Device.h"

namespace vtt {
    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            explicit Builder(const Device& device): m_deviceRef(device) {}

            Builder& addBinding(VkDescriptorSetLayoutBinding binding);
            [[nodiscard]] std::unique_ptr<DescriptorSetLayout> build() const;

        private:
            const Device& m_deviceRef;
            std::vector<VkDescriptorSetLayoutBinding> bindings{};
        };

        DescriptorSetLayout(const Device &device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
        ~DescriptorSetLayout();
        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        [[nodiscard]] VkDescriptorSetLayout descriptorSetLayout() const { return m_layout; }


    private:
        const Device& m_deviceRef;
        VkDescriptorSetLayout m_layout{};
    };
}



#endif //VULKANTUTORIAL_DESCRIPTORSETLAYOUT_H
