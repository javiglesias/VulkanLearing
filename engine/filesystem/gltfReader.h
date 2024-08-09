#define CGLTF_IMPLEMENTATION
struct cgltf_data;
namespace VKR
{
	namespace render
	{
		class R_Model;
	}
	namespace filesystem
	{
		cgltf_data* read_glTF(const char* _filepath, const char* _modelName, render::R_Model* tempModel_);
	}
}