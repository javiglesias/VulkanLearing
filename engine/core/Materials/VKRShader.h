#ifndef _C_SHADER
#define _C_SHADER

#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#ifndef WIN32
 #include "../../../dependencies/glslang/SPIRV/GlslangToSpv.h"
 #include "../../../dependencies/glslang/glslang/Public/ResourceLimits.h"
 #include "../../../dependencies/glslang/glslang/Public/ShaderLang.h"
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
			bool GLSLCompile(bool _recompile = false);
		private: // Functions
			void ToSPVShader();
			void ReadFile();
		};
	}
}
#endif