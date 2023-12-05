#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Application.h"

extern Vortex::Application* Vortex::CreateApplication(Vortex::ApplicationCommandLineArgs);
bool g_ApplicationRunning = true;

int main(int argc, char** argv)
{
	Vortex::InitalizeEngine();

	while (g_ApplicationRunning)
	{	
		VX_PROFILE_BEGIN_SESSION("Startup", "VortexProfile-Startup.json");
		Vortex::Application* application = Vortex::CreateApplication({ argc, argv });
		VX_PROFILE_END_SESSION();

		VX_PROFILE_BEGIN_SESSION("Runtime", "VortexProfile-Runtime.json");
		application->Run();
		VX_PROFILE_END_SESSION();

		VX_PROFILE_BEGIN_SESSION("Shutdown", "VortexProfile-Shutdown.json");
		delete application;
		VX_PROFILE_END_SESSION();
	}

	Vortex::ShutdownEngine();
}
