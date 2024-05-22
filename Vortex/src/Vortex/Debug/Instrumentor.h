#pragma once

#include "Vortex/Core/Base.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>

#define VX_ENABLE_PROFILING 0

namespace Vortex {

	using VORTEX_API FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

	struct VORTEX_API ProfileResult
	{
		std::string Name;

		FloatingPointMicroseconds Start;
		std::chrono::microseconds ElapsedTime;
		std::thread::id ThreadID;
	};

	struct VORTEX_API InstrumentationSession
	{
		std::string Name;
	};

	class VORTEX_API Instrumentor
	{
	public:
		Instrumentor(const Instrumentor&) = delete;
		Instrumentor(Instrumentor&&) = delete;

		void BeginSession(const std::string& name, const std::string& filepath = "results.json");
		void EndSession();

		void WriteProfile(const ProfileResult& result);

		static Instrumentor& Get();

	private:
		Instrumentor();
		~Instrumentor();

		void WriteHeader();
		void WriteFooter();

		// Note: you must already own lock on m_Mutex before
		// calling InternalEndSession()
		void InternalEndSession();

	private:
		std::mutex m_Mutex;
		InstrumentationSession* m_CurrentSession;
		std::ofstream m_OutputStream;
	};

	class VORTEX_API InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name);
		~InstrumentationTimer();

		float ElapsedMS();
		void Stop();

	private:
		std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
		const char* m_Name;
		bool m_Stopped;
	};

	namespace InstrumentorUtils {

		template <size_t N>
		struct ChangeResult
		{
			char Data[N];
		};

		template <size_t N, size_t K>
		constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
		{
			ChangeResult<N> result = {};

			size_t srcIndex = 0;
			size_t dstIndex = 0;
			while (srcIndex < N)
			{
				size_t matchIndex = 0;
				while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
					matchIndex++;
				if (matchIndex == K - 1)
					srcIndex += matchIndex;
				result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
				srcIndex++;
			}
			return result;
		}
	}
}

#if VX_ENABLE_PROFILING
// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
	#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
		#define VX_FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
		#define VX_FUNC_SIG __PRETTY_FUNCTION__
	#elif (defined(__FUNCSIG__) || (_MSC_VER))
		#define VX_FUNC_SIG __FUNCSIG__
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
		#define VX_FUNC_SIG __FUNCTION__
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
		#define VX_FUNC_SIG __FUNC__
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
		#define VX_FUNC_SIG __func__
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
		#define VX_FUNC_SIG __func__
	#else
		#define VX_FUNC_SIG "VX_FUNC_SIG unknown!"
	#endif

	#define VX_PROFILE_BEGIN_SESSION(name, filepath) ::Vortex::Instrumentor::Get().BeginSession(name, filepath)
	#define VX_PROFILE_END_SESSION() ::Vortex::Instrumentor::Get().EndSession()
	#define VX_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Vortex::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
												   ::Vortex::InstrumentationTimer timer##line(fixedName##line.Data)
	#define VX_PROFILE_SCOPE_LINE(name, line) VX_PROFILE_SCOPE_LINE2(name, line)
	#define VX_PROFILE_SCOPE(name) VX_PROFILE_SCOPE_LINE(name, __LINE__)
	#define VX_PROFILE_FUNCTION() VX_PROFILE_SCOPE(VX_FUNC_SIG)
#else
	#define VX_FUNC_SIG
	#define VX_PROFILE_BEGIN_SESSION(name, filepath)
	#define VX_PROFILE_END_SESSION()
	#define VX_PROFILE_SCOPE(name)
	#define VX_PROFILE_FUNCTION()
#endif