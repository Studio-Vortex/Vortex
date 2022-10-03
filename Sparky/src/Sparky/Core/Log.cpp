#include "sppch.h"

namespace Sparky {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> Log::s_RingbufferSink;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("SPARKY");
		s_CoreLogger->set_level(spdlog::level::trace);
		
		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);

		// Ringbuffer sink allows us to retrives a certain number of logged messages
		s_RingbufferSink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(128);

		// Add the sink to both loggers to retrive messages later on
		s_CoreLogger->sinks().push_back(s_RingbufferSink);
		s_ClientLogger->sinks().push_back(s_RingbufferSink);

		auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Resources/Logs/Log.txt");

		s_CoreLogger->sinks().push_back(fileSink);
		s_ClientLogger->sinks().push_back(fileSink);
	}

	std::vector<std::string> Log::GetMessages(size_t messageCount)
	{
		return s_RingbufferSink->last_formatted(messageCount);
	}

}