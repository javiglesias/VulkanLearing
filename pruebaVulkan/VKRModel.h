#include <vulkan/vulkan_core.h>
extern std::string g_ConsoleMSG;

struct Texture 
{
public:
	std::string sPath;
	VkImage tImage;
	VkDeviceMemory tImageMem;
	VkImageView tImageView;
	VkSampler m_Sampler;
	Texture()
	{
		sPath[0] = 0;
	}
	Texture(std::string _path)
	{
		sPath= _path;
	}
};
struct R_Material
{
	VkShaderModule m_VertShaderModule;
	VkShaderModule m_FragShaderModule;
	char _vertPath[64];
	char _fragPath[64];
	public:
		R_Material() {}
		R_Material(const char* _vertPath, const char* _fragPath);
	private:
		void CreateShaderModule(const char* _shaderPath);
};

struct R_Mesh
{// lo necesario para poder renderizar una Malla
public:
	std::vector<Vertex3D> m_Vertices;
	std::vector<uint16_t> m_Indices;
	Texture m_Texture;
	R_Material m_Material;
	VkDescriptorPool m_DescriptorPool;
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;
	std::vector<VkDescriptorSetLayout> m_DescLayouts;
	std::vector<VkDescriptorSet> m_DescriptorSet;
public:
	R_Mesh(){}
	void CreateDescriptorPool(VkDevice _LogicDevice)
	{
		std::array<VkDescriptorPoolSize, 3> descPoolSize {};
		// UBO
		descPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descPoolSize[0].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
		// Textura diffuse
		descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descPoolSize[1].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
		// Textura specular
		descPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descPoolSize[2].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo descPoolInfo {};
		descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descPoolInfo.poolSizeCount = static_cast<uint32_t>(descPoolSize.size());
		descPoolInfo.pPoolSizes = descPoolSize.data();
		descPoolInfo.maxSets = FRAMES_IN_FLIGHT;
		if(vkCreateDescriptorPool(_LogicDevice, &descPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
			exit(-66);
	}
	void CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout)
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

	void UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers)
	{
		// Escribimos la info de los descriptors
		for(size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo {};
			bufferInfo.buffer = _UniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject); // VK_WHOLE
			
			std::array<VkWriteDescriptorSet, 3> descriptorsWrite {};
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
			VkDescriptorImageInfo textureimage{};
			textureimage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			textureimage.imageView = m_Texture.tImageView;
			if(m_Texture.m_Sampler != nullptr)
			{
				textureimage.sampler = m_Texture.m_Sampler;
			}
			else
			{
				printf("Invalid Sampler for frame %ld\n", i);
				continue;
			}
			g_ConsoleMSG += m_Texture.sPath;
			g_ConsoleMSG += '\n';
			// Diffuse
			descriptorsWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrite[1].dstSet = m_DescriptorSet[i];
			descriptorsWrite[1].dstBinding = 1;
			descriptorsWrite[1].dstArrayElement = 0;
			descriptorsWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorsWrite[1].descriptorCount = 1;
			descriptorsWrite[1].pBufferInfo = nullptr;
			descriptorsWrite[1].pImageInfo = &textureimage;
			descriptorsWrite[1].pTexelBufferView = nullptr;
			//Specular
			descriptorsWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrite[2].dstSet = m_DescriptorSet[i];
			descriptorsWrite[2].dstBinding = 2;
			descriptorsWrite[2].dstArrayElement = 0;
			descriptorsWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorsWrite[2].descriptorCount = 1;
			descriptorsWrite[2].pBufferInfo = nullptr;
			descriptorsWrite[2].pImageInfo = &textureimage;
			descriptorsWrite[2].pTexelBufferView = nullptr;
			vkUpdateDescriptorSets(_LogicDevice, descriptorsWrite.size(), descriptorsWrite.data(), 0, nullptr);
		}
	}
	void CleanMeshData(VkDevice _LogicDevice)
	{
		if(m_Indices.size() > 0)
		{
			vkDestroyBuffer(_LogicDevice, m_IndexBuffer, nullptr);
			vkFreeMemory(_LogicDevice,m_IndexBufferMemory, nullptr);
		}
		vkDestroyBuffer(_LogicDevice, m_VertexBuffer, nullptr);
		vkFreeMemory(_LogicDevice, m_VertexBufferMemory, nullptr);
		// Delete texture things
		vkDestroySampler(_LogicDevice, m_Texture.m_Sampler, nullptr);
		vkDestroyImageView(_LogicDevice, m_Texture.tImageView, nullptr);
		vkDestroyImage(_LogicDevice,m_Texture.tImage, nullptr);
		vkFreeMemory(_LogicDevice, m_Texture.tImageMem, nullptr);
		vkDestroyDescriptorPool(_LogicDevice, m_DescriptorPool, nullptr);
	}
};
struct R_Model //Render Model
{// lo necesario para poder renderizar un Modelo
public:
	R_Model() {}
public:
	std::vector<R_Mesh*> m_Meshes;
};