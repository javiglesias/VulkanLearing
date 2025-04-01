#ifndef _C_MATERIAL
#define _C_MATERIAL

#include <vector>
#include <string>
#include <glm/vec4.hpp>
#include <vulkan/vulkan_core.h>

#define MAX_TEXTURES 7

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
			std::vector< VkDescriptorSetLayoutBinding> ShaderBindings;
			void _addBind(uint8_t _nbind
						, VkDescriptorType _type, VkShaderStageFlags _stage
						, uint8_t _ndescriptors = 1);
			void _buildPipeline();
			void Cleanup(VkDevice _LogicDevice);
		};

		struct MaterialInstance
		{
			sMaterialPipeline pipeline;
			std::vector<VkDescriptorSet> materialSets;
			std::vector<VkDescriptorSet> compute_descriptors_sets;
			VkDescriptorPool descriptorPool = nullptr;
			VkDescriptorPool compute_descriptor_pool = nullptr;
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
			std::vector<VkWriteDescriptorSet> m_DescriptorsWrite;
			char _vertPath[64];
			char _fragPath[64];
			char m_Tags[128];
			Texture* textures[MAX_TEXTURES];
			Texture* shadow_texture;
			void PrepareMaterialToDraw(VKBackend* _backend);
			void CreateDescriptor(VkDevice _LogicDevice);
			void PrepareDescriptorWrite(int16_t _setDst, uint32_t _bind
							, VkDescriptorType _descType , VkBuffer _buffer, VkDeviceSize _range
							, uint32_t _arrayElement = 0, VkDeviceSize _offset = 0, uint32_t _count = 1);
			void PrepareDescriptorWrite(int16_t _setDst, uint32_t _bind, VkDescriptorType _descType
						, VkImageView _imageView, VkSampler _sampler
						, uint32_t _arrayElement = 0, uint32_t _count = 1);
			void UpdateDescriptorSet(VkDevice _LogicDevice
				, std::vector<VkBuffer> _UniformBuffers
				, std::vector<VkBuffer> _DynamicBuffers
				, std::vector<VkBuffer> _LightsBuffers
				, std::vector<VkBuffer> _ComputeBuffers);
			void CreateDescriptorPool( VkDevice _LogicDevice
				, std::vector<VkDescriptorPoolSize> _desc_pool_size
				, VkDescriptorPool _desc_pool
				, VkDescriptorSetLayout _desc_set_layout
				, VkDescriptorSet* _desc_sets);
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
