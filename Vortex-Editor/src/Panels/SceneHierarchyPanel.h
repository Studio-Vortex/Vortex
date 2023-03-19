#pragma once

#include <Vortex.h>

namespace Vortex {
	
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const SharedReference<Scene>& context);
		~SceneHierarchyPanel() = default;

		void OnGuiRender(Entity hoveredEntity, const EditorCamera* editorCamera);
		void SetProjectContext(SharedReference<Project>& project) {}
		void SetSceneContext(const SharedReference<Scene>& scene);

		Entity& GetSelectedEntity();

		void SetSelectedEntity(Entity entity);
		void DeselectEntity();

		inline bool GetEntityShouldBeRenamed() const { return m_EntityShouldBeRenamed; }
		inline void EditSelectedEntityName(bool enabled) { m_EntityShouldBeRenamed = enabled; }

		inline void SetEntityToBeDestroyed(bool destroy) { m_EntityShouldBeDestroyed = destroy; }

		void DisplayCreateEntityMenu(const EditorCamera* editorCamera);

		inline bool& IsOpen() { return s_ShowSceneHierarchyPanel; }
		inline bool& IsInspectorOpen() { return s_ShowInspectorPanel; }

	private:
		void DisplayInsectorPanel(Entity hoveredEntity);

		template <typename TComponent>
		void DisplayComponentMenuItem(const std::string& name)
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

		void DisplayAddComponentPopup();

		void DisplayAddMarkerPopup(TagComponent& tagComponent);

		void DrawEntityNode(Entity entity, const EditorCamera* editorCamera);
		void DrawComponents(Entity entity);

		void RecursiveEntitySearch(UUID topEntity, const EditorCamera* editorCamera, uint32_t& searchDepth);

	private:
		inline static bool s_ShowSceneHierarchyPanel = true;
		inline static bool s_ShowInspectorPanel = true;

	private:
		SharedReference<Scene> m_ContextScene = nullptr;
		Entity m_SelectedEntity;
		
		// TODO think of a better way of doing this
		TransformComponent m_TransformToCopy;
		ParticleEmitterProperties m_ParticleEmitterToCopy;

		ImGuiTextFilter m_EntitySearchInputTextFilter;
		ImGuiTextFilter m_ComponentSearchInputTextFilter;
		ImGuiTextFilter m_EntityClassNameInputTextFilter;
		ImGuiTextFilter m_EntityFieldSearchInputTextFilter;
		ImGuiTextFilter m_MaterialSearchInputTextFilter;

		bool m_EntityShouldBeRenamed = false;
		bool m_EntityShouldBeDestroyed = false;
		bool m_DisplayAddMarkerPopup = false;
	};

}
