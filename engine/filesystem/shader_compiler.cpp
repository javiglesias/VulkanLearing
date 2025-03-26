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

void precompile_shader(const char* _filename, int _stage)
{
	char raw_code_file[128];
	char spv_code_file[128];
	memset(raw_code_file, 0, 128);
	memset(spv_code_file, 0, 128);
	sprintf(raw_code_file, "%s", _filename);
	sprintf(spv_code_file, "%s.spv", raw_code_file);
	struct stat file_stat_raw;
	struct stat file_stat_spv;
	stat(raw_code_file, &file_stat_raw);
	stat(spv_code_file, &file_stat_spv);
	

	char* data;
	FILE* shader_file = nullptr;
	time_t _time = time(NULL);
	time_t _start_timer;
	time_t _end_timer;

	std::vector<uint32_t> intermediate_data;
	intermediate_data.clear();

	_start_timer = time(NULL);
	shader_file = fopen(raw_code_file, "r");
	fseek(shader_file, 0, SEEK_END);
	int fileSize = ftell(shader_file);
	data = static_cast<char*>(malloc(sizeof(char) * fileSize));
	memset(data, 0, sizeof(char) * fileSize);
	rewind(shader_file);
	fread(data, fileSize, 1, shader_file);
	fclose(shader_file);
	data[fileSize] = 0; // clean data
	_end_timer = time(NULL);
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
	fprintf(stderr, "Parse result: %s\n", m_TShader->getInfoLog());
	_end_timer = time(NULL);
	// Write the compiled source
	_start_timer = time(NULL);
	glslang::TProgram m_Program;
	m_Program.addShader(m_TShader);
	m_Program.link(EShMsgDefault);    // link and report default error/warning messages
	fprintf(stderr, "Link result: %s\n", m_TShader->getInfoLog());
	glslang::TIntermediate* m_IntermediateSrc;
	m_IntermediateSrc = m_Program.getIntermediate(static_cast<EShLanguage>(_stage));
	glslang::GlslangToSpv(*m_IntermediateSrc, intermediate_data);
	glslang::OutputSpvBin(intermediate_data, spv_code_file);
	_end_timer = time(NULL);
	fprintf(stdout, "SPIRV compilation %s: %s elapsed time %lld\n", raw_code_file, m_TShader->getInfoLog(), _end_timer - _start_timer);
}

std::vector<uint32_t> _read_shader(const char* _filename, int _stage)
{
	FILE* shader_file = nullptr;
	time_t _start_timer;
	time_t _end_timer;
	char spv_code_file[128];
	std::vector<uint32_t> intermediate_data;
	intermediate_data.clear();
	memset(spv_code_file, 0, 128);
	sprintf(spv_code_file, "%s.spv", _filename);
	uint32_t* data;

	_start_timer = time(NULL);
	shader_file = fopen(spv_code_file, "rb");
	if (shader_file)
	{
		fseek(shader_file, 0, SEEK_END);
		int fileSize = ftell(shader_file);
		data = static_cast<uint32_t*>(malloc(sizeof(uint32_t) * fileSize));
		rewind(shader_file);
		fread(data, fileSize, 1, shader_file);
		fclose(shader_file);
		data[fileSize] = 0; // clean data
		_end_timer = time(NULL);
		printf("Read shader %s elapsed %lld\n", spv_code_file, _end_timer - _start_timer);
		for (size_t datum = 0; datum < fileSize; datum++)
			intermediate_data.push_back(data[datum]);
	}
	time_t _end_time = time(NULL);
	return intermediate_data;
}