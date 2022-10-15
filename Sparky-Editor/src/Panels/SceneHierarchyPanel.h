#pragma once

#include <Sparky.h>

namespace Sparky {
	
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const SharedRef<Scene>& context);

		void SetContext(const SharedRef<Scene>& context);

		void OnGuiRender(Entity hoveredEntity);

		Entity& GetSelectedEntity() { return m_SelectedEntity; }
		const Entity& GetSelectedEntity() const { return m_SelectedEntity; }

		void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

		void SetEntityToBeRenamed(bool enabled) { m_EntityShouldBeRenamed = enabled; }
		void SetEntityToBeDestroyed(bool destroy) { m_EntityShouldBeDestroyed = destroy; }

	private:
		template <typename TComponent>
		void DisplayAddComponentPopup(const std::string& name, bool lastComponent = false, bool allowMultiple = false);

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

	private:
		SharedRef<Scene> m_ContextScene;
		Entity m_SelectedEntity;
		bool m_EntityShouldBeRenamed = false;
		bool m_EntityShouldBeDestroyed = false;
	};

	template <typename TComponent>
	void SceneHierarchyPanel::DisplayAddComponentPopup(const std::string& name, bool lastComponent, bool allowMultiple)
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
