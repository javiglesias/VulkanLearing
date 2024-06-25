#include "VKRenderers.h"

namespace VKR
{
    namespace render
    {
        void Renderer::CreateShaderStages()
        {
            /// Color Blending
            m_ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            m_ColorBlendAttachment.blendEnable = VK_FALSE;
            m_ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            m_ColorBlending.logicOpEnable = VK_FALSE;
            m_ColorBlending.attachmentCount = 1;
            m_ColorBlending.pAttachments = &m_ColorBlendAttachment;
            /// Creacion de los shader stage de la Pipeline
            m_VertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_VertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            m_VertShaderStageInfo.module = m_VertShaderModule;
            m_VertShaderStageInfo.pName = "main";
            m_FragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_FragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            m_FragShaderStageInfo.module = m_FragShaderModule;
            m_FragShaderStageInfo.pName = "main";
            m_ShaderStages[0] = m_VertShaderStageInfo;
            m_ShaderStages[1] = m_FragShaderStageInfo;
            /// Dynamic State (stados que permiten ser cambiados sin re-crear toda la pipeline)
            m_DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            m_DynamicState.dynamicStateCount = static_cast<uint32_t>(m_DynamicStates.size());
            m_DynamicState.pDynamicStates = m_DynamicStates.data();
        }

