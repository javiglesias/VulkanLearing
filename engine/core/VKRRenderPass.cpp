#include "VKRRenderPass.h"
#include <signal.h>

namespace VKR
{
    namespace render
    {
        RenderPass::RenderPass(VkRenderPass _Pass) : m_Pass(_Pass), m_Subpass{}
        {}

		void RenderPass::CreateColorAttachment(VkFormat _format)
		{
			VkAttachmentDescription attachment{};
			attachment.format = _format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			m_ColorAttachments.push_back(attachment);
		}

		void RenderPass::CreateDepthAttachment(VkFormat _format, VkImageLayout _finalLayout)
		{
			// Depth
			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = _format;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = _finalLayout;
			m_DepthAttachments.push_back(depthAttachment);
		}

		void RenderPass::CreateSubPass()
		{
			// SUB-PASSES
			/// Attachment References
			// color
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// depth
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			/// Sub-pass
			m_Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			m_Subpass.colorAttachmentCount = m_ColorAttachments.size();
			m_Subpass.pColorAttachments = &colorAttachmentRef;
			m_Subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}

		void RenderPass::CreateDepthOnlySubPass()
		{
			/// Attachment References
			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 0;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			/// Sub-pass
			m_Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			m_Subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}

		void RenderPass::CreatePass(VkDevice _LogicDevice)
		{
			/// Subpass dependencies
			VkSubpassDependency subpassDep{};
			subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDep.dstSubpass = 0;
			subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			subpassDep.srcAccessMask = 0;
			subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			/// Render pass
			std::vector<VkAttachmentDescription> attachments;
			attachments.insert(attachments.end(), m_ColorAttachments.begin(), m_ColorAttachments.end());
			attachments.insert(attachments.end(), m_DepthAttachments.begin(), m_DepthAttachments.end());
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &m_Subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &subpassDep;
			if (vkCreateRenderPass(_LogicDevice, &renderPassInfo, nullptr,
				&m_Pass) != VK_SUCCESS)
				raise(SIGTRAP);
		}

		void RenderPass::Cleanup(VkDevice _LogicDevice)
		{
			vkDestroyRenderPass(_LogicDevice, m_Pass, nullptr);
		}
    }
}
