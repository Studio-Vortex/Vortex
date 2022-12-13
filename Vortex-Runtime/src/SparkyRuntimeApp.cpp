#include <Vortex.h>
#include <Vortex/Core/EntryPoint.h>

#include "RuntimeLayer.h"

namespace Vortex {

	class VortexRuntime : public Application
	{
	public:
		VortexRuntime(const ApplicationProperties& props)
			: Application(props)
		{
			PushLayer(new RuntimeLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		std::string applicationName;
		
		if (args.Count > 1)
		{
			const char* projectFilepath = args[1];
			std::string path(projectFilepath);
			std::string projectName = path.substr(0, path.find('.'));
			applicationName = projectName.substr(projectName.find_last_of('\\') + 1, projectName.length());
		}
		else
		{
			applicationName = "Application";
		}

		ApplicationProperties props;
		props.Name = applicationName;
		props.WindowWidth = 1600;
		props.WindowHeight = 900;
		props.SampleCount = 1;
		props.MaximizeWindow = true;
		props.WindowDecorated = false;
		props.VSync = true;
		props.EnableGUI = true;
		props.GraphicsAPI = RendererAPI::API::OpenGL;

		props.WorkingDirectory = "C:/dev/Vortex Engine";
		props.CommandLineArgs = args;

		return new VortexRuntime(props);
	}

}
