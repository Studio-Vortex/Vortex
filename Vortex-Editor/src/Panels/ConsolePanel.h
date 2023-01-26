#pragma once

#include <Vortex.h>
#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class ConsolePanel : public EditorPanel
	{
	public:
		ConsolePanel() = default;
		~ConsolePanel() override = default;

		void OnGuiRender() override;
		void SetProjectContext(SharedRef<Project> project) override {}
		void SetSceneContext(SharedRef<Scene> scene) override {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
