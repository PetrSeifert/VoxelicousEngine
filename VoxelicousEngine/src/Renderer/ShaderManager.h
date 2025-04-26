#pragma once

#include "vulkan/vulkan.h"
#include "Device.h"
#include <string>
#include <unordered_map>
#include <filesystem>
#include <optional>

namespace VoxelicousEngine
{
    enum class ShaderType
    {
        Vertex,
        Fragment,
        Compute,
        Unknown
    };

    struct ShaderInfo
    {
        std::string filePath;
        ShaderType type;
        bool hasChanged = false;
        std::optional<std::vector<uint32_t>> compiledCode;
        std::filesystem::file_time_type lastModifiedTime;
    };

    class ShaderManager
    {
    public:
        ShaderManager();
        ~ShaderManager();

        // Prevent copying
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        // Load and compile a shader, return SPIR-V code
        std::vector<uint32_t> LoadShader(const std::string& filePath, ShaderType type, bool forceRecompile = false);
        
        // Check if any shaders need recompilation and update them
        void CheckForChanges();
        
        // Automatically determine shader type from file extension
        static ShaderType GetShaderTypeFromPath(const std::string& filePath);
        
        // Get argument for the shader type for glslc
        static std::string GetShaderKindArg(ShaderType type);
        
        // Get path where compiled shaders should be stored
        static std::string GetCompiledShaderPath(const std::string& filePath);

    private:
        std::unordered_map<std::string, ShaderInfo> m_ShaderCache;
        bool m_OptimizeShaders = true;

        // Check if source file has been modified
        bool HasShaderSourceChanged(const std::string& filePath, std::filesystem::file_time_type& lastModified);
        
        // Compile shader using glslc command-line compiler
        bool CompileShaderWithGlslc(const std::string& filePath, ShaderType type, bool optimize);
        
        // Read compiled shader from file if it exists
        std::optional<std::vector<uint32_t>> ReadCompiledShader(const std::string& filePath);
    };
} 