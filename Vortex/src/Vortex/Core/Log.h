#pragma once

#include "Base.h"
#include "Vortex/Core/Math.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>
#pragma warning(pop)

#include <map>

namespace Vortex {

	class VORTEX_API Log
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
		static void Init(bool enableFileOutput = false);
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetEditorConsoleLogger() { return s_EditorConsoleLogger; }

		static bool HasTag(const std::string& tag) { return s_EnabledTags.find(tag) != s_EnabledTags.end(); }
		static std::map<std::string, TagDetails>& EnabledTags() { return s_EnabledTags; }

		template <typename... Args>
		static void PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args);

	private:
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
	};

}

template <typename OStream>
inline OStream& operator<<(OStream& os, const glm::vec2& vector)
{
	return os << "vec2(" << vector.x << ", " << vector.y << ")";
}

template <typename OStream>
inline OStream& operator<<(OStream& os, const glm::vec3& vector)
{
	return os << "vec3(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
}

template <typename OStream>
inline OStream& operator<<(OStream& os, const glm::vec4& vector)
{
	return os << "vec4(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
}

template <typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template <typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

namespace fmt {

	template<>
	struct formatter<glm::vec2>
	{
		char presentation = 'f';

		constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
		{
			auto it = ctx.begin(), end = ctx.end();
			if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

			if (it != end && *it != '}') throw format_error("invalid format");

			return it;
		}

		template <typename FormatContext>
		auto format(const glm::vec2& vec, FormatContext& ctx) const -> decltype(ctx.out())
		{
			return presentation == 'f'
				? fmt::format_to(ctx.out(), "({:.3f}, {:.3f})", vec.x, vec.y)
				: fmt::format_to(ctx.out(), "({:.3e}, {:.3e})", vec.x, vec.y);
		}
	};

	template<>
	struct formatter<glm::vec3>
	{
		char presentation = 'f';

		constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
		{
			auto it = ctx.begin(), end = ctx.end();
			if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

			if (it != end && *it != '}') throw format_error("invalid format");

			return it;
		}

		template <typename FormatContext>
		auto format(const glm::vec3& vec, FormatContext& ctx) const -> decltype(ctx.out())
		{
			return presentation == 'f'
				? fmt::format_to(ctx.out(), "({:.3f}, {:.3f}, {:.3f})", vec.x, vec.y, vec.z)
				: fmt::format_to(ctx.out(), "({:.3e}, {:.3e}, {:.3e})", vec.x, vec.y, vec.z);
		}
	};

	template<>
	struct formatter<glm::vec4>
	{
		char presentation = 'f';

		constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
		{
			auto it = ctx.begin(), end = ctx.end();
			if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

			if (it != end && *it != '}') throw format_error("invalid format");

			return it;
		}

		template <typename FormatContext>
		auto format(const glm::vec4& vec, FormatContext& ctx) const -> decltype(ctx.out())
		{
			return presentation == 'f'
				? fmt::format_to(ctx.out(), "({:.3f}, {:.3f}, {:.3f}, {:.3f})", vec.x, vec.y, vec.z, vec.w)
				: fmt::format_to(ctx.out(), "({:.3e}, {:.3e}, {:.3e}, {:.3e})", vec.x, vec.y, vec.z, vec.w);
		}
	};

}

namespace Vortex {

	template <typename... Args>
	void Log::PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args)
	{
		/*auto detail = s_EnabledTags[std::string(tag)];
		if (detail.Enabled && detail.LevelFilter <= level)
		{
			auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
			std::string logString = tag.empty() ? "{0}{1}" : "[{0}] {1}";

			switch (level)
			{
			case Level::Trace:
				logger->trace(logString, tag, fmt::runtime(fmt::format(std::forward<Args>(args)...)));
				break;
			case Level::Info:
				logger->info(logString, tag, fmt::runtime(fmt::format(std::forward<Args>(args)...)));
				break;
			case Level::Warn:
				logger->warn(logString, tag, fmt::runtime(fmt::format(std::forward<Args>(args)...)));
				break;
			case Level::Error:
				logger->error(logString, tag, fmt::runtime(fmt::format(std::forward<Args>(args)...)));
				break;
			case Level::Fatal:
				logger->critical(logString, tag, fmt::runtime(fmt::format(std::forward<Args>(args)...)));
				break;
			}
		}*/
	}
}


// Core logging
#define VX_CORE_TRACE_TAG(tag, ...) ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Trace, tag, __VA_ARGS__)
#define VX_CORE_INFO_TAG(tag, ...)  ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Info, tag, __VA_ARGS__)
#define VX_CORE_WARN_TAG(tag, ...)  ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Warn, tag, __VA_ARGS__)
#define VX_CORE_ERROR_TAG(tag, ...) ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Error, tag, __VA_ARGS__)
#define VX_CORE_FATAL_TAG(tag, ...) ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Fatal, tag, __VA_ARGS__)

// Client logging
#define VX_TRACE_TAG(tag, ...) ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Trace, tag, __VA_ARGS__)
#define VX_INFO_TAG(tag, ...)  ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Info, tag, __VA_ARGS__)
#define VX_WARN_TAG(tag, ...)  ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Warn, tag, __VA_ARGS__)
#define VX_ERROR_TAG(tag, ...) ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Error, tag, __VA_ARGS__)
#define VX_FATAL_TAG(tag, ...) ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Fatal, tag, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core Logging
#define VX_CORE_TRACE(...)  ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Trace, "", __VA_ARGS__)
#define VX_CORE_INFO(...)   ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Info, "", __VA_ARGS__)
#define VX_CORE_WARN(...)   ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Warn, "", __VA_ARGS__)
#define VX_CORE_ERROR(...)  ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Error, "", __VA_ARGS__)
#define VX_CORE_FATAL(...)  ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Core, ::Vortex::Log::Level::Fatal, "", __VA_ARGS__)

// Client Logging
#define VX_TRACE(...)   ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Trace, "", __VA_ARGS__)
#define VX_INFO(...)    ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Info, "", __VA_ARGS__)
#define VX_WARN(...)    ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Warn, "", __VA_ARGS__)
#define VX_ERROR(...)   ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Error, "", __VA_ARGS__)
#define VX_FATAL(...)   ::Vortex::Log::PrintMessage(::Vortex::Log::Type::Client, ::Vortex::Log::Level::Fatal, "", __VA_ARGS__)

// Editor Console Logging Macros
#define VX_CONSOLE_LOG_TRACE(...)   Vortex::Log::GetEditorConsoleLogger()->trace(__VA_ARGS__)
#define VX_CONSOLE_LOG_INFO(...)    Vortex::Log::GetEditorConsoleLogger()->info(__VA_ARGS__)
#define VX_CONSOLE_LOG_WARN(...)    Vortex::Log::GetEditorConsoleLogger()->warn(__VA_ARGS__)
#define VX_CONSOLE_LOG_ERROR(...)   Vortex::Log::GetEditorConsoleLogger()->error(__VA_ARGS__)
#define VX_CONSOLE_LOG_FATAL(...)   Vortex::Log::GetEditorConsoleLogger()->critical(__VA_ARGS__)
