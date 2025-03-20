#ifndef RENDER_COMMON_H_
#define RENDER_COMMON_H_

#include <vulkan/vulkan_core.h>
namespace VKR
{
    namespace render
    {
        struct sRenderPipeline
        {
            VkPipeline pipeline;
            VkPipeline compute;
            VkPipelineLayout layout;
            VkPipelineLayout compute_layout;
            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSetLayout compute_descriptorSetLayout;
        };
    }
}
#endif

