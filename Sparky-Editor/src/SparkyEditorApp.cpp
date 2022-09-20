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
		props.WindowWidth = 1280;
		props.WindowHeight = 720;
		props.MaximizeWindow = true;
		props.WindowDecorated = false;
		props.VSync = true;
		props.EnableGUI = true;

		props.WorkingDirectory = "C:/dev/Sparky Editor Resources";
		props.CommandLineArgs = args;

		return new SparkyEditor(props);
	}
}
