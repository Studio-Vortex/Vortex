#pragma once

#include <Vortex.h>
#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class PerformancePanel : public EditorPanel
	{
	public:
		PerformancePanel() = default;
		~PerformancePanel() override = default;

		void OnGuiRender(size_t entityCount);
		void SetProjectContext(SharedRef<Project> project) override {}
		void SetSceneContext(SharedRef<Scene> scene) override {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
