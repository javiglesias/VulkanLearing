#pragma once
#include "../memory/mem_alloc.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include <fstream>
#include <vector>

namespace VKR
{
	namespace render
	{
	#define GLSL_VERSION 460
		inline
		std::vector<uint32_t> CompileToSPVShader(const std::string _filename, int _shaderStage)
		{
			std::ifstream f(_filename, std::ios::ate | std::ios::binary);
			size_t fileSize = (size_t)f.tellg();
			std::vector<char> buffer(fileSize);
			f.seekg(0);
			f.read(buffer.data(), fileSize);
			f.close();
			std::string _source = std::string(buffer.begin(), buffer.end());
			const EShLanguage vStage = (EShLanguage)_shaderStage;
			glslang::TShader vShader(vStage);
			auto str = _source.c_str();
			vShader.setStrings(&str, 1);
			vShader.setPreamble("#extension GL_GOOGLE_include_directive : require\n");
			// esto activa tambien el #extension GL_GOOGLE_cpp_style_line_directive : enable
			vShader.setEnvInput(glslang::EShSourceGlsl, (EShLanguage)_shaderStage, glslang::EShClientVulkan, GLSL_VERSION);
			vShader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
			vShader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);
			vShader.parse(
				GetDefaultResources(),  // default TBuiltInResource from ResourceLimits.h
				GLSL_VERSION,                    // default version
				false,                  // not forward compatible
				EShMsgDefault           // report default error/warning messages
			);
			printf("Parsing shader: %s\n", vShader.getInfoLog());
			glslang::TProgram program;
			program.addShader(&vShader);
			program.link(EShMsgDefault);    // link and report default error/warning messages
			printf("Linking program: %s\n", program.getInfoLog()); // get the log for linking the program
			glslang::TIntermediate* intermediate = program.getIntermediate((EShLanguage)_shaderStage);
			std::vector<uint32_t> spirv;                    // the vector for the output
			glslang::GlslangToSpv(*intermediate, spirv);    // convert the glslang intermediate into SPIR-V bytes
			printf("SPIRV creation: %s\n", vShader.getInfoLog());
			return spirv; // Usually the result is optimized with RVO so don't worry about copying
		}

		inline
		bool CheckSPVCompile(const std::string _filename, int _shaderStage)
		{
			std::ifstream f(_filename, std::ios::ate | std::ios::binary);
			size_t fileSize = (size_t)f.tellg();
			std::vector<char> buffer(fileSize);
			f.seekg(0);
			f.read(buffer.data(), fileSize);
			f.close();
			std::string _source = std::string(buffer.begin(), buffer.end());
			const EShLanguage vStage = (EShLanguage)_shaderStage;
			glslang::TShader vShader(vStage);
			auto str = _source.c_str();
			vShader.setStrings(&str, 1);
			vShader.setPreamble("#extension GL_GOOGLE_include_directive : require\n");
			// esto activa tambien el #extension GL_GOOGLE_cpp_style_line_directive : enable
			vShader.setEnvInput(glslang::EShSourceGlsl, (EShLanguage)_shaderStage, glslang::EShClientVulkan, GLSL_VERSION);
			vShader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
			vShader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);
			vShader.parse(
				GetDefaultResources(),  // default TBuiltInResource from ResourceLimits.h
				GLSL_VERSION,                    // default version
				false,                  // not forward compatible
				EShMsgDefault           // report default error/warning messages
			);
			printf("Parsing shader: %s\n", vShader.getInfoLog());
			glslang::TProgram program;
			program.addShader(&vShader);
			program.link(EShMsgDefault);    // link and report default error/warning messages
			printf("Linking program: %s\n", program.getInfoLog()); // get the log for linking the program
			glslang::TIntermediate* intermediate = program.getIntermediate((EShLanguage)_shaderStage);
			if(intermediate->getNumErrors() <= 0)
			{
				std::vector<uint32_t> spirv;                    // the vector for the output
				glslang::SpvOptions options;
				options.validate = true;
				glslang::GlslangToSpv(*intermediate, spirv, &options);    // convert the glslang intermediate into SPIR-V bytes
				printf("SPIRV creation: %s\n", vShader.getInfoLog());
				return (spirv.size() > 0); // Usually the result is optimized with RVO so don't worry about copying
			}
			return false;
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
			return buffer;
		}
	}
}
