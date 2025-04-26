#pragma once
#include "vulkan/vulkan.h"
#include <string>
#include <vector>

struct SpirvHelper
{
	static std::string ReadFile(const std::string& filePath);
	static std::vector<uint32_t> LoadSpirvBinary(const std::string& filePath);
};