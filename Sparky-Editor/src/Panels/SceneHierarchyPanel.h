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

		inline Entity& GetSelectedEntity() { return m_SelectedEntity; }
		inline const Entity& GetSelectedEntity() const { return m_SelectedEntity; }

		inline void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

		inline bool GetEntityShouldBeRenamed() const { return m_EntityShouldBeRenamed; }
		inline void SetEntityShouldBeRenamed(bool enabled) { m_EntityShouldBeRenamed = enabled; }

		inline void SetEntityToBeDestroyed(bool destroy) { m_EntityShouldBeDestroyed = destroy; }

		void DisplayCreateEntityMenu();

		inline bool& IsOpen() { return s_ShowSceneHierarchyPanel; }
		inline bool& IsInspectorOpen() { return s_ShowInspectorPanel; }

	private:
		template <typename TComponent>
		void DisplayAddComponentPopup(const std::string& name, bool lastComponent = false, bool allowMultiple = false);

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

	private:
		inline static bool s_ShowSceneHierarchyPanel = true;
		inline static bool s_ShowInspectorPanel = true;

	private:
		SharedRef<Scene> m_ContextScene;
		Entity m_SelectedEntity;
		ImGuiTextFilter m_EntitySearchInputTextFilter;
		ImGuiTextFilter m_ComponentSearchInputTextFilter;
		ImGuiTextFilter m_EntityClassNameInputTextFilter;
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
