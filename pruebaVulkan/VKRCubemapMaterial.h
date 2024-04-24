#pragma once
#include "VKBackend.h"
#include <string>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

namespace VKR
{
	namespace render
	{
		class Texture;
		struct R_CubemapMaterial
		{
		private:
			std::string m_Path;
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
			void PrepareMaterialToDraw(VKBackend* _backend);
			void CreateDescriptorPool(VkDevice _LogicDevice);
			void CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout);

			void UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, std::vector<VkBuffer> _DynamicBuffers);
			void Cleanup(VkDevice _LogicDevice);
		};
	}
}
