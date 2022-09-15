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

		~SparkyEditor() override = default;
	};

	Application* CreateApplication() {
		ApplicationProperties props;
		props.Name = "Sparky Editor";
		props.GraphicsAPI = RendererAPI::API::OpenGL;
		props.MaxmizeWindow = false;

		return new SparkyEditor(props);
	}
}
