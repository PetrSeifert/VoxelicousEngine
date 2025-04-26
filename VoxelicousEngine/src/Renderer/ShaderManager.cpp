#include "vepch.h"
#include "ShaderManager.h"
#include "Core/Core.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <process.h>
#include <algorithm>  // For std::replace

namespace VoxelicousEngine
{
    ShaderManager::ShaderManager() = default;
    ShaderManager::~ShaderManager() = default;

    std::vector<uint32_t> ShaderManager::LoadShader(const std::string& filePath, ShaderType type, bool forceRecompile)
    {
        // If type is unknown, try to infer from file extension
        ShaderType shaderType = type;
        if (shaderType == ShaderType::Unknown)
        {
            shaderType = GetShaderTypeFromPath(filePath);
            if (shaderType == ShaderType::Unknown)
            {
                VE_CORE_ERROR("Could not determine shader type from file extension: {}", filePath);
                return {};
            }
        }

        // Check if we already have this shader in the cache
        auto it = m_ShaderCache.find(filePath);
        if (it != m_ShaderCache.end())
        {
            // Check if shader needs recompilation
            std::filesystem::file_time_type lastModified;
            bool sourceChanged = HasShaderSourceChanged(filePath, lastModified);
            
            // Update the last modified time
            it->second.lastModifiedTime = lastModified;
            
            // If the shader has changed or force recompilation is requested
            if (sourceChanged || forceRecompile)
            {
                VE_CORE_INFO("Recompiling shader: {}", filePath);
                
                if (CompileShaderWithGlslc(filePath, shaderType, m_OptimizeShaders))
                {
                    it->second.hasChanged = true;
                    it->second.compiledCode = ReadCompiledShader(GetCompiledShaderPath(filePath));
                }
                
                return it->second.compiledCode.value_or(std::vector<uint32_t>{});
            }
            
            // If the shader hasn't changed and we have compiled code
            if (it->second.compiledCode.has_value())
            {
                return it->second.compiledCode.value();
            }
        }
        else
        {
            // First time seeing this shader, add it to the cache
            ShaderInfo info;
            info.filePath = filePath;
            info.type = shaderType;
            
            // Check if there's already a compiled version on disk
            std::optional<std::vector<uint32_t>> existingCompiled = ReadCompiledShader(GetCompiledShaderPath(filePath));
            
            // Get the last modified time of the source
            std::filesystem::file_time_type lastModified;
            bool sourceChanged = HasShaderSourceChanged(filePath, lastModified);
            info.lastModifiedTime = lastModified;
            
            // If we don't have a compiled version or the source has changed, compile it
            if (!existingCompiled.has_value() || sourceChanged || forceRecompile)
            {
                VE_CORE_INFO("Compiling shader: {}", filePath);
                
                if (CompileShaderWithGlslc(filePath, shaderType, m_OptimizeShaders))
                {
                    info.compiledCode = ReadCompiledShader(GetCompiledShaderPath(filePath));
                }
            }
            else
            {
                // Use existing compiled version
                info.compiledCode = existingCompiled;
            }
            
            // Add to cache
            m_ShaderCache[filePath] = info;
            
            // Return the compiled code or empty vector if compilation failed
            return info.compiledCode.value_or(std::vector<uint32_t>{});
        }
        
        // If we reach here, we need to compile the shader
        VE_CORE_INFO("Compiling shader: {}", filePath);
        
        bool success = CompileShaderWithGlslc(filePath, shaderType, m_OptimizeShaders);
        
        // Update or create cache entry
        ShaderInfo& info = m_ShaderCache[filePath];
        info.filePath = filePath;
        info.type = shaderType;
        
        if (success)
        {
            info.compiledCode = ReadCompiledShader(GetCompiledShaderPath(filePath));
        }
        
        return info.compiledCode.value_or(std::vector<uint32_t>{});
    }

    void ShaderManager::CheckForChanges()
    {
        for (auto& [path, info] : m_ShaderCache)
        {
            std::filesystem::file_time_type lastModified;
            bool changed = HasShaderSourceChanged(path, lastModified);
            
            if (changed)
            {
                VE_CORE_INFO("Shader changed, recompiling: {}", path);
                info.lastModifiedTime = lastModified;
                
                if (CompileShaderWithGlslc(path, info.type, m_OptimizeShaders))
                {
                    info.compiledCode = ReadCompiledShader(GetCompiledShaderPath(path));
                    info.hasChanged = true;
                }
            }
        }
    }

    ShaderType ShaderManager::GetShaderTypeFromPath(const std::string& filePath)
    {
        const std::string extension = std::filesystem::path(filePath).extension().string();
        
        if (extension == ".vert")
            return ShaderType::Vertex;
        else if (extension == ".frag")
            return ShaderType::Fragment;
        else if (extension == ".comp")
            return ShaderType::Compute;
        
        return ShaderType::Unknown;
    }

