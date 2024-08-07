#pragma once

#include "Core/Core.h"
#include "Core/LogCustomFormatters.h"

#include <spdlog/spdlog.h>

#include <map>
#include <memory>
#include <string>
#include <format>
#include <utility>

#define VE_ASSERT_MESSAGE_BOX (!VE_DIST && VE_PLATFORM_WINDOWS)

#if VE_ASSERT_MESSAGE_BOX
	#ifdef VE_PLATFORM_WINDOWS
		#include <Windows.h>
	#endif
#endif

namespace VoxelicousEngine
{
	class Log
	{
	public:
		enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};
		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal
		};
		struct TagDetails
		{
			bool Enabled = true;
			Level LevelFilter = Level::Trace;
		};

	public:
		static void Init();
		static void Shutdown();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		static std::shared_ptr<spdlog::logger>& GetEditorConsoleLogger() { return s_EditorConsoleLogger; }

		static bool HasTag(const std::string& tag) { return s_EnabledTags.find(tag) != s_EnabledTags.end(); }
		static std::map<std::string, TagDetails>& EnabledTags() { return s_EnabledTags; }
		static void SetDefaultTagSettings();

#if defined(VE_PLATFORM_WINDOWS)
		template<typename... Args>
		static void PrintMessage(Type type, Level level, std::format_string<Args...> format, Args&&... args);
#else
		template<typename... Args>
		static void PrintMessage(Type type, Level level, const std::string_view format, Args&&... args);
#endif

		template<typename... Args>
		static void PrintMessageTag(Type type, Level level, std::string_view tag, std::format_string<Args...> format, Args&&... args);

		static void PrintMessageTag(Type type, Level level, std::string_view tag, std::string_view message);

		template<typename... Args>
		static void PrintAssertMessage(Type type, std::string_view prefix, std::format_string<Args...> message, Args&&... args);

		static void PrintAssertMessage(Type type, std::string_view prefix);

	public:
		// Enum utils
		static const char* LevelToString(Level level)
		{
			switch (level)
			{
				case Level::Trace: return "Trace";
				case Level::Info:  return "Info";
				case Level::Warn:  return "Warn";
				case Level::Error: return "Error";
				case Level::Fatal: return "Fatal";
			}
			return "";
		}
		static Level LevelFromString(std::string_view string)
		{
			if (string == "Trace") return Level::Trace;
			if (string == "Info")  return Level::Info;
			if (string == "Warn")  return Level::Warn;
			if (string == "Error") return Level::Error;
			if (string == "Fatal") return Level::Fatal;

			return Level::Trace;
		}

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_EditorConsoleLogger;

		inline static std::map<std::string, TagDetails> s_EnabledTags;
		static std::map<std::string, TagDetails> s_DefaultTagDetails;
	};

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tagged logs (prefer these!)                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core logging
#define VE_CORE_TRACE_TAG(tag, ...) ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Trace, tag, __VA_ARGS__)
#define VE_CORE_INFO_TAG(tag, ...)  ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Info, tag, __VA_ARGS__)
#define VE_CORE_WARN_TAG(tag, ...)  ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Warn, tag, __VA_ARGS__)
#define VE_CORE_ERROR_TAG(tag, ...) ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Error, tag, __VA_ARGS__)
#define VE_CORE_FATAL_TAG(tag, ...) ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Fatal, tag, __VA_ARGS__)

// Client logging
#define VE_TRACE_TAG(tag, ...) ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Trace, tag, __VA_ARGS__)
#define VE_INFO_TAG(tag, ...)  ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Info, tag, __VA_ARGS__)
#define VE_WARN_TAG(tag, ...)  ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Warn, tag, __VA_ARGS__)
#define VE_ERROR_TAG(tag, ...) ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Error, tag, __VA_ARGS__)
#define VE_FATAL_TAG(tag, ...) ::VoxelicousEngine::Log::PrintMessageTag(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Fatal, tag, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core Logging
#define VE_CORE_TRACE(...)  ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Trace, __VA_ARGS__)
#define VE_CORE_INFO(...)   ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Info, __VA_ARGS__)
#define VE_CORE_WARN(...)   ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Warn, __VA_ARGS__)
#define VE_CORE_ERROR(...)  ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Error, __VA_ARGS__)
#define VE_CORE_FATAL(...)  ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Core, ::VoxelicousEngine::Log::Level::Fatal, __VA_ARGS__)

// Client Logging
#define VE_TRACE(...)   ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Trace, __VA_ARGS__)
#define VE_INFO(...)    ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Info, __VA_ARGS__)
#define VE_WARN(...)    ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Warn, __VA_ARGS__)
#define VE_ERROR(...)   ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Error, __VA_ARGS__)
#define VE_FATAL(...)   ::VoxelicousEngine::Log::PrintMessage(::VoxelicousEngine::Log::Type::Client, ::VoxelicousEngine::Log::Level::Fatal, __VA_ARGS__)

