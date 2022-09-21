#include <Sparky.h>
#include <Sparky/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Sparky {

	class SparkyEditor : public Application
	{
	public:
		SparkyEditor(const ApplicationProperties& props)
			: Application(props)
		{
			PushLayer(new EditorLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationProperties props;
		props.Name = "Sparky Editor";
		props.WindowWidth = 1600;
		props.WindowHeight = 900;
		props.MaximizeWindow = false;
		props.WindowDecorated = true;
		props.VSync = true;
		props.EnableGUI = true;
		props.GraphicsAPI = RendererAPI::API::OpenGL;

		props.WorkingDirectory = "C:/dev/Sparky_Editor_Resources";
		props.CommandLineArgs = args;

		return new SparkyEditor(props);
	}
}
