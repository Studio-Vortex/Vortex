#include "vxpch.h"

#include "Vortex/Editor/EditorConsoleSink.h"
#include "Vortex/Utils/FileSystem.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Vortex {

#ifndef VX_DIST
	#define VX_HAS_CONSOLE 1
#endif

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::logger> Log::s_EditorConsoleLogger;

	void Log::Init()
	{
		std::string logsDirectory = "Resources/Logs";
		if (!FileSystem::Exists(logsDirectory))
			FileSystem::CreateDirectoryV(logsDirectory);

		std::vector<spdlog::sink_ptr> vortexSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("Resources/Logs/VORTEX.log", true),
#if VX_HAS_CONSOLE
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
#endif
		};

		std::vector<spdlog::sink_ptr> appSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("Resources/Logs/APP.log", true),
#if VX_HAS_CONSOLE
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
#endif
		};

		std::vector<spdlog::sink_ptr> editorConsoleSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("Resources/Logs/APP.log", true),
#if VX_HAS_CONSOLE
			std::make_shared<EditorConsoleSink>(1)
#endif
		};

		vortexSinks[0]->set_pattern("[%T] [%l] %n: %v");
		appSinks[0]->set_pattern("[%T] [%l] %n: %v");

#if VX_HAS_CONSOLE
		vortexSinks[1]->set_pattern("%^[%T] %n: %v%$");
		appSinks[1]->set_pattern("%^[%T] %n: %v%$");
		for (auto sink : editorConsoleSinks)
			sink->set_pattern("%^%v%$");
#endif

		s_CoreLogger = std::make_shared<spdlog::logger>("VORTEX", vortexSinks.begin(), vortexSinks.end());
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", appSinks.begin(), appSinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);

		s_EditorConsoleLogger = std::make_shared<spdlog::logger>("Console", editorConsoleSinks.begin(), editorConsoleSinks.end());
		s_EditorConsoleLogger->set_level(spdlog::level::trace);
	}

    void Log::Shutdown()
    {
		s_EditorConsoleLogger.reset();
		s_ClientLogger.reset();
		s_CoreLogger.reset();

		spdlog::drop_all();
    }

}
