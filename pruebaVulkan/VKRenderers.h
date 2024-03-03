#pragma once
#include <vulkan/vulkan_core.h>
#include "VKRShader.h"
#include "Types.h"

namespace VKR
{
    namespace render
    {
        extern std::string g_ConsoleMSG;

         inline auto m_BindingDescription = Vertex3D::getBindingDescription();
         inline auto m_AttributeDescriptions = Vertex3D::getAttributeDescriptions();
         inline auto m_DbgBindingDescription = DBG_Vertex3D::getBindingDescription();
         inline auto m_DbgAttributeDescriptions = DBG_Vertex3D::getAttributeDescriptions();

        struct Renderer
        {
            /// Data needed
            int m_PolygonMode = VK_POLYGON_MODE_FILL;
            VkCullModeFlagBits m_CullMode = VK_CULL_MODE_BACK_BIT;
            VkDescriptorSetLayout m_DescSetLayout;
            VkPipelineLayout m_PipelineLayout;
            VkPipeline m_Pipeline;
            VkPipelineShaderStageCreateInfo m_ShaderStages[2] = {};
            VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{};
            VkPipelineDynamicStateCreateInfo m_DynamicState{};
            VkPipelineVertexInputStateCreateInfo m_VertexInputInfo{};
            VkPipelineViewportStateCreateInfo m_ViewportState{
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
            };
            VkPipelineRasterizationStateCreateInfo m_Rasterizer{};
            VkPipelineMultisampleStateCreateInfo m_Multisampling{};
            VkPipelineDepthStencilStateCreateInfo m_DepthStencil{};
            VkPipelineColorBlendStateCreateInfo m_ColorBlending{};
            VkPipelineColorBlendAttachmentState m_ColorBlendAttachment{};
            VkPipelineShaderStageCreateInfo m_VertShaderStageInfo{};
            VkPipelineShaderStageCreateInfo m_FragShaderStageInfo{};
            VkShaderModule m_VertShaderModule;
            VkShaderModule m_FragShaderModule;
            std::vector<VkDynamicState> m_DynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            VkDevice m_LogicDevice;
        public:
            virtual void Initialize() {}
            virtual void CreateDescriptorSetLayout() {}

            Renderer() {}
            ~Renderer() {}
            void CreateShaderStages();
            void CreatePipelineLayoutSetup(VkExtent2D* _CurrentExtent, VkViewport* _Viewport, VkRect2D* _Scissor);
            void CreatePipeline(VkRenderPass _RenderPass);
            void CreateShaderModule(const char* _shaderPath, VkShaderModule* _shaderModule);
        };


        struct GraphicsRenderer : Renderer
        {
        public: // Functions

            // Creamos el layout de los Descriptor set que vamos a utlizar
            void CreateDescriptorSetLayout() override;
            GraphicsRenderer(VkDevice _LogicalDevice, int _PolygonMode = VK_POLYGON_MODE_FILL)
            {
                m_LogicDevice = _LogicalDevice;
                m_PolygonMode = _PolygonMode;
            }
            void Initialize() override;
            void CreatePipelineLayout();
            void CleanShaderModules();

            ~GraphicsRenderer()
            {
                vkDestroyDescriptorSetLayout(m_LogicDevice, m_DescSetLayout, nullptr);
                vkDestroyPipeline(m_LogicDevice, m_Pipeline, nullptr);
                vkDestroyPipelineLayout(m_LogicDevice, m_PipelineLayout, nullptr);
                // vkDestroyRenderPass(m_LogicDevice, m_UIRenderPass, nullptr);
            }
        };

        struct DebugRenderer : Renderer
        {
        public: // Functions
	        void CleanShaderModules();
            void CreateDescriptorSetLayout() override;
            DebugRenderer(VkDevice _LogicalDevice, int _PolygonMode = VK_POLYGON_MODE_LINE)
            {
                m_LogicDevice = _LogicalDevice;
                m_PolygonMode = _PolygonMode;
            }
            void Initialize() override;
	        void CreatePipelineLayout();

	        ~DebugRenderer()
            {
                vkDestroyDescriptorSetLayout(m_LogicDevice, m_DescSetLayout, nullptr);
                vkDestroyPipeline(m_LogicDevice, m_Pipeline, nullptr);
                vkDestroyPipelineLayout(m_LogicDevice, m_PipelineLayout, nullptr);
            }
        };

        struct m_UIRenderer : Renderer
        {
            m_UIRenderer() {}
            ~m_UIRenderer() {}
        };

        struct m_CustomRenderer : Renderer
        {
        };
    }
}
