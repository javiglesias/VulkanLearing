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
            RenderPass();
            void CreateDepthAttachment(VkFormat _format, VkImageLayout _finalLayout);
            void CreateColorAttachment(VkFormat _format);
            void CreateSubPass();
            void CreateRenderPass(VkDevice _LogicDevice);
        public:
            VkRenderPass m_RenderPass;
        private:
            VkSubpassDescription m_Subpass;
            std::vector<VkAttachmentDescription> m_ColorAttachments;
            std::vector<VkAttachmentDescription> m_DepthAttachments;
            
        };
    }
}