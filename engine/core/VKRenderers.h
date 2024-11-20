#ifndef _C_RENDERERERS
#define _C_RENDERERERS

#include "Materials/VKRShader.h"
#include "../video/Types.h"

namespace VKR
{
    namespace render
    {
        extern std::string g_ConsoleMSG;

		inline auto m_BindingDescription = Vertex3D::getBindingDescription();
		inline auto m_AttributeDescriptions = Vertex3D::getAttributeDescriptions();
		inline auto m_DbgBindingDescription = DBG_Vertex3D::getBindingDescription();
		inline auto m_DbgAttributeDescriptions = DBG_Vertex3D::getAttributeDescriptions();
		inline auto m_GridBindingDescription = Grid_Vertex3D::getBindingDescription();
		inline auto m_GridAttributeDescriptions = Grid_Vertex3D::getAttributeDescriptions();
        inline auto m_QuadBindingDescription = Vertex2D::getBindingDescription();
        inline auto m_QuadAttributeDescriptions = Vertex2D::getAttributeDescriptions();
		inline auto m_ShadowBindingDescription = m_BindingDescription;
		inline auto m_ShadowAttributeDescriptions = m_AttributeDescriptions;
        inline auto m_CubemapBindingDescription = m_DbgBindingDescription    ;
        inline auto m_CubemapAttributeDescriptions= m_DbgAttributeDescriptions ;

        struct Renderer
        {
            /// Data needed
            int m_PolygonMode = VK_POLYGON_MODE_FILL;
            VkCullModeFlagBits m_CullMode = VK_CULL_MODE_BACK_BIT;
            VkFrontFace m_FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
            Shader* m_VertShader = nullptr;
            Shader* m_FragShader = nullptr;
        public:
            virtual bool Initialize(bool _reload = false) {return true;}
            virtual void CreateDescriptorSetLayout();
            virtual void CreatePipelineLayout();
            virtual void CleanShaderModules();
			virtual bool CreateShaderStages(const char* _vertShader, const char* _fragShader = "", bool _force_recompile = false);

            void CreatePipelineLayoutSetup(VkExtent2D* _CurrentExtent, VkViewport* _Viewport, VkRect2D* _Scissor);
            void CreatePipeline(VkRenderPass _RenderPass);
            bool CreateShaderModule(Shader* _shader, VkShaderModule* _shaderModule);
            void Cleanup();
            Renderer() {}
        	~Renderer()
            {
                Cleanup();
            }
        };


        struct GraphicsRenderer : Renderer
        {
        public: // Functions

            // Creamos el layout de los Descriptor set que vamos a utlizar
            bool Initialize(bool _reload = false) override;
            bool CreateShaderModules();
            void CreateDescriptorSetLayout() override;
            GraphicsRenderer(VkDevice _LogicalDevice, int _PolygonMode = VK_POLYGON_MODE_FILL)
            {
                m_LogicDevice = _LogicalDevice;
                m_PolygonMode = _PolygonMode;
                m_CullMode = VK_CULL_MODE_BACK_BIT;
            }
            bool CheckCompileSPV(const char* _shaderPath, int _stage);
        };

        struct DebugRenderer : Renderer
        {
        public: // Functions
            bool Initialize(bool _reload = false) override;
            void CreateDescriptorSetLayout() override;

            DebugRenderer(VkDevice _LogicalDevice, int _PolygonMode = VK_POLYGON_MODE_FILL)
            {
                m_LogicDevice = _LogicalDevice;
                m_PolygonMode = VK_POLYGON_MODE_FILL;
                m_FrontFace = VK_FRONT_FACE_CLOCKWISE;
                m_CullMode = VK_CULL_MODE_NONE;
            }
        };

        struct ShadowRenderer : Renderer
        {
        public: // Functions
            // Creamos el layout de los Descriptor set que vamos a utlizar
            bool Initialize(bool _reload = false) override;
            bool CreateShaderStages(const char* _vertShader, const char* _fragShader = "", bool _force_recompile = false) override;
            void CreatePipeline(VkRenderPass _RenderPass);
            void CleanShaderModules() override;
            ShadowRenderer(VkDevice _LogicalDevice, int _PolygonMode = VK_POLYGON_MODE_FILL)
            {
                m_LogicDevice = _LogicalDevice;
                m_PolygonMode = _PolygonMode;
                m_CullMode = VK_CULL_MODE_NONE;
            }
        };

        struct CubemapRenderer : Renderer
        {
        public: // Functions
            // Creamos el layout de los Descriptor set que vamos a utlizar
            bool Initialize(bool _reload = false) override;
            bool CreateShaderModules();
            void CreateDescriptorSetLayout() override;
            CubemapRenderer(VkDevice _LogicalDevice, int _PolygonMode = VK_POLYGON_MODE_FILL)
            {
                m_LogicDevice = _LogicalDevice;
                m_PolygonMode = _PolygonMode;
                m_CullMode = VK_CULL_MODE_NONE;
            }
        };

        struct ShaderRenderer : Renderer
        {
        public: // Functions
            bool Initialize(bool _reload = false) override;
            void CreateDescriptorSetLayout() override;

            ShaderRenderer(VkDevice _LogicalDevice, int _PolygonMode = VK_POLYGON_MODE_FILL)
            {
                m_LogicDevice = _LogicalDevice;
            }
        };

        struct QuadRenderer : Renderer
        {
        public: // Functions
            bool Initialize(bool _reload = false) override;
            void CreateDescriptorSetLayout() override;

            QuadRenderer(VkDevice _LogicalDevice, int _PolygonMode = VK_POLYGON_MODE_FILL)
            {
                m_LogicDevice = _LogicalDevice;
            }
        };
    }
}
#endif