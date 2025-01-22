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
			VkPipeline compute;
			VkPipelineLayout layout;
			VkPipelineLayout compute_layout;
			VkDescriptorSetLayout descriptorSetLayout;
			VkDescriptorSetLayout compute_descriptorSetLayout;
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

		struct MaterialItem
		{
			uint8_t id;
			MaterialInstance* instance;
		};
		static MaterialItem* materialList[256];
		static int currentMaterials=0;
		struct R_Material
		{
			VkShaderModule m_VertShaderModule;
			VkShaderModule m_FragShaderModule;
			MaterialInstance* material;

			char _vertPath[64];
			char _fragPath[64];
			Texture* textures[8];

			void PrepareMaterialToDraw(VKBackend* _backend);
			void CreateDescriptor(VkDevice _LogicDevice);
			void UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, 
									 std::vector<VkBuffer> _DynamicBuffers, std::vector<VkBuffer> _LightsBuffers);
			void Cleanup(VkDevice _LogicDevice);
		};

		static void add_instance_to_list(MaterialInstance* _material)
		{
			MaterialItem* nouvo = new MaterialItem();
			nouvo->id = 0;
			nouvo->instance = _material;
			materialList[currentMaterials] = nouvo;
			++currentMaterials;
		}
		static uint8_t MaterialHashId(MaterialInstance _instance)
		{

			return 0;
		}
		static MaterialInstance* find_instance(uint8_t _id)
		{
			for (size_t i = 0; i < currentMaterials; i++)
			{
				return materialList[i]->instance;
			}
			return nullptr;
		}
		static void clean_material_list()
		{
			currentMaterials = 0;
		}
	}
}
#endif
