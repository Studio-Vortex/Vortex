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
		void SetSceneContext(const SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void DrawHeading(const char* title);

	private:
		inline static bool s_ShowPanel = false;
	};

}
