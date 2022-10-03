#include <Sparky.h>
#include <Sparky/Core/EntryPoint.h>

#include "RuntimeLayer.h"

namespace Sparky {

	class SparkyRuntime : public Application
	{
	public:
		SparkyRuntime(const ApplicationProperties& props)
			: Application(props)
		{
			PushLayer(new RuntimeLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		const char* sceneFilepath = args[1];
		std::string path(sceneFilepath);
		std::string sceneName = path.substr(0, path.find('.'));
		std::string applicationName = sceneName.substr(sceneName.find_last_of('\\') + 1, sceneName.length());

		ApplicationProperties props;
		props.Name = applicationName;
		props.WindowWidth = 1600;
		props.WindowHeight = 900;
		props.MaximizeWindow = true;
		props.WindowDecorated = false;
		props.VSync = true;
		props.EnableGUI = true;
		props.GraphicsAPI = RendererAPI::API::OpenGL;

		//props.WorkingDirectory = "../Sparky-Editor";
		props.WorkingDirectory = "C:/dev/Sparky Engine";
		props.CommandLineArgs = args;

		return new SparkyRuntime(props);
	}

}