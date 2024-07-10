#include "vepch.h"
#include "SpirvHelper.h"

#include <filesystem>

std::string ReadFile(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
 
    if(!file.is_open())
        VE_CORE_ERROR("Failed to open file: {}", filePath.string());

    const size_t fileSize = file.tellg();
    std::string buffer;
    buffer.resize(fileSize);
 
    file.seekg(0);
    file.read(buffer.data(), fileSize);
 
    file.close();
 
    return buffer;
}
 
std::vector<uint32_t> CompileShader(const std::filesystem::path& filePath, const shaderc_shader_kind kind, const bool optimize)
{
    const shaderc::Compiler compiler;
    shaderc::CompileOptions options;
 
    options.AddMacroDefinition("MY_DEFINE", "1");
    if(optimize)
        options.SetOptimizationLevel(shaderc_optimization_level_size);

    const std::string& source = ReadFile(filePath);
    const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, kind, filePath.stem().string().c_str(), options);
 
    if(module.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        std::cerr << module.GetErrorMessage();
        return {};
    }
 
    return { module.cbegin(), module.cend() };
}