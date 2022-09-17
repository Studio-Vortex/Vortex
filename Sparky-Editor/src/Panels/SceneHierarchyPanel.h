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

		Entity& GetSelectedEntity() { return m_SelectedEntity; }
		const Entity& GetSelectedEntity() const { return m_SelectedEntity; }

		void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

	private:
		template <typename TComponent>
		void DisplayAddComponentPopup(const std::string& name, bool allowMultiple = false, bool lastComponent = false);

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

	private:
		SharedRef<Scene> m_ContextScene;
		Entity m_SelectedEntity;
	};

	template <typename TComponent>
	void SceneHierarchyPanel::DisplayAddComponentPopup(const std::string& name, bool allowMultiple, bool lastComponent)
	{
		if (!m_SelectedEntity.HasComponent<TComponent>() || allowMultiple)
		{
			if (Gui::MenuItem(name.c_str()))
			{
				m_SelectedEntity.AddComponent<TComponent>();
				Gui::CloseCurrentPopup();
			}

			if (!lastComponent)
				Gui::Separator();
		}
	}

}