    std::string ShaderManager::GetShaderKindArg(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::Vertex:
            return "-fshader-stage=vertex";
        case ShaderType::Fragment:
            return "-fshader-stage=fragment";
        case ShaderType::Compute:
            return "-fshader-stage=compute";
        default:
            VE_CORE_ERROR("Unknown shader type");
            return "";
        }
    }

    std::string ShaderManager::GetCompiledShaderPath(const std::string& filePath)
    {
        // Simply append .spv to the original path
        return filePath + ".spv";
    }

    bool ShaderManager::HasShaderSourceChanged(const std::string& filePath, std::filesystem::file_time_type& lastModified)
    {
        try
        {
            const auto path = std::filesystem::path(filePath);
            if (!std::filesystem::exists(path))
            {
                VE_CORE_ERROR("Shader file does not exist: {}", filePath);
                return false;
            }
            
            lastModified = std::filesystem::last_write_time(path);
            
            // Check if this file is in the cache and has a different timestamp
            auto it = m_ShaderCache.find(filePath);
            if (it != m_ShaderCache.end())
            {
                return lastModified > it->second.lastModifiedTime;
            }
            
            // If not in cache, consider it changed
            return true;
        }
        catch (const std::exception& e)
        {
            VE_CORE_ERROR("Error checking shader file modification time: {}", e.what());
            return false;
        }
    }

    bool ShaderManager::CompileShaderWithGlslc(const std::string& filePath, ShaderType type, bool optimize)
    {
        try
        {
            // Make sure the file exists
            if (!std::filesystem::exists(filePath))
            {
                VE_CORE_ERROR("Shader file does not exist: {}", filePath);
                return false;
            }
            
            // Use a fixed path to glslc
            std::string glslcPath;
            
            // Try to get VULKAN_SDK environment variable
            const char* vulkanSdkDir = std::getenv("VULKAN_SDK");
            if (vulkanSdkDir)
            {
                glslcPath = std::string(vulkanSdkDir) + "\\bin\\glslc.exe";
                if (!std::filesystem::exists(glslcPath))
                {
                    VE_CORE_ERROR("Failed to find glslc.exe. Please set VULKAN_SDK environment variable.");
                    return false;
                }
            }
            else
            {
                VE_CORE_ERROR("Failed to find glslc.exe. Please set VULKAN_SDK environment variable.");
                return false;
            }

            
            // Normalize file paths to avoid any issues with slashes
            std::string normalizedInputPath = filePath;
            std::replace(normalizedInputPath.begin(), normalizedInputPath.end(), '/', '\\');
            
            std::string normalizedOutputPath = GetCompiledShaderPath(filePath);
            std::replace(normalizedOutputPath.begin(), normalizedOutputPath.end(), '/', '\\');
            
            // Construct the command for glslc - use cmd.exe to ensure the command is executed correctly
            std::string cmd = "cmd.exe /c \"\"" + glslcPath + "\" ";
            
            // Add optimization flag if requested
            if (optimize)
                cmd += "-O ";
            
            // Input file and output file with an extra closing quote
            cmd += "\"" + normalizedInputPath + "\" -o \"" + normalizedOutputPath + "\"\"";
            
            // Execute the command
            VE_CORE_INFO("Executing: {}", cmd);
            int result = system(cmd.c_str());
            
            if (result != 0)
            {
                VE_CORE_ERROR("Failed to compile shader with glslc. Error code: {}", result);
                return false;
            }
            
            return true;
        }
        catch (const std::exception& e)
        {
            VE_CORE_ERROR("Error compiling shader with glslc: {}", e.what());
            return false;
        }
    }

    std::optional<std::vector<uint32_t>> ShaderManager::ReadCompiledShader(const std::string& filePath)
    {
        try
        {
            const auto path = std::filesystem::path(filePath);
            if (!std::filesystem::exists(path))
            {
                return std::nullopt;
            }
            
            std::ifstream file(filePath, std::ios::binary | std::ios::ate);
            if (!file.is_open())
            {
                VE_CORE_ERROR("Failed to open compiled shader file: {}", filePath);
                return std::nullopt;
            }
            
            const size_t fileSize = file.tellg();
            if (fileSize % sizeof(uint32_t) != 0)
            {
                VE_CORE_ERROR("Compiled shader file size is not a multiple of sizeof(uint32_t): {}", filePath);
                return std::nullopt;
            }
            
            std::vector<uint32_t> spirvCode(fileSize / sizeof(uint32_t));
            
            file.seekg(0);
            file.read(reinterpret_cast<char*>(spirvCode.data()), fileSize);
            file.close();
            
            return spirvCode;
        }
        catch (const std::exception& e)
        {
            VE_CORE_ERROR("Error reading compiled shader: {}", e.what());
            return std::nullopt;
        }
    }
} 