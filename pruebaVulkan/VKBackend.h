#include "VKRenderers.h"
namespace VKR
{
    namespace render
    {

    }
}

VkRenderPass m_RenderPass;
auto m_GraphicsRender = m_GraphicsRenderer();
auto m_DbgRender = m_DebugRenderer();

void CreateRenderPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo, VkRenderPass* _RenderPass)
    {
        // RENDER PASES
        /// Attachment description
        // Color
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_SwapChainCreateInfo->imageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        // Depth
        VkAttachmentDescription depthAttachment{};
        // depthAttachment.format = FindDepthTestFormat(); // d32_SFLOAT
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        // SUB-PASSES
        /// Attachment References
        // color
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // depth
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        /// Sub-pass
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        /// Subpass dependencies
        VkSubpassDependency subpassDep{};
        subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDep.dstSubpass = 0;
        subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDep.srcAccessMask = 0;
        subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        /// Render pass
        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &subpassDep;
        if (vkCreateRenderPass(m_LogicDevice, &renderPassInfo, nullptr,
            _RenderPass) != VK_SUCCESS)
            exit(-8);

        // m_RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        // m_RenderPassInfo.attachmentCount = 1;
        // m_RenderPassInfo.pAttachments = &m_ColorAttachment;
        // m_RenderPassInfo.subpassCount = 1;
        // m_RenderPassInfo.pSubpasses = &m_Subpass;
        // m_RenderPassInfo.dependencyCount = 1;
        // m_RenderPassInfo.pDependencies = &m_SubpassDep;
        // if (vkCreateRenderPass(m_LogicDevice, &m_RenderPassInfo, nullptr,
        // 	&m_UIRenderPass) != VK_SUCCESS)
        // 	exit(-8);
    }
