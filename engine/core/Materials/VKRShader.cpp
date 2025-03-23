#include "VKRShader.h"

#ifdef _UNIX
#include <csignal>
#endif

std::vector<uint32_t> _read_shader(const char* _filename, int _stage);

namespace VKR
{
	namespace render
	{
		Shader::Shader(const std::string& _filename, int _shaderStage)
		{
			m_Filename = _filename;
			m_Stage = _shaderStage;
		}
		void Shader::LoadShader()
		{
			m_SpirvSrc = _read_shader(m_Filename.c_str(), m_Stage);
		}

		void Shader::ConfigureShader(VkDevice m_LogicDevice, VkShaderStageFlagBits _type,  VkPipelineShaderStageCreateInfo* shaderStageInfo_)
		{
			VkShaderModule shaderModule;
			VkShaderModuleCreateInfo shaderModuleCreateInfo{};
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
#ifdef WIN32
			shaderModuleCreateInfo.codeSize = m_SpirvSrc.size();
			shaderModuleCreateInfo.pCode = m_SpirvSrc.data();
#else
			shaderModuleCreateInfo.codeSize = m_CompiledSource.size();
			shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_CompiledSource.data());
#endif
			if (vkCreateShaderModule(m_LogicDevice, &shaderModuleCreateInfo, nullptr, &shaderModule)
				!= VK_SUCCESS)
				#ifdef WIN32
					__debugbreak();
				#else
					raise(SIGTRAP);
				#endif
			shaderStageInfo_->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo_->stage = _type;
			shaderStageInfo_->module = shaderModule;
			shaderStageInfo_->pName = "main";
		}
	}
}
