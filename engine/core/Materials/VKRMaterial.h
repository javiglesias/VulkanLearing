#ifndef _C_MATERIAL
#define _C_MATERIAL

#include <vector>
#include <string>
#include <glm/vec4.hpp>
#include <vulkan/vulkan_core.h>

namespace VKR
{
	namespace render
	{
		extern const int FRAMES_IN_FLIGHT;
		class VKBackend;
		struct Texture;

		struct PBR_material
		{
			char name[32];
			glm::vec4 base_color;
			float pbr_factors[2]; // metallic, roughness
			Texture* pbr_texture;
		};

		struct sMaterialPipeline
		{
			bool debugMaterial;

			VkPipeline pipeline;
			VkPipelineLayout layout;
			VkDescriptorSetLayout descriptorSetLayout;
			void _buildPipeline();
			void Cleanup(VkDevice _LogicDevice);
		};

		struct MaterialInstance
		{
			sMaterialPipeline pipeline;
			std::vector<VkDescriptorSet> materialSets;
			VkDescriptorPool descriptorPool = nullptr;
			void Cleanup(VkDevice _LogicDevice);
		};

		struct R_Material
		{
			VkShaderModule m_VertShaderModule;
			VkShaderModule m_FragShaderModule;
			MaterialInstance material;

			char _vertPath[64];
			char _fragPath[64];
			Texture* textures[8];

			void PrepareMaterialToDraw(VKBackend* _backend);
			void CreateDescriptor(VkDevice _LogicDevice);
			void UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, 
									 std::vector<VkBuffer> _DynamicBuffers, std::vector<VkBuffer> _LightsBuffers);
			void Cleanup(VkDevice _LogicDevice);
		};
	}
}
#endif
