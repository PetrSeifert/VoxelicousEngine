#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace VoxelicousEngine {

	class VOXELICOUS_ENGINE_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// Core log macros
#define VE_CORE_TRACE(...)    ::VoxelicousEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VE_CORE_INFO(...)     ::VoxelicousEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define VE_CORE_WARN(...)     ::VoxelicousEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VE_CORE_ERROR(...)    ::VoxelicousEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define VE_CORE_FATAL(...)    ::VoxelicousEngine::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define VE_TRACE(...)	      ::VoxelicousEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define VE_INFO(...)	      ::VoxelicousEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define VE_WARN(...)	      ::VoxelicousEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define VE_ERROR(...)	      ::VoxelicousEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define VE_FATAL(...)	      ::VoxelicousEngine::Log::GetClientLogger()->fatal(__VA_ARGS__)