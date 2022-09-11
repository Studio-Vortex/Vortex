#include <Sparky.h>
#include <Sparky/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Sparky {

	class SparkyEditor : public Application
	{
	public:
		SparkyEditor()
			: Application("Sparky Editor")
		{
			PushLayer(new EditorLayer());
		}

		~SparkyEditor() override = default;
	};

	Application* CreateApplication() {
		return new SparkyEditor();
	}
}
