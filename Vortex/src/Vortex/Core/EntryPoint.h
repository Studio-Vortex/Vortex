#pragma once

#include "Vortex/Core/Base.h"
#include "Application.h"

#ifdef VX_PLATFORM_WINDOWS

	extern Vortex::Application* Vortex::CreateApplication(Vortex::ApplicationCommandLineArgs);
	bool g_ApplicationRunning = true;

	int main(int argc, char* argv[])
	{
		Vortex::InitalizeEngine();

		while (g_ApplicationRunning)
		{
			VX_PROFILE_BEGIN_SESSION("Startup", "VortexProfile-Startup.json");
			auto app = Vortex::CreateApplication({ argc, argv });
			VX_PROFILE_END_SESSION();

			VX_PROFILE_BEGIN_SESSION("Runtime", "VortexProfile-Runtime.json");
			app->Run();
			VX_PROFILE_END_SESSION();

			VX_PROFILE_BEGIN_SESSION("Shutdown", "VortexProfile-Shutdown.json");
			delete app;
			VX_PROFILE_END_SESSION();
		}

		Vortex::ShutdownEngine();
	}

#elif VX_PLATFORM_MACOS

	extern Vortex::Application* Vortex::CreateApplication();

	int main(int argc, char* argv[])
	{
		Vortex::Log::Init();

		auto app = Vortex::CreateApplication({ argc, argv });

		app->Run();

		delete app;
	}

#endif // VX_PLATFORM_WINDOWS
