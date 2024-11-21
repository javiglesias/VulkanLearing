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
		extern std::string g_ConsoleMSG;
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
		struct R_Material
		{
			VkShaderModule m_VertShaderModule;
			VkShaderModule m_FragShaderModule;
			VkDescriptorPool m_DescriptorPool = nullptr;
			std::vector<VkDescriptorSetLayout> m_DescLayouts;
			std::vector<VkDescriptorSet> m_DescriptorSet;
		public:
			enum STATE : uint16_t
			{
				UNDEFINED,
				CREATED,
				INICIALITEZ,
				READY,
				DESTROYED
			};
			STATE m_Status = UNDEFINED;
			char _vertPath[64];
			char _fragPath[64];
			Texture* m_TextureBaseColor;
			Texture* m_TextureDiffuse;
			Texture* m_TextureSpecular;
			Texture* m_TextureAmbient;
			Texture* m_TextureOcclusion;
			Texture* m_TextureNormal;
			Texture* m_TextureShadowMap;
			Texture* m_TextureEmissive;
		public:
			void PrepareMaterialToDraw(VKBackend* _backend);
			void CreateDescriptorPool(VkDevice _LogicDevice);
			void CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout);
			void UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, 
									 std::vector<VkBuffer> _DynamicBuffers, std::vector<VkBuffer> _LightsBuffers);
			void Cleanup(VkDevice _LogicDevice);
		};
	}
}
#endif