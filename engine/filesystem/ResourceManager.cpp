#include "ResourceManager.h"
#include "gltfReader.h"
#include "../core/Objects/VKRModel.h"
#include "../video/VKBackend.h"

namespace VKR
{
	namespace RM
	{
		render::R_Model* tempModel;
		bool LoadModel_ALT(const char* _filepath, const char* _modelName)
		{
			char filename[128];
			tempModel = new render::R_Model();
			sprintf(filename, "%s%s", _filepath, _modelName);
			sprintf(tempModel->m_Path , "%s",  _modelName);
			auto data = filesystem::read_glTF(_filepath, _modelName, tempModel);
			if(data == nullptr) return false;
			render::m_StaticModels[render::m_CurrentStaticModels] = tempModel;
			render::m_CurrentStaticModels++;
			return true;
		}

		void _AddRequest(TYPE _type, const char* _filepath, const char* _resourceName)
		{
			_RMRequests[_NumRequests].type = _type;
			_RMRequests[_NumRequests].filepath = _filepath;
			_RMRequests[_NumRequests].resourceName = _resourceName;
			_NumRequests++;
		}

		void _Init()
		{
			_Loop();
		}

		void _Loop()
		{
			while(true)
			{
				if(_NumRequests > 0)
					switch(_RMRequests[_NumRequests].type)
					{
						case STATIC_MODEL:
						{
							LoadModel_ALT(_RMRequests[_NumRequests].filepath, _RMRequests[_NumRequests].resourceName);
							render::m_SceneDirty = true;
							_NumRequests--;
							break;
						}
						default:
							break;
					}
			}
			_Destroy();
		}

		void _Destroy()
		{
		}
	}
}