// Editor Console Logging Macros
#define VE_CONSOLE_LOG_TRACE(...)   VoxelicousEngine::Log::GetEditorConsoleLogger()->trace(__VA_ARGS__)
#define VE_CONSOLE_LOG_INFO(...)    VoxelicousEngine::Log::GetEditorConsoleLogger()->info(__VA_ARGS__)
#define VE_CONSOLE_LOG_WARN(...)    VoxelicousEngine::Log::GetEditorConsoleLogger()->warn(__VA_ARGS__)
#define VE_CONSOLE_LOG_ERROR(...)   VoxelicousEngine::Log::GetEditorConsoleLogger()->error(__VA_ARGS__)
#define VE_CONSOLE_LOG_FATAL(...)   VoxelicousEngine::Log::GetEditorConsoleLogger()->critical(__VA_ARGS__)

namespace VoxelicousEngine
{
#if defined(VE_PLATFORM_WINDOWS)
	template<typename... Args>
	void Log::PrintMessage(Type type, Level level, std::format_string<Args...> format, Args&&... args)
#else
	template<typename... Args>
	void Log::PrintMessage(Type type, Level level, const std::string_view format, Args&&... args)
#endif
	{
		if (const auto detail = s_EnabledTags[""]; detail.Enabled && detail.LevelFilter <= level)
		{
			const auto logger = type == Type::Core ? GetCoreLogger() : GetClientLogger();
			switch (level)
			{
			case Level::Trace:
				logger->trace(format, std::forward<Args>(args)...);
				break;
			case Level::Info:
				logger->info(format, std::forward<Args>(args)...);
				break;
			case Level::Warn:
				logger->warn(format, std::forward<Args>(args)...);
				break;
			case Level::Error:
				logger->error(format, std::forward<Args>(args)...);
				break;
			case Level::Fatal:
				logger->critical(format, std::forward<Args>(args)...);
				break;
			}
		}
	}


	template<typename... Args>
	void Log::PrintMessageTag(const Type type, const Level level, std::string_view tag, const std::format_string<Args...> format, Args&&... args)
	{
		if (const auto detail = s_EnabledTags[std::string(tag)]; detail.Enabled && detail.LevelFilter <= level)
		{
			const auto logger = type == Type::Core ? GetCoreLogger() : GetClientLogger();
			std::string formatted = std::format(format, std::forward<Args>(args)...);
			switch (level)
			{
				case Level::Trace:
					logger->trace("[{0}] {1}", tag, formatted);
					break;
				case Level::Info:
					logger->info("[{0}] {1}", tag, formatted);
					break;
				case Level::Warn:
					logger->warn("[{0}] {1}", tag, formatted);
					break;
				case Level::Error:
					logger->error("[{0}] {1}", tag, formatted);
					break;
				case Level::Fatal:
					logger->critical("[{0}] {1}", tag, formatted);
					break;
			}
		}
	}


	inline void Log::PrintMessageTag(const Type type, const Level level, std::string_view tag, std::string_view message)
	{
		if (const auto detail = s_EnabledTags[std::string(tag)]; detail.Enabled && detail.LevelFilter <= level)
		{
			const auto logger = type == Type::Core ? GetCoreLogger() : GetClientLogger();
			switch (level)
			{
				case Level::Trace:
					logger->trace("[{0}] {1}", tag, message);
					break;
				case Level::Info:
					logger->info("[{0}] {1}", tag, message);
					break;
				case Level::Warn:
					logger->warn("[{0}] {1}", tag, message);
					break;
				case Level::Error:
					logger->error("[{0}] {1}", tag, message);
					break;
				case Level::Fatal:
					logger->critical("[{0}] {1}", tag, message);
					break;
			}
		}
	}


	template<typename... Args>
	void Log::PrintAssertMessage(const Type type, std::string_view prefix, std::format_string<Args...> message, Args&&... args)
	{
		const auto logger = type == Type::Core ? GetCoreLogger() : GetClientLogger();
		auto formatted = std::format(message, std::forward<Args>(args)...);
		logger->error("{0}: {1}", prefix, formatted);

#if VE_ASSERT_MESSAGE_BOX
		MessageBoxA(nullptr, formatted.data(), "VoxelicousEngine Assert", MB_OK | MB_ICONERROR);
#endif
	}


	inline void Log::PrintAssertMessage(const Type type, std::string_view prefix)
	{
		const auto logger = type == Type::Core ? GetCoreLogger() : GetClientLogger();
		logger->error("{0}", prefix);
#if VE_ASSERT_MESSAGE_BOX
		MessageBoxA(nullptr, "No message :(", "VoxelicousEngine Assert", MB_OK | MB_ICONERROR);
#endif
	}
}