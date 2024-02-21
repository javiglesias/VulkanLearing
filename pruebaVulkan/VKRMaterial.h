#pragma once
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>
extern std::string g_ConsoleMSG;

struct Texture 
{
public:
	std::string sPath;
	VkImage tImage= nullptr;
	VkDeviceMemory tImageMem = nullptr;
	VkImageView tImageView = nullptr;
	VkSampler m_Sampler= nullptr;
	Texture(std::string _path)
	{
		sPath= _path;
	}
	void CleanTextureData(VkDevice _LogicDevice)
	{
		vkDestroySampler(_LogicDevice, m_Sampler, nullptr);
        m_Sampler = nullptr;
		vkDestroyImageView(_LogicDevice, tImageView, nullptr);
        tImageView = nullptr;
		vkDestroyImage(_LogicDevice,tImage, nullptr);
        tImage = nullptr;
		vkFreeMemory(_LogicDevice, tImageMem, nullptr);
        tImageMem = nullptr;
	}
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
	Texture* m_TextureDiffuse;
	Texture* m_TextureSpecular;
	Texture* m_TextureAmbient;
public:
	void CreateDescriptorPool(VkDevice _LogicDevice)
	{
        if(m_DescriptorPool == nullptr)
        {
            std::array<VkDescriptorPoolSize, 5> descPoolSize {};
            // UBO
            descPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descPoolSize[0].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
            // Textura diffuse
            descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descPoolSize[1].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
            // Textura Specular
            descPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descPoolSize[2].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
            // Textura Ambient
            descPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descPoolSize[3].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

			 // Model Matrix
            descPoolSize[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            descPoolSize[4].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

            VkDescriptorPoolCreateInfo descPoolInfo {};
            descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descPoolInfo.poolSizeCount = static_cast<uint32_t>(descPoolSize.size());
            descPoolInfo.pPoolSizes = descPoolSize.data();
            descPoolInfo.maxSets = FRAMES_IN_FLIGHT;
            if(vkCreateDescriptorPool(_LogicDevice, &descPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
                exit(-66);
        }
	}
	void CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout)
	{	
        if(m_DescriptorSet.size() <= 0)
        {
            m_DescLayouts.resize(FRAMES_IN_FLIGHT);
            m_DescLayouts[0]= _DescSetLayout;
            m_DescLayouts[1]= _DescSetLayout;
            VkDescriptorSetAllocateInfo descAllocInfo {};
            descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descAllocInfo.descriptorPool = m_DescriptorPool;
            descAllocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT); // SwapchainImages.size()
            descAllocInfo.pSetLayouts = m_DescLayouts.data();
            m_DescriptorSet.resize(FRAMES_IN_FLIGHT);
            if(vkAllocateDescriptorSets(_LogicDevice, &descAllocInfo, m_DescriptorSet.data()) != VK_SUCCESS)
                exit(-67);
        }
	}

	void UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, std::vector<VkBuffer> _DynamicBuffers)
	{
		// Escribimos la info de los descriptors
		for(size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo {};
			bufferInfo.buffer = _UniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject); // VK_WHOLE
			
			std::array<VkWriteDescriptorSet, 5> descriptorsWrite {};
			descriptorsWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrite[0].dstSet = m_DescriptorSet[i];
			descriptorsWrite[0].dstBinding = 0;
			descriptorsWrite[0].dstArrayElement = 0;
			descriptorsWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorsWrite[0].descriptorCount = 1;
			descriptorsWrite[0].pBufferInfo = &bufferInfo;
			descriptorsWrite[0].pImageInfo = nullptr;
			descriptorsWrite[0].pTexelBufferView = nullptr;
			// Texturas
			// Diffuse
			VkDescriptorImageInfo TextureDiffuseImage{};
			TextureDiffuseImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			TextureDiffuseImage.imageView = m_TextureDiffuse->tImageView;
			if(m_TextureDiffuse->m_Sampler != nullptr)
			{
				TextureDiffuseImage.sampler = m_TextureDiffuse->m_Sampler;
			}
			else
			{
				printf("Invalid Sampler for frame %ld\n", i);
				continue;
			}
			g_ConsoleMSG += m_TextureDiffuse->sPath;
			g_ConsoleMSG += '\n';
			descriptorsWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrite[1].dstSet = m_DescriptorSet[i];
			descriptorsWrite[1].dstBinding = 1;
			descriptorsWrite[1].dstArrayElement = 0;
			descriptorsWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorsWrite[1].descriptorCount = 1;
			descriptorsWrite[1].pBufferInfo = nullptr;
			descriptorsWrite[1].pImageInfo = &TextureDiffuseImage;
			descriptorsWrite[1].pTexelBufferView = nullptr;
			//Specular
			VkDescriptorImageInfo TextureSpecularImage{};
			TextureSpecularImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			TextureSpecularImage.imageView = m_TextureSpecular->tImageView;
			if(m_TextureSpecular->m_Sampler != nullptr)
			{
				TextureSpecularImage.sampler = m_TextureSpecular->m_Sampler;
			}
			else
			{
				printf("Invalid Sampler for frame %ld\n", i);
				continue;
			}
			g_ConsoleMSG += m_TextureSpecular->sPath;
			g_ConsoleMSG += '\n';
			descriptorsWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrite[2].dstSet = m_DescriptorSet[i];
			descriptorsWrite[2].dstBinding = 2;
			descriptorsWrite[2].dstArrayElement = 0;
			descriptorsWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorsWrite[2].descriptorCount = 1;
			descriptorsWrite[2].pBufferInfo = nullptr;
			descriptorsWrite[2].pImageInfo = &TextureSpecularImage;
			descriptorsWrite[2].pTexelBufferView = nullptr;
			//Ambient
			VkDescriptorImageInfo TextureAmbientImage{};
			TextureAmbientImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			TextureAmbientImage.imageView = m_TextureAmbient->tImageView;
			if(m_TextureAmbient->m_Sampler != nullptr)
			{
				TextureAmbientImage.sampler = m_TextureAmbient->m_Sampler;
			}
			else
			{
				printf("Invalid Sampler for frame %ld\n", i);
				continue;
			}
			g_ConsoleMSG += m_TextureAmbient->sPath;
			g_ConsoleMSG += '\n';
			descriptorsWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrite[3].dstSet = m_DescriptorSet[i];
			descriptorsWrite[3].dstBinding = 3;
			descriptorsWrite[3].dstArrayElement = 0;
			descriptorsWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorsWrite[3].descriptorCount = 1;
			descriptorsWrite[3].pBufferInfo = nullptr;
			descriptorsWrite[3].pImageInfo = &TextureAmbientImage;
			descriptorsWrite[3].pTexelBufferView = nullptr;
		// Dynamic
			VkDescriptorBufferInfo dynBufferInfo {};
			dynBufferInfo.buffer = _DynamicBuffers[i];
			dynBufferInfo.offset = 0;
			dynBufferInfo.range = sizeof(DynamicBufferObject); // VK_WHOLE
			
			descriptorsWrite[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrite[4].dstSet = m_DescriptorSet[i];
			descriptorsWrite[4].dstBinding = 4;
			descriptorsWrite[4].dstArrayElement = 0;
			descriptorsWrite[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorsWrite[4].descriptorCount = 1;
			descriptorsWrite[4].pBufferInfo = &dynBufferInfo;
			descriptorsWrite[4].pImageInfo = nullptr;
			descriptorsWrite[4].pTexelBufferView = nullptr;
			vkUpdateDescriptorSets(_LogicDevice, descriptorsWrite.size(), descriptorsWrite.data(), 0, nullptr);
		}
	}
	void Cleanup(VkDevice _LogicDevice)
	{
		// Delete Material things
		vkDestroyDescriptorPool(_LogicDevice, m_DescriptorPool, nullptr);
		m_TextureDiffuse->CleanTextureData(_LogicDevice);
        m_TextureDiffuse = nullptr;
		m_TextureSpecular->CleanTextureData(_LogicDevice);
        m_TextureSpecular = nullptr;
		m_TextureAmbient->CleanTextureData(_LogicDevice);
        m_TextureAmbient = nullptr;
	}
};
