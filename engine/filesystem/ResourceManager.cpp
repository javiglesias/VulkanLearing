#include "ResourceManager.h"
#include "gltfReader.h"

namespace VKR
{
	namespace RM
	{
		RMRequest _RMRequests[256];
		int _NumRequests = 0;
		void _AddRequest(REQ_TYPE _type, RES_TYPE _res_type, const char* _filepath, const char* _resourceName, render::R_Model* model_)
		{
			if(model_ != nullptr)
				switch (_res_type)
				{
				case STATIC_MODEL:
				{
					filesystem::LoadModel_ALT(_filepath, _resourceName, model_);
					//render::m_SceneDirty = true;
					_NumRequests = 0;
					break;
				}
				case ASSIMP_MODEL:
				{
					filesystem::LoadModel(_filepath, _resourceName, model_);
					//render::m_SceneDirty = true;
					_NumRequests = 0;
					break;
				}
				case MAP_FILE:
				{
					// read map file from filesystem
					_NumRequests++;
					break;
				}
				default:
					break;
				}
			else
			{
				_RMRequests[_NumRequests].type = _type;
				_RMRequests[_NumRequests].res_type = _res_type;
				if (_filepath && _resourceName)
				{
					strcpy(_RMRequests[_NumRequests].filepath, _filepath);
					strcpy(_RMRequests[_NumRequests].resourceName, _resourceName);
				}
				_RMRequests[_NumRequests].modelPtr = new render::R_Model();
				_NumRequests++;
			}
		}

		void _Init()
		{
			_Loop();
		}

		void _Loop()
		{
			if (_NumRequests > 0)
				switch(_RMRequests[_NumRequests-1].type)
				{
					case LOAD:
					{
						render::R_Model* model;
						filesystem::LoadModel(_RMRequests[_NumRequests-1].filepath, _RMRequests[_NumRequests-1].resourceName, static_cast<render::R_Model*>(_RMRequests[_NumRequests-1].modelPtr));
						_NumRequests--;
						break;
						filesystem::LoadModel_ALT(_RMRequests[_NumRequests-1].filepath, _RMRequests[_NumRequests-1].resourceName, static_cast<render::R_Model*>(_RMRequests[_NumRequests - 1].modelPtr));
						break;
					}
					case SAVE:
					{
						_SaveResources();
						_NumRequests--;
					}
						break;
					default:
						break;
				}
		}

		void _Destroy()
		{
		}
		void _SaveResources()
		{
			// TODO Savedata methods for savable resources
			switch (_RMRequests[_NumRequests - 1].res_type)
			{
				case MAP_FILE:
				{
					char map_name[256];
					sprintf(map_name, "%s/mapfile.rm", MAPS_PATHS);
					FILE* map = fopen(map_name, "w+");
					fprintf(map, "mapa\n\tRecurso2");
					fclose(map);
				}
				break;
			}
		}
		RMRequest::RMRequest(const char* _PathFromLoad)
		{
			memcpy(filepath, _PathFromLoad, 256);
		}
	}
}
