#include <vulkan/vulkan_core.h>
struct Texture 
{
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
std::unordered_map<unsigned int, Texture> m_ModelTextures;

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
	R_Mesh(){}
public:
	std::vector<Vertex3D> m_Vertices;
	std::vector<uint16_t> m_Indices;
	Texture m_Texture;
	R_Material m_Material;
	VkDescriptorPool m_DescriptorPool;
	VkBuffer m_VertexBuffer;
	VkBuffer m_IndexBuffer;
};
struct R_Model //Render Model
{// lo necesario para poder renderizar un Modelo
public:
	R_Model() {}
public:
	std::vector<R_Mesh*> m_Meshes;
};