#ifndef _C_SHADER
#define _C_SHADER

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>


namespace VKR
{
	namespace render
	{
		struct Shader
		{
			int m_Stage;
			std::string m_Filename;
			std::vector<uint32_t> m_SpirvSrc;
			Shader(const std::string& _filename, int _shaderStage);
			void LoadShader();
			void ConfigureShader(VkDevice m_LogicDevice,VkShaderStageFlagBits _type, VkPipelineShaderStageCreateInfo* shaderStageInfo_);
		};
		struct ShaderItem
		{
			uint8_t id;
			Shader* shader;
		};
		static ShaderItem* ShaderList[256];
		static int currentShaders = 0;
		static void clean_shader_list()
		{
			currentShaders = 0;
		}
		static void add_shader_to_list(Shader* _shader)
		{
			ShaderItem* nouvo = new ShaderItem();
			nouvo->id = (uint8_t)currentShaders;
			nouvo->shader = _shader;
			ShaderList[currentShaders] = nouvo;
			++VKR::render::currentShaders;
		}
		static uint8_t ShaderHashId(VKR::render::Shader* _shader)
		{
			
			return 0;
		}
		static Shader* find_shader(const char* _filename)
		{
			for (size_t i = 0; i < currentShaders; i++)
			{
				char filename[128];
				sprintf(filename, "%s.spv", _filename);
				if (strcmp(ShaderList[i]->shader->m_Filename.c_str(), filename) == 0)
					return ShaderList[i]->shader;
			}
			return nullptr;
		}
	}
}
#endif
