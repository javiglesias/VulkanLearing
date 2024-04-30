#pragma once
#include "glslang/Public/ShaderLang.h"
#include <fstream>
#include <vector>

namespace VKR
{
	namespace render
	{
		inline
		std::vector<uint32_t> CompileToSPVShader(const char* const* _source)
		{
			const EShLanguage vStage = EShLangVertex;
			const EShLanguage fStage = EShLangFragment;
			glslang::TShader vShader(vStage);
			vShader.setStrings(_source, 1);
			return std::vector<uint32_t>(1);
		}
		
		inline 
		std::vector<char> LoadShader(const std::string& _filename)
		{
			std::ifstream f(_filename, std::ios::ate | std::ios::binary);
			size_t fileSize = (size_t)f.tellg();
			std::vector<char> buffer(fileSize);
			f.seekg(0);
			f.read(buffer.data(), fileSize);
			f.close();
			//CompileToSPVShader((const char* const*)buffer.data());
			return buffer;
		}
	}
}
