#include "vepch.h"
#include "SpirvHelper.h"

#include <fstream>
#include <iostream>

std::string SpirvHelper::ReadFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
 
    if(!file.is_open())
        VE_CORE_ERROR("Failed to open file: {}", filePath);

    const size_t fileSize = file.tellg();
    std::string buffer;
    buffer.resize(fileSize);
 
    file.seekg(0);
    file.read(buffer.data(), fileSize);
 
    file.close();
 
    return buffer;
}
 
std::vector<uint32_t> SpirvHelper::LoadSpirvBinary(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    
    if(!file.is_open())
    {
        VE_CORE_ERROR("Failed to open SPIR-V binary file: {}", filePath);
        return {};
    }
    
    const size_t fileSize = file.tellg();
    if (fileSize % sizeof(uint32_t) != 0)
    {
        VE_CORE_ERROR("SPIR-V binary size is not a multiple of sizeof(uint32_t): {}", filePath);
        return {};
    }
    
    std::vector<uint32_t> spirvCode(fileSize / sizeof(uint32_t));
    
    file.seekg(0);
    file.read(reinterpret_cast<char*>(spirvCode.data()), fileSize);
    file.close();
    
    return spirvCode;
}