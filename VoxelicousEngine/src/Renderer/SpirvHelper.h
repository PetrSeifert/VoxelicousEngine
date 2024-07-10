#pragma once
#include "vulkan/vulkan.h"
#include "shaderc/shaderc.hpp"

struct SpirvHelper
{
	static std::string ReadFile(const std::filesystem::path& filePath);
	static std::vector<uint32_t> CompileShader(const std::filesystem::path& filePath, shaderc_shader_kind kind, bool optimize);
};