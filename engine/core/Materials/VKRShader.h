#pragma once
#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/SPIRV/GlslangToSpv.h"
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
			EShLanguage m_Stage;
			glslang::TShader* m_TShader;
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
