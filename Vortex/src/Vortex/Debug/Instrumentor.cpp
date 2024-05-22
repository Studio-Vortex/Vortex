#include "vxpch.h"

namespace Vortex {

	void Instrumentor::BeginSession(const std::string& name, const std::string& filepath)
	{
		std::lock_guard lock(m_Mutex);
		if (m_CurrentSession)
		{
			// If there is already a current session, then close it before beginning new one.
			// Subsequent profiling output meant for the original session will end up in the
			// newly opened session instead.  That's better than having badly formatted
			// profiling output.
			if (Log::GetCoreLogger()) // Edge case: BeginSession() might be before Log::Init()
			{
				VX_CORE_ERROR_TAG("Timer", "Instrumentor::BeginSession('{0}') when session '{1}' already open.", name, m_CurrentSession->Name);
			}
			InternalEndSession();
		}
		m_OutputStream.open(filepath);

		if (m_OutputStream.is_open())
		{
			m_CurrentSession = new InstrumentationSession({ name });
			WriteHeader();
		}
		else
		{
			if (Log::GetCoreLogger()) // Edge case: BeginSession() might be before Log::Init()
			{
				VX_CORE_ERROR_TAG("Timer", "Instrumentor could not open results file '{0}'.", filepath);
			}
		}
	}

	void Instrumentor::EndSession()
	{
		std::lock_guard lock(m_Mutex);
		InternalEndSession();
	}

	void Instrumentor::WriteProfile(const ProfileResult& result)
	{
		std::stringstream json;

		json << std::setprecision(3) << std::fixed;
		json << ",{";
		json << "\"cat\":\"function\",";
		json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
		json << "\"name\":\"" << result.Name << "\",";
		json << "\"ph\":\"X\",";
		json << "\"pid\":0,";
		json << "\"tid\":" << result.ThreadID << ",";
		json << "\"ts\":" << result.Start.count();
		json << "}";

		std::lock_guard lock(m_Mutex);
		if (m_CurrentSession)
		{
			m_OutputStream << json.str();
			m_OutputStream.flush();
		}
	}

	Instrumentor& Instrumentor::Get()
	{
		static Instrumentor instance;
		return instance;
	}

	Instrumentor::Instrumentor()
		: m_CurrentSession(nullptr)
	{
	}

	Instrumentor::~Instrumentor()
	{
		EndSession();
	}

	void Instrumentor::WriteHeader()
	{
		m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
		m_OutputStream.flush();
	}

	void Instrumentor::WriteFooter()
	{
		m_OutputStream << "]}";
		m_OutputStream.flush();
	}

	void Instrumentor::InternalEndSession()
	{
		if (m_CurrentSession)
		{
			WriteFooter();
			m_OutputStream.close();
			delete m_CurrentSession;
			m_CurrentSession = nullptr;
		}
	}

	InstrumentationTimer::InstrumentationTimer(const char* name)
		: m_Name(name), m_Stopped(false)
	{
		m_StartTimepoint = std::chrono::steady_clock::now();
	}

	InstrumentationTimer::~InstrumentationTimer()
	{
		if (!m_Stopped)
		{
			Stop();
		}
	}

	float InstrumentationTimer::ElapsedMS()
	{
		const auto endTimepoint = std::chrono::steady_clock::now();
		const auto highResStart = FloatingPointMicroseconds{ m_StartTimepoint.time_since_epoch() };
		const auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

		ProfileResult profile;
		profile.Name = m_Name;
		profile.Start = highResStart;
		profile.ElapsedTime = elapsedTime;
		profile.ThreadID = std::this_thread::get_id();
		
		Instrumentor::Get().WriteProfile(profile);

		m_Stopped = true;
		return elapsedTime.count() / 1000.0f;
	}

	void InstrumentationTimer::Stop()
	{
		const auto endTimepoint = std::chrono::steady_clock::now();
		const auto highResStart = FloatingPointMicroseconds{ m_StartTimepoint.time_since_epoch() };
		const auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

		ProfileResult profile;
		profile.Name = m_Name;
		profile.Start = highResStart;
		profile.ElapsedTime = elapsedTime;
		profile.ThreadID = std::this_thread::get_id();

		Instrumentor::Get().WriteProfile(profile);
		
#if VX_ENABLE_PROFILING
		VX_CONSOLE_LOG_INFO("Timer: {} - {} ms ({} us)", m_Name, elapsedTime.count() / 1000.0f, elapsedTime.count());
#endif

		m_Stopped = true;
	}

}
