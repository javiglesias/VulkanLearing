#ifndef _C_SHADER
#define _C_SHADER

#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#ifndef WIN32
 #include "../../../dependencies/glslang/glslang/Public/ResourceLimits.h"
 #include "../../../dependencies/glslang/glslang/Public/ShaderLang.h"
#endif
#include <fstream>
#include <vector>
#include <vulkan/vulkan_core.h>

#define GLSL_VERSION 460

namespace VKR
{
	namespace render
	{
		struct Shader
		{
			std::string m_Filename;
#ifdef WIN32
			std::string m_RawSource;
			EShLanguage m_Stage;
			glslang::TShader* m_TShader;
		public: // variables
			std::vector<uint32_t> m_SpirvSrc;
			std::vector<uint32_t> spirvCode;
#else
		private:
			std::vector<char> m_CompiledSource;
		public:
			std::vector<char> m_CompiledSpv;
#endif
		public: // functions
			Shader(const std::string& _filename, int _shaderStage);
			#ifdef WIN32
			std::vector<uint32_t>
			#else
			std::vector<char>
			#endif 
			LoadShader();
			void ConfigureShader(VkDevice m_LogicDevice,VkShaderStageFlagBits _type, VkPipelineShaderStageCreateInfo* shaderStageInfo_);
			bool GLSLCompile(bool _recompile = false);
		private: // Functions
			void ToSPVShader();
			void ReadFile();
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
		static Shader* find_shader(char* _filename)
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
