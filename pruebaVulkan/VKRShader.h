#pragma once
#include <fstream>
#include <vector>

namespace VKR
{
	namespace render
	{
		inline std::vector<char> LoadShader(const std::string& _filename)
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
