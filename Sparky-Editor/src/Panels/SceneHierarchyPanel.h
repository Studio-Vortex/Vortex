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

		template <typename TComponent>
		void DrawComponent(const char* name, void (*callback)(Entity))
		{
			if (Gui::TreeNodeEx((void*)typeid(TComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, name))
			{
				callback(m_SelectedEntity);
				Gui::TreePop();
			}
		}

	private:
		SharedRef<Scene> m_ContextScene;
		Entity m_SelectedEntity;
	};

}
