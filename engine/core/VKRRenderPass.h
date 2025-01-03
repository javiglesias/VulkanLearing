#ifndef _C_RENDER_PASS
#define _C_RENDER_PASS
#include <vulkan/vulkan_core.h>
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
            void CreatePass(VkDevice _LogicDevice);
            void Cleanup(VkDevice _LogicDevice);
			VkRenderPass* GetPass() { return &m_Pass; }
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
#endif