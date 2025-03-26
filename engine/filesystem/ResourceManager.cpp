#include "ResourceManager.h"
#include "gltfReader.h"


namespace VKR
{
	namespace RM
	{
		void _AddRequest(TYPE _type, const char* _filepath, const char* _resourceName, render::R_Model* model_)
		{
			if(model_ != nullptr)
				switch (_type)
				{
				case STATIC_MODEL:
				{
					filesystem::LoadModel_ALT(_filepath, _resourceName, model_);
					//render::m_SceneDirty = true;
					_NumRequests--;
					break;
				}
				case ASSIMP_MODEL:
				{
					filesystem::LoadModel(_filepath, _resourceName, model_);
					//render::m_SceneDirty = true;
					_NumRequests--;
					break;
				}
				default:
					break;
				}
			else
			{
				_RMRequests[_NumRequests].type = _type;
				strcpy(_RMRequests[_NumRequests].filepath, _filepath);
				strcpy(_RMRequests[_NumRequests].resourceName, _resourceName);
				_NumRequests++;
			}
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
					switch(_RMRequests[_NumRequests-1].type)
					{
						case STATIC_MODEL:
						{
							filesystem::LoadModel_ALT(_RMRequests[_NumRequests-1].filepath, _RMRequests[_NumRequests-1].resourceName, static_cast<render::R_Model*>(_RMRequests[_NumRequests - 1].modelPtr));
							//render::m_SceneDirty = true;
							_NumRequests--;
							break;
						}
						case ASSIMP_MODEL:
						{
							filesystem::LoadModel(_RMRequests[_NumRequests-1].filepath, _RMRequests[_NumRequests-1].resourceName, static_cast<render::R_Model*>(_RMRequests[_NumRequests-1].modelPtr));
							//render::m_SceneDirty = true;
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
