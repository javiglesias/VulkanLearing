#include "VKRShader.h"
#include <csignal>

namespace VKR
{
	namespace render
	{
		Shader::Shader(const std::string& _filename, int _shaderStage)
		{
			m_Filename = _filename;
#ifdef _WIN32
			m_Stage = (EShLanguage)_shaderStage;
#else
			m_Filename += ".spv";
#endif
		}

		void Shader::ReadFile()
		{
#ifdef _WIN32
			m_RawSource.clear();
			std::ifstream f(m_Filename, std::ios::ate | std::ios::binary);
			size_t fileSize = (size_t)f.tellg();
			std::vector<char> buffer(fileSize);
			f.seekg(0);
			f.read(buffer.data(), fileSize);
			f.close();
			m_RawSource = std::string(buffer.begin(), buffer.end());
#else	
			m_CompiledSource.clear();
			std::ifstream f(m_Filename, std::ios::ate | std::ios::binary);
			if(f.is_open())
			{
				size_t fileSize = (size_t)f.tellg();
				m_CompiledSource.resize(fileSize);
				f.seekg(0);
				f.read(m_CompiledSource.data(), fileSize);
			} else {
				raise(SIGTRAP);

			}
			f.close();
#endif
		}
		#ifdef _WIN32
		std::vector<uint32_t>
		#else
		std::vector<char>
		#endif
		Shader::LoadShader()
		{
			ReadFile();
			#ifdef _WIN32
			if(GLSLCompile())
				ToSPVShader();
			return m_SpirvSrc;
			#else
			return m_CompiledSource;
			#endif
		}

		void Shader::ToSPVShader()
		{
			#ifdef _WIN32
				glslang::TProgram m_Program;
				m_SpirvSrc.clear();
				m_Program.addShader(m_TShader);
				m_Program.link(EShMsgDefault);    // link and report default error/warning messages
				printf("Linking program %s: %s\n", m_Filename.c_str(), m_Program.getInfoLog()); // get the log for linking the program
				glslang::TIntermediate* m_IntermediateSrc;
				m_IntermediateSrc = m_Program.getIntermediate(m_Stage);
				glslang::GlslangToSpv(*m_IntermediateSrc, m_SpirvSrc);    // convert the glslang intermediate into SPIR-V bytes
				printf("SPIRV creation %s: %s\n", m_Filename.c_str(), m_TShader->getInfoLog());
			#endif
		}

		bool Shader::GLSLCompile(bool _recompile)
		{
			if(_recompile) ReadFile();
#ifdef _WIN32
			auto str = m_RawSource.c_str();
			m_TShader = nullptr;
			m_TShader = new glslang::TShader(m_Stage);
			m_TShader->setStrings(&str, 1);
			m_TShader->setPreamble("#extension GL_GOOGLE_include_directive : require\n");
			m_TShader->setEnvInput(glslang::EShSourceGlsl, m_Stage, glslang::EShClientVulkan, GLSL_VERSION);
			m_TShader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
			m_TShader->setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);
			m_TShader->parse(
				GetDefaultResources(),  // default TBuiltInResource from ResourceLimits.h
				GLSL_VERSION,                    // default version
				false,                  // not forward compatible
				EShMsgDefault           // report default error/warning messages
			);
			char parseResult[256];
			strcpy(parseResult, m_TShader->getInfoLog());
			printf("Parse shader: %s\n", parseResult);
			return (strcmp(parseResult, "") == 0);
#else // we have to read the file compiled
			
#endif
			return true;
		}
	}
}
