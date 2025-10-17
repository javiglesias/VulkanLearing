#include "VKRRenderPass.h"
#include <signal.h>

namespace VKR
{
    namespace render
    {
        RenderPass::RenderPass(VkRenderPass _Pass) : pass(_Pass), subpass{}
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
			attachments.push_back(attachment);
		}

		void RenderPass::CreateGeometryAttachment(VkFormat _format)
		{
			VkAttachmentDescription attachment{};
			attachment.format = VK_FORMAT_R32G32B32A32_UINT;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			attachments.push_back(attachment);
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
			attachments.push_back(depthAttachment);
		}

		void RenderPass::CreateSubPass()
		{
			// SUB-PASSES
			/// Attachment References
			// color
			color_ref.attachment = 0;
			color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// depth
			depth_ref.attachment = 1;
			depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			/// Sub-pass
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_ref;
			subpass.pDepthStencilAttachment = &depth_ref;
		}

		void RenderPass::CreateDepthOnlySubPass()
		{
			/// Attachment References
			depth_ref.attachment = 0;
			depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			/// Sub-pass
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.pDepthStencilAttachment = &depth_ref;
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
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &subpassDep;
			if (vkCreateRenderPass(_LogicDevice, &renderPassInfo, nullptr,
				&pass) != VK_SUCCESS)
#ifdef _MSVC
				__debugbreak();
#else
					raise(SIGTRAP);
#endif
		}

		void RenderPass::Cleanup(VkDevice _LogicDevice)
		{
			vkDestroyRenderPass(_LogicDevice, pass, nullptr);
		}
    }
}
