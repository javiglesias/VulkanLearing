#ifndef _C_CUBEMAP_MATERIAL
#define _C_CUBEMAP_MATERIAL


#include <string>
#include <vector>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>
namespace VKR
{
	namespace render
	{
		class VKBackend;
		struct Texture;

		struct R_CubemapMaterial
		{
		private:
			char m_Path[64];
		public: // Variables
			VkShaderModule m_VertShaderModule;
			VkShaderModule m_FragShaderModule;
			VkDescriptorPool m_DescriptorPool = nullptr;
			std::vector<VkDescriptorSetLayout> m_DescLayouts;
			std::vector<VkDescriptorSet> m_DescriptorSet;
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
			Texture* m_Texture;

		public: // Functions
			R_CubemapMaterial(const char* _texture);
			void PrepareMaterialToDraw(VKBackend* _backend);
			void CreateDescriptorPool(VkDevice _LogicDevice);
			void CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout);

			void UpdateDescriptorSet(VkDevice _LogicDevice, std::array<VkBuffer, 2> _UniformBuffers);
			void Cleanup(VkDevice _LogicDevice);
		};
	}
}
#endif