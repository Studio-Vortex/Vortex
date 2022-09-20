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
		props.WorkingDirectory = "C:/dev/Sparky Editor Resources";
		props.GraphicsAPI = RendererAPI::API::OpenGL;
		props.CommandLineArgs = args;
		props.MaxmizeWindow = true;

		return new SparkyEditor(props);
	}
}
