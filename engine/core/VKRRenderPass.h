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
            VkRenderPass pass;
        public:
            RenderPass(VkRenderPass _Pass = VK_NULL_HANDLE);
            void CreateDepthAttachment(VkFormat _format, VkImageLayout _finalLayout);
            void CreateColorAttachment(VkFormat _format);
            void CreateGeometryAttachment(VkFormat _format);
            void CreateSubPass();
            void CreateDepthOnlySubPass();
            void CreatePass(VkDevice _LogicDevice);
            void Cleanup(VkDevice _LogicDevice);
			VkRenderPass* GetPass() { return &pass; }
        private:
            VkSubpassDescription subpass;
            VkAttachmentReference color_ref{};
            VkAttachmentReference depth_ref{};
            std::vector<VkAttachmentDescription> attachments;
            
        };
    }
}
#endif