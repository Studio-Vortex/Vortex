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
		template <typename TComponent>
		void DisplayAddComponentPopup(const std::string& name)
		{
			if (!m_SelectedEntity.HasComponent<TComponent>())
			{
				if (Gui::MenuItem(name.c_str()))
				{
					m_SelectedEntity.AddComponent<TComponent>();
					Gui::CloseCurrentPopup();
				}
			}
		}

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

	private:
		SharedRef<Scene> m_ContextScene;
		Entity m_SelectedEntity;
	};

}
