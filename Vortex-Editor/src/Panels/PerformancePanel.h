#pragma once

#include <Vortex.h>

namespace Vortex {

	class PerformancePanel
	{
	public:
		PerformancePanel() = default;
		~PerformancePanel() = default;

		void OnGuiRender(size_t entityCount);
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(SharedRef<Scene> scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
