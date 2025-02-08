#include <cstdlib>
#include <stdio.h>
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

std::vector<uint32_t> _read_shader(const char* _filename, int _stage)
{
	FILE* shader_file;
	time_t _time = time(NULL);
	char raw_code_file[128];
	char spv_code_file[128];
	struct stat file_stat_raw;
	struct stat file_stat_spv;
	int need_compilation = 1;
	std::vector<uint32_t> intermediate_data;
	sprintf(raw_code_file, "%s", _filename);
	sprintf(spv_code_file, "%s.spv", raw_code_file);
	printf("Actual time: %lld\n", _time);
	stat(raw_code_file, &file_stat_raw);
	stat(spv_code_file, &file_stat_spv);

	printf("Modification %s time: %lld\n", raw_code_file, file_stat_raw.st_mtime);
	printf("Modification %s time: %lld\n", spv_code_file, file_stat_spv.st_mtime);
	if (need_compilation || file_stat_raw.st_mtime > file_stat_spv.st_mtime)
	{
		shader_file = fopen(raw_code_file, "r");
		need_compilation = 1;
	}
	else
		shader_file = fopen(spv_code_file, "rb");
	time_t _start_timer;
	time_t _end_timer;
	if (shader_file)
	{
		if (need_compilation)
		{
			char* data;
			_start_timer = time(NULL);
			fseek(shader_file, 0, SEEK_END);
			int fileSize = ftell(shader_file);
			data = static_cast<char*>(malloc(sizeof(char) * fileSize));
			rewind(shader_file);
			fread(data, fileSize, 1, shader_file);
			fclose(shader_file);
			data[fileSize] = 0; // clean data
			_end_timer = time(NULL);
			printf("Read file elapsed %lld\n", _end_timer - _start_timer);
			_start_timer = time(NULL);
			// Write the raw source
			glslang::TShader* m_TShader = new glslang::TShader(static_cast<EShLanguage>(_stage));
			m_TShader->setStrings(&data, 1);
			m_TShader->setPreamble("#extension GL_GOOGLE_include_directive : require\n");
			m_TShader->setEnvInput(glslang::EShSourceGlsl, static_cast<EShLanguage>(_stage), glslang::EShClientVulkan, GLSL_VERSION);
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
			_end_timer = time(NULL);
			printf("Parse shader %s: %s elapsed time: %lld\n", raw_code_file, parseResult, _end_timer - _start_timer);
			// Write the compiled source
			_start_timer = time(NULL);
			glslang::TProgram m_Program;
			m_Program.addShader(m_TShader);
			m_Program.link(EShMsgDefault);    // link and report default error/warning messages
			printf("Linking program %s: %s\n", raw_code_file, m_Program.getInfoLog()); // get the log for linking the program
			glslang::TIntermediate* m_IntermediateSrc;
			m_IntermediateSrc = m_Program.getIntermediate(static_cast<EShLanguage>(_stage));
			glslang::GlslangToSpv(*m_IntermediateSrc, intermediate_data);
			_end_timer = time(NULL);
			printf("SPIRV creation %s: %s elapsed time %lld\n", raw_code_file, m_TShader->getInfoLog(), _end_timer - _start_timer);
			glslang::OutputSpvBin(intermediate_data, spv_code_file);
		}
		else
		{
			uint32_t* data;
			_start_timer = time(NULL);
			fseek(shader_file, 0, SEEK_END);
			int fileSize = ftell(shader_file);
			data = static_cast<uint32_t*>(malloc(sizeof(uint32_t) * fileSize));
			rewind(shader_file);
			fread(data, fileSize, 1, shader_file);
			fclose(shader_file);
			data[fileSize] = 0; // clean data
			_end_timer = time(NULL);
			printf("Read file elapsed %lld\n", _end_timer - _start_timer);
			intermediate_data.insert(intermediate_data.begin(), data[0], data[fileSize]);
		}
	}
	time_t _end_time = time(NULL);
	printf("Process elapsed %lld\n", _end_time - _time);
	return intermediate_data;
}