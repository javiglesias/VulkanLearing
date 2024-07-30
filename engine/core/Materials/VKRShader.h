#ifndef _C_SHADER
#define _C_SHADER
#ifdef _WINDOWS
#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#else
#include "../../../dependencies/glslang/SPIRV/GlslangToSpv.h"
// #include "../../../dependencies/glslang/glslang/Public/ResourceLimits.h"
// #include "../../../dependencies/glslang/glslang/Public/ShaderLang.h"
#endif
#include <fstream>
#include <vector>

#define GLSL_VERSION 460
namespace VKR
{
	namespace render
	{
		struct Shader
		{
		private: // variables
			std::string m_Filename;
			std::string m_RawSource;
			#ifdef _WINDOWS
			EShLanguage m_Stage;
			glslang::TShader* m_TShader;
			#endif
		public: // variables
			std::vector<uint32_t> m_SpirvSrc;
			std::vector<uint32_t> spirvCode;
		public: // functions
			Shader(const std::string& _filename, int _shaderStage);
			std::vector<uint32_t> LoadShader();
			bool GLSLCompile(bool _recompile = false);
		private: // Functions
			void ReadFile();
			void ToSPVShader();
		};
	}
}
#endif