struct Texture 
{
	std::string sPath;
	VkImage tImage;
	VkDeviceMemory tImageMem;
	VkImageView tImageView;
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
	std::vector<unsigned int> m_Indices;
	Texture m_Texture;
	R_Material m_Material;
};
struct R_Model //Render Model
{// lo necesario para poder renderizar un Modelo
public:
	R_Model() {}
public:
	std::vector<R_Mesh*> m_Meshes;
};

std::vector<R_Model*> m_StaticModels;
R_Model* tempModel;