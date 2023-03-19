#pragma once

#include <Vortex.h>

namespace Vortex {

	class PhysicsMaterialEditorPanel
	{
	public:
		PhysicsMaterialEditorPanel() = default;
		~PhysicsMaterialEditorPanel() = default;

		void OnGuiRender(Entity selectedEntity);
		void SetProjectContext(SharedReference<Project>& project) {}
		void SetSceneContext(SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
