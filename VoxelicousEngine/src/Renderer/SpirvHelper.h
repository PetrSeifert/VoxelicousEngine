#pragma once
#include "vulkan/vulkan.h"
#include "shaderc/shaderc.hpp"
#include <string>
#include <vector>

struct SpirvHelper
{
	static std::string ReadFile(const std::string& filePath);
	static std::vector<uint32_t> CompileShader(const std::string& filePath, shaderc_shader_kind kind, bool optimize);
};