#ifndef _C_DEBUG_MATERIAL
#define _C_DEBUG_MATERIAL

#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>


namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;
		struct Texture;

		struct R_DbgMaterial
		{
			VkShaderModule m_VertShaderModule;
			VkShaderModule m_FragShaderModule;
			VkDescriptorPool m_DescriptorPool = nullptr;
			std::vector<VkDescriptorSetLayout> m_DescLayouts;
			std::vector<VkDescriptorSet> m_DescriptorSet;
			Texture* m_Texture;
			
		public:
			void Cleanup(VkDevice _LogicDevice);
			void CreateDescriptorPool(VkDevice _LogicDevice);
			void CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout);
			void UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, std::vector<VkBuffer> _DynamicBuffers);
		};
	}
}
#endif
