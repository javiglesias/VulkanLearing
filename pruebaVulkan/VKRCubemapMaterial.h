#pragma once
#pragma once
#include "VKRTexture.h"

#include <string>
#include <vector>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

namespace VKR
{
	namespace render
	{
		struct R_CubemapMaterial
		{
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
			R_CubemapMaterial(std::string _path);
			void CreateDescriptorPool(VkDevice _LogicDevice);
			void CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout);

			void UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, std::vector<VkBuffer> _DynamicBuffers);
			void Cleanup(VkDevice _LogicDevice);
		};
	}
}