        void Renderer::CreatePipelineLayoutSetup(VkExtent2D* _CurrentExtent, VkViewport* _Viewport, VkRect2D* _Scissor)
        {
            /// Definimos la geometria que vamos a pintar
            m_InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            m_InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            m_InputAssembly.primitiveRestartEnable = VK_FALSE;
            /// Definimos el Viewport de la app
            _Viewport->x = 0.f;
            _Viewport->y = 0.f;
            _Viewport->width = _CurrentExtent->width;
            _Viewport->height = _CurrentExtent->height;
            _Viewport->minDepth = 0.0f;
            _Viewport->maxDepth = 1.0f;
            /// definamos el Scissor Rect de la app
            _Scissor->offset = { 0, 0 };
            _Scissor->extent.height = _CurrentExtent->height;
            _Scissor->extent.width = _CurrentExtent->width;
            m_ViewportState.viewportCount = 1;
            m_ViewportState.scissorCount = 1;
            m_ViewportState.pViewports = _Viewport;
            m_ViewportState.pScissors = _Scissor;
            /* Si no estuvieramos en modo Dynamico, necesitariamos establecer la
            * informacion de creacion del ViewportState y los Vewport punteros.
            */
            /// El rasterizador convierte los vertices en fragmentos para darles color
            m_Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            m_Rasterizer.depthClampEnable = VK_FALSE;
            m_Rasterizer.rasterizerDiscardEnable = VK_FALSE;
            m_Rasterizer.polygonMode = (VkPolygonMode)m_PolygonMode;
            m_Rasterizer.lineWidth = 1.f;
            m_Rasterizer.cullMode = m_CullMode;
            m_Rasterizer.frontFace = m_FrontFace;
            m_Rasterizer.depthBiasEnable = VK_FALSE;
            /// Multisampling para evitar los bordes de sierra (anti-aliasing).
            m_Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            m_Multisampling.sampleShadingEnable = VK_FALSE;
            m_Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            m_Multisampling.minSampleShading = 1.f;
            m_Multisampling.pSampleMask = nullptr;
            m_Multisampling.alphaToCoverageEnable = VK_FALSE;
            m_Multisampling.alphaToOneEnable = VK_FALSE;
            /// Depth and stencil
            m_DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            m_DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            m_DepthStencil.depthTestEnable = VK_TRUE;
            m_DepthStencil.depthWriteEnable = VK_TRUE;
            m_DepthStencil.depthBoundsTestEnable = VK_FALSE;
            m_DepthStencil.minDepthBounds = 0.0f;
            m_DepthStencil.maxDepthBounds = 1.0f;
            m_DepthStencil.stencilTestEnable = VK_FALSE;
            m_DepthStencil.front = {};
            m_DepthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
        }
        void Renderer::CreatePipelineLayout()
        {
            /// Pipeline Layout
            VkPipelineLayoutCreateInfo m_PipelineLayoutCreateInfo{};
            m_PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            m_PipelineLayoutCreateInfo.setLayoutCount = 1;
            m_PipelineLayoutCreateInfo.pSetLayouts = &m_DescSetLayout;
            m_PipelineLayoutCreateInfo.pushConstantRangeCount = 0;
            m_PipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
            if (vkCreatePipelineLayout(m_LogicDevice, &m_PipelineLayoutCreateInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
                exit(-7);
        }

        void Renderer::CreatePipeline(VkRenderPass _RenderPass)
        {
            // Creamos la Pipeline para pintar objs
            VkGraphicsPipelineCreateInfo m_PipelineInfoCreateInfo{};
            m_PipelineInfoCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            m_PipelineInfoCreateInfo.stageCount = 2;
            m_PipelineInfoCreateInfo.pStages = m_ShaderStages;
            m_PipelineInfoCreateInfo.pVertexInputState = &m_VertexInputInfo;
            m_PipelineInfoCreateInfo.pInputAssemblyState = &m_InputAssembly;
            m_PipelineInfoCreateInfo.pViewportState = &m_ViewportState;
            m_PipelineInfoCreateInfo.pRasterizationState = &m_Rasterizer;
            m_PipelineInfoCreateInfo.pMultisampleState = &m_Multisampling;
            m_PipelineInfoCreateInfo.pDepthStencilState = &m_DepthStencil;
            m_PipelineInfoCreateInfo.pColorBlendState = &m_ColorBlending;
            m_PipelineInfoCreateInfo.pDynamicState = &m_DynamicState;
            m_PipelineInfoCreateInfo.layout = m_PipelineLayout;
            m_PipelineInfoCreateInfo.renderPass = _RenderPass;
            m_PipelineInfoCreateInfo.subpass = 0;
            m_PipelineInfoCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            m_PipelineInfoCreateInfo.basePipelineIndex = -1;

            if (vkCreateGraphicsPipelines(m_LogicDevice, VK_NULL_HANDLE, 1, &m_PipelineInfoCreateInfo,
                nullptr, &m_Pipeline) != VK_SUCCESS)
                exit(-9);
        }
        void Renderer::CreateDescriptorSetLayout()
        {
            // estructura UBO
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            uboLayoutBinding.pImmutableSamplers = nullptr;

            // estructura Dynamic Uniforms
            VkDescriptorSetLayoutBinding dynOLayoutBinding{};
            dynOLayoutBinding.binding = 1;
            dynOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            dynOLayoutBinding.descriptorCount = 1;
            dynOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            dynOLayoutBinding.pImmutableSamplers = nullptr;

            std::array<VkDescriptorSetLayoutBinding, 2> ShaderBindings = {
                uboLayoutBinding,
                dynOLayoutBinding
            };
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(ShaderBindings.size());
            layoutInfo.pBindings = ShaderBindings.data();
            if (vkCreateDescriptorSetLayout(m_LogicDevice, &layoutInfo, nullptr, &m_DescSetLayout) != VK_SUCCESS)
                exit(-99);
        }
        bool Renderer::CreateShaderModule(const char* _shaderPath, VkShaderModule* _shaderModule, int _stage) // 0: vert, 4:frag
        {
            char shaderPath[64];
            strcpy(shaderPath, _shaderPath);
            auto spvCompiled = CompileToSPVShader(_shaderPath, _stage);
            VkShaderModuleCreateInfo shaderModuleCreateInfo{};
            shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderModuleCreateInfo.codeSize = 4 * spvCompiled.size();
            shaderModuleCreateInfo.pCode = static_cast<uint32_t*>(spvCompiled.data());
            if (vkCreateShaderModule(m_LogicDevice, &shaderModuleCreateInfo, nullptr, _shaderModule)
                != VK_SUCCESS)
                return false;
            return true;
        }
        void Renderer::Cleanup()
        {
            vkDestroyDescriptorSetLayout(m_LogicDevice, m_DescSetLayout, nullptr);
            vkDestroyPipeline(m_LogicDevice, m_Pipeline, nullptr);
            vkDestroyPipelineLayout(m_LogicDevice, m_PipelineLayout, nullptr);
        }
        void Renderer::CleanShaderModules()
        {
            // Destruymos los ShaderModule ahora que ya no se necesitan.
            vkDestroyShaderModule(m_LogicDevice, m_VertShaderModule, nullptr);
            vkDestroyShaderModule(m_LogicDevice, m_FragShaderModule, nullptr);
        }

        bool DebugRenderer::Initialize()
        {
            // DEBUG SHADERS
            //"engine/shaders/Standard.vert"
            if(CreateShaderModule("engine/shaders/Debug.vert", &m_VertShaderModule, 0) &&
                CreateShaderModule("engine/shaders/Debug.frag", &m_FragShaderModule, 4))
            {
                CreateShaderStages();
                /// Vertex Input (los datos que l epasamos al shader per-vertex o per-instance)
                m_VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                m_VertexInputInfo.vertexBindingDescriptionCount = 1;
                m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<unsigned int>(m_DbgAttributeDescriptions.size());
                m_VertexInputInfo.pVertexBindingDescriptions = &m_DbgBindingDescription;
                m_VertexInputInfo.pVertexAttributeDescriptions = m_DbgAttributeDescriptions.data();
                return true;
            }
            return false;
        }

        void DebugRenderer::CreateDescriptorSetLayout()
        {
            // estructura UBO
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            // Textura Diffuse
            VkDescriptorSetLayoutBinding textureDiffuseLayoutBinding{};
            textureDiffuseLayoutBinding.binding = 1;
            textureDiffuseLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textureDiffuseLayoutBinding.descriptorCount = 1;
            textureDiffuseLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            textureDiffuseLayoutBinding.pImmutableSamplers = nullptr;
            // estructura Dynamic Uniforms
            VkDescriptorSetLayoutBinding dynOLayoutBinding{};
            dynOLayoutBinding.binding = 2;
            dynOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            dynOLayoutBinding.descriptorCount = 1;
            dynOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            dynOLayoutBinding.pImmutableSamplers = nullptr;

            std::array<VkDescriptorSetLayoutBinding, 3> ShaderBindings = {
                uboLayoutBinding,
                textureDiffuseLayoutBinding,
                dynOLayoutBinding
            };
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(ShaderBindings.size());
            layoutInfo.pBindings = ShaderBindings.data();
            if (vkCreateDescriptorSetLayout(m_LogicDevice, &layoutInfo, nullptr, &m_DescSetLayout) != VK_SUCCESS)
                exit(-99);
        }

        // GRAPHIC PIPELINE

        bool GraphicsRenderer::Initialize()
        {
            /// Vamos a crear los shader module para cargar el bytecode de los shaders
            if(CreateShaderModules())
            {
                CreateShaderStages();
                /// Vertex Input (los datos que l epasamos al shader per-vertex o per-instance)
                m_VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                m_VertexInputInfo.vertexBindingDescriptionCount = 1;
                m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<unsigned int>(m_AttributeDescriptions.size());
                m_VertexInputInfo.pVertexBindingDescriptions = &m_BindingDescription;
                m_VertexInputInfo.pVertexAttributeDescriptions = m_AttributeDescriptions.data();
                return true;
            }
            return false;
        }

        bool GraphicsRenderer::CreateShaderModules()
        {
            return CreateShaderModule("engine/shaders/Standard.vert", &m_VertShaderModule, 0) && 
                CreateShaderModule("engine/shaders/Standard.frag", &m_FragShaderModule, 4);
        }

        void GraphicsRenderer::CreateDescriptorSetLayout()
        {
            // estructura UBO
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            // Textura Diffuse
            VkDescriptorSetLayoutBinding textureDiffuseLayoutBinding{};
            textureDiffuseLayoutBinding.binding = 1;
            textureDiffuseLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textureDiffuseLayoutBinding.descriptorCount = 1;
            textureDiffuseLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            textureDiffuseLayoutBinding.pImmutableSamplers = nullptr;

            // Textura specular
            VkDescriptorSetLayoutBinding textureSpecularLayoutBinding{};
            textureSpecularLayoutBinding.binding = 2;
            textureSpecularLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textureSpecularLayoutBinding.descriptorCount = 1;
            textureSpecularLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            textureSpecularLayoutBinding.pImmutableSamplers = nullptr;

            // Textura Ambient
            VkDescriptorSetLayoutBinding textureAmbientLayoutBinding{};
            textureAmbientLayoutBinding.binding = 3;
            textureAmbientLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textureAmbientLayoutBinding.descriptorCount = 1;
            textureAmbientLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            textureAmbientLayoutBinding.pImmutableSamplers = nullptr;

            // Textura Shadow
            VkDescriptorSetLayoutBinding textureShadowLayoutBinding{};
            textureShadowLayoutBinding.binding = 5;
            textureShadowLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textureShadowLayoutBinding.descriptorCount = 1;
            textureShadowLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            textureShadowLayoutBinding.pImmutableSamplers = nullptr;

            // estructura Dynamic Uniforms
            VkDescriptorSetLayoutBinding dynOLayoutBinding{};
            dynOLayoutBinding.binding = 4;
            dynOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            dynOLayoutBinding.descriptorCount = 1;
            dynOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            dynOLayoutBinding.pImmutableSamplers = nullptr;

            std::array<VkDescriptorSetLayoutBinding, 6> ShaderBindings = {
                uboLayoutBinding,
                textureDiffuseLayoutBinding,
                textureSpecularLayoutBinding,
                textureAmbientLayoutBinding,
                dynOLayoutBinding,
                textureShadowLayoutBinding
            };
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(ShaderBindings.size());
            layoutInfo.pBindings = ShaderBindings.data();
            if (vkCreateDescriptorSetLayout(m_LogicDevice, &layoutInfo, nullptr, &m_DescSetLayout) != VK_SUCCESS)
                exit(-99);
        }

        bool GraphicsRenderer::CheckCompileSPV(const char* _shaderPath, int _stage)
        {
            return CheckSPVCompile(_shaderPath, _stage);
        }

        bool ShadowRenderer::Initialize()
        {
            /// Vamos a crear los shader module para cargar el bytecode de los shaders
            if(CreateShaderModule("engine/shaders/Shadow.vert", &m_VertShaderModule, 0))
            {
                CreateShaderStages();
                /// Vertex Input (los datos que l epasamos al shader per-vertex o per-instance)
                m_VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                m_VertexInputInfo.vertexBindingDescriptionCount = 1;
                m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<unsigned int>(m_ShadowAttributeDescriptions.size());
                m_VertexInputInfo.pVertexBindingDescriptions = &m_ShadowBindingDescription;
                m_VertexInputInfo.pVertexAttributeDescriptions = m_ShadowAttributeDescriptions.data();
                return true;
            }
            return false;
        }

        void ShadowRenderer::CreateShaderStages()
        {
            /// Creacion de los shader stage de la Pipeline
            m_VertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_VertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            m_VertShaderStageInfo.module = m_VertShaderModule;
            m_VertShaderStageInfo.pName = "main";
            m_ShaderStages[0] = m_VertShaderStageInfo;
            /// Dynamic State (stados que permiten ser cambiados sin re-crear toda la pipeline)
            m_DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            m_DynamicState.dynamicStateCount = static_cast<uint32_t>(m_DynamicStates.size());
            m_DynamicState.pDynamicStates = m_DynamicStates.data();
        }

        void ShadowRenderer::CreatePipeline(VkRenderPass _RenderPass)
        {
            // Creamos la Pipeline para pintar objs
            VkGraphicsPipelineCreateInfo m_PipelineInfoCreateInfo{};
            m_PipelineInfoCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            m_PipelineInfoCreateInfo.stageCount = 1;
            m_PipelineInfoCreateInfo.pStages = m_ShaderStages;
            m_PipelineInfoCreateInfo.pVertexInputState = &m_VertexInputInfo;
            m_PipelineInfoCreateInfo.pInputAssemblyState = &m_InputAssembly;
            m_PipelineInfoCreateInfo.pViewportState = &m_ViewportState;
            m_PipelineInfoCreateInfo.pRasterizationState = &m_Rasterizer;
            m_PipelineInfoCreateInfo.pMultisampleState = &m_Multisampling;
            m_PipelineInfoCreateInfo.pDepthStencilState = &m_DepthStencil;
            m_PipelineInfoCreateInfo.pDynamicState = &m_DynamicState;
            m_PipelineInfoCreateInfo.layout = m_PipelineLayout;
            m_PipelineInfoCreateInfo.renderPass = _RenderPass;
            m_PipelineInfoCreateInfo.subpass = 0;
            m_PipelineInfoCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            m_PipelineInfoCreateInfo.basePipelineIndex = -1;

            if (vkCreateGraphicsPipelines(m_LogicDevice, VK_NULL_HANDLE, 1, &m_PipelineInfoCreateInfo,
                nullptr, &m_Pipeline) != VK_SUCCESS)
                exit(-9);
        }

        void ShadowRenderer::CleanShaderModules()
        {
            // Destruymos los ShaderModule ahora que ya no se necesitan.
            vkDestroyShaderModule(m_LogicDevice, m_VertShaderModule, nullptr);
        }

        bool CubemapRenderer::Initialize()
        {
            /// Vamos a crear los shader module para cargar el bytecode de los shaders
            if(CreateShaderModule("engine/shaders/Cubemap.vert", &m_VertShaderModule, 0) &&
            CreateShaderModule("engine/shaders/Cubemap.frag", &m_FragShaderModule, 4))
            {
                CreateShaderStages();
                /// Vertex Input (los datos que l epasamos al shader per-vertex o per-instance)
                m_VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                m_VertexInputInfo.vertexBindingDescriptionCount = 1;
                m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<unsigned int>(m_CubemapAttributeDescriptions.size());
                m_VertexInputInfo.pVertexBindingDescriptions = &m_CubemapBindingDescription;
                m_VertexInputInfo.pVertexAttributeDescriptions = m_CubemapAttributeDescriptions.data();
                return true;
            }
            return false;
        }

        bool CubemapRenderer::CreateShaderModules()
        {
            return CreateShaderModule("engine/shaders/Cubemap.vert", &m_VertShaderModule, 0) &&
                    CreateShaderModule("engine/shaders/Cubemap.frag", &m_FragShaderModule, 4);
        }

        void CubemapRenderer::CreateDescriptorSetLayout()
        {
            // estructura UBO
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            uboLayoutBinding.pImmutableSamplers = nullptr;

            // Textura Diffuse
            VkDescriptorSetLayoutBinding cubemapDiffuseLayoutBinding{};
            cubemapDiffuseLayoutBinding.binding = 1;
            cubemapDiffuseLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            cubemapDiffuseLayoutBinding.descriptorCount = 1;
            cubemapDiffuseLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            cubemapDiffuseLayoutBinding.pImmutableSamplers = nullptr;

            // estructura Dynamic Uniforms
            VkDescriptorSetLayoutBinding dynOLayoutBinding{};
            dynOLayoutBinding.binding = 2;
            dynOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            dynOLayoutBinding.descriptorCount = 1;
            dynOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            dynOLayoutBinding.pImmutableSamplers = nullptr;

            std::array<VkDescriptorSetLayoutBinding, 3> ShaderBindings = {
                uboLayoutBinding,
            	cubemapDiffuseLayoutBinding,
                dynOLayoutBinding
            };
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(ShaderBindings.size());
            layoutInfo.pBindings = ShaderBindings.data();
            if (vkCreateDescriptorSetLayout(m_LogicDevice, &layoutInfo, nullptr, &m_DescSetLayout) != VK_SUCCESS)
                exit(-99);
        }
    }
}
