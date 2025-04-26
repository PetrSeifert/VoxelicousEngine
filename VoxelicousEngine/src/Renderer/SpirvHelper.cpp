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
 
std::vector<uint32_t> SpirvHelper::CompileShader(const std::string& filePath, const shaderc_shader_kind kind, const bool optimize)
{
    const shaderc::Compiler compiler;
    shaderc::CompileOptions options;
 
    options.AddMacroDefinition("MY_DEFINE", "1");
    if(optimize)
        options.SetOptimizationLevel(shaderc_optimization_level_size);

    const std::string& source = ReadFile(filePath);
    
    // Extract filename without extension for shader module name
    std::string filename = filePath;
    const size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        filename = filePath.substr(lastSlash + 1);
    const size_t lastDot = filename.find_last_of('.');
    if (lastDot != std::string::npos)
        filename = filename.substr(0, lastDot);
    
    const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, kind, filename.c_str(), options);
 
    if(module.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        std::cerr << module.GetErrorMessage();
        return {};
    }
 
    return { module.cbegin(), module.cend() };
}