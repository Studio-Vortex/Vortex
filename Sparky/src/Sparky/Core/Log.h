#pragma once

#include "Base.h"
#include "Sparky/Core/Math.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma warning(pop)

namespace Sparky {

	class SPARKY_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		static std::vector<std::string> GetMessages(size_t messageCount = 0);

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> s_RingbufferSink;
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

// Core log macros
#define SP_CORE_TRACE(...)      ::Sparky::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SP_CORE_INFO(...)       ::Sparky::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SP_CORE_WARN(...)       ::Sparky::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SP_CORE_ERROR(...)      ::Sparky::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SP_CORE_CRITICAL(...)   ::Sparky::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SP_TRACE(...)           ::Sparky::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SP_INFO(...)            ::Sparky::Log::GetClientLogger()->info(__VA_ARGS__)
#define SP_WARN(...)            ::Sparky::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SP_ERROR(...)           ::Sparky::Log::GetClientLogger()->error(__VA_ARGS__)
#define SP_CRITICAL(...)        ::Sparky::Log::GetClientLogger()->critical(__VA_ARGS__)
