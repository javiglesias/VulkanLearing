#pragma once
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <vector>
namespace VKR
{
    namespace render
    {
        class RenderPass
        {
        public:
            RenderPass(VkRenderPass _Pass = VK_NULL_HANDLE);
            void CreateDepthAttachment(VkFormat _format, VkImageLayout _finalLayout);
            void CreateColorAttachment(VkFormat _format);
            void CreateSubPass();
            void CreateDepthOnlySubPass();
            void CreateRenderPass(VkDevice _LogicDevice);
            void Cleanup(VkDevice _LogicDevice);
        public:
            VkRenderPass m_Pass;
        private:
            VkSubpassDescription m_Subpass;
            VkAttachmentReference colorAttachmentRef{};
            VkAttachmentReference depthAttachmentRef{};
            std::vector<VkAttachmentDescription> m_ColorAttachments;
            std::vector<VkAttachmentDescription> m_DepthAttachments;
            
        };
    }
}