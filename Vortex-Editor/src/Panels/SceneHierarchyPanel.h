#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>
#include <Vortex/Editor/SelectionManager.h>

namespace Vortex {
	
	class SceneHierarchyPanel : public EditorPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const SharedReference<Scene>& context);
		~SceneHierarchyPanel() override = default;

		void OnGuiRender(Entity hoveredEntity, const EditorCamera* editorCamera);
		void OnGuiRender() override {}
		void SetSceneContext(SharedReference<Scene> scene) override;

		inline bool IsEditingEntityName() const { return m_IsEditingEntityName; }
		inline void EditSelectedEntityName(bool enabled) { m_EntityShouldBeRenamed = enabled; }

		void DisplayCreateEntityMenu(const EditorCamera* editorCamera);

		inline bool& IsInspectorOpen() { return s_ShowInspectorPanel; }

		EDITOR_PANEL_TYPE(SceneHierarchy)

	private:
		void DisplayInsectorPanel(Entity hoveredEntity);

		template <typename TComponent>
		void DisplayComponentMenuItem(const std::string& name, const char* icon)
		{
			Entity& selectedEntity = SelectionManager::GetSelectedEntity();

			if (!selectedEntity.HasComponent<TComponent>())
			{
				Gui::Text(icon);
				Gui::SameLine();
				Gui::AlignTextToFramePadding();

				if (Gui::MenuItem(name.c_str()))
				{
					selectedEntity.AddComponent<TComponent>();
					Gui::CloseCurrentPopup();
				}

				UI::Draw::Underline();
				Gui::Spacing();
			}
		}

		void DisplayAddComponentPopup();

		void DisplayAddMarkerPopup(TagComponent& tagComponent);

		void DrawEntityNode(Entity entity, const EditorCamera* editorCamera);
		void DrawComponents(Entity entity);

		void RecursiveEntitySearch(UUID topEntity, const EditorCamera* editorCamera, uint32_t& searchDepth);

	private:
		inline static bool s_ShowInspectorPanel = true;

	private:
		SharedReference<Scene> m_ContextScene = nullptr;
		
		// TODO think of a better way of doing this
		TransformComponent m_TransformToCopy;
		ParticleEmitterProperties m_ParticleEmitterToCopy;

		ImGuiTextFilter m_EntitySearchInputTextFilter;
		ImGuiTextFilter m_ComponentSearchInputTextFilter;
		ImGuiTextFilter m_EntityClassNameInputTextFilter;
		ImGuiTextFilter m_EntityFieldSearchInputTextFilter;
		ImGuiTextFilter m_MaterialSearchInputTextFilter;

		bool m_EntityShouldBeRenamed = false;
		bool m_IsEditingEntityName = false;
		bool m_EntityShouldBeDestroyed = false;
		bool m_DisplayAddMarkerPopup = false;
	};

}
