#pragma once

#include <Vortex.h>
#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {
	
	class SceneHierarchyPanel : public EditorPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const SharedRef<Scene>& context);
		~SceneHierarchyPanel() override = default;

		void OnGuiRender(Entity hoveredEntity, const EditorCamera* editorCamera);
		void SetProjectContext(SharedRef<Project> project) override {}
		void SetSceneContext(SharedRef<Scene> scene) override;

		inline Entity& GetSelectedEntity() { return m_SelectedEntity; }
		inline const Entity& GetSelectedEntity() const { return m_SelectedEntity; }

		inline void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

		inline bool GetEntityShouldBeRenamed() const { return m_EntityShouldBeRenamed; }
		inline void SetEntityShouldBeRenamed(bool enabled) { m_EntityShouldBeRenamed = enabled; }

		inline void SetEntityToBeDestroyed(bool destroy) { m_EntityShouldBeDestroyed = destroy; }

		void DisplayCreateEntityMenu(const EditorCamera* editorCamera);

		inline bool& IsOpen() { return s_ShowSceneHierarchyPanel; }
		inline bool& IsInspectorOpen() { return s_ShowInspectorPanel; }

	private:
		void DisplayInsectorPanel(Entity hoveredEntity);

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

		void DisplayAddMarkerPopup(TagComponent& tagComponent);

		void DrawEntityNode(Entity entity, const EditorCamera* editorCamera);
		void DrawComponents(Entity entity);

	private:
		inline static bool s_ShowSceneHierarchyPanel = true;
		inline static bool s_ShowInspectorPanel = true;

	private:
		SharedRef<Scene> m_ContextScene = nullptr;
		Entity m_SelectedEntity;
		TransformComponent m_TransformToCopy;
		ImGuiTextFilter m_EntitySearchInputTextFilter;
		ImGuiTextFilter m_ComponentSearchInputTextFilter;
		ImGuiTextFilter m_EntityClassNameInputTextFilter;
		bool m_EntityShouldBeRenamed = false;
		bool m_EntityShouldBeDestroyed = false;
		bool m_DisplayAddMarkerPopup = false;
	};

}
