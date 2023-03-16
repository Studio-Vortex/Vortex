#pragma once

#include <Vortex.h>

namespace Vortex {

	class PhysicsStatisticsPanel
	{
	public:

		PhysicsStatisticsPanel() = default;
		~PhysicsStatisticsPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(const SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
