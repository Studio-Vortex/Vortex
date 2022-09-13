#pragma once

#include <Sparky.h>

namespace Sparky {
	
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const SharedRef<Scene>& context);

		void SetContext(const SharedRef<Scene>& context);

		void OnGuiRender();

	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

	private:
		SharedRef<Scene> m_ContextScene;
		Entity m_SelectedEntity;
	};

}
