#ifndef _GLTF_READER_H
#define _GLTF_READER_H
#define CGLTF_IMPLEMENTATION
struct cgltf_data;
namespace VKR
{
	namespace render
	{
		class R_Model;
		class R_DbgModel;
	}
	namespace filesystem
	{
		cgltf_data* read_glTF(const char* _filepath, const char* _modelName, render::R_Model* tempModel_);
		cgltf_data* read_glTF_DBG(const char* _filepath, const char* _modelName, render::R_DbgModel* tempModel_);
	}
}
#endif