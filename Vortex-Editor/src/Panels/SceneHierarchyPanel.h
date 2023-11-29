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
		void DisplayAddComponentMenuItem(const std::string& name, const char* icon, bool drawUnderline = true)
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

				if (drawUnderline)
				{
					UI::Draw::Underline();
					Gui::Spacing();
				}
			}
		}

		void DisplayAddComponentPopup();

		void DisplayAddMarkerPopup(TagComponent& tagComponent);

		void DrawEntityNode(Entity entity, const EditorCamera* editorCamera);
		void DrawComponents(Entity entity);

		void RecursiveEntitySearch(UUID topEntity, const EditorCamera* editorCamera, uint32_t& searchDepth);

	private:
		void TransformComponentOnGuiRender(TransformComponent& component, Entity entity);
		void CameraComponentOnGuiRender(CameraComponent& component, Entity entity);
		void SkyboxComponentOnGuiRender(SkyboxComponent& component, Entity entity);
		void LightSourceComponentOnGuiRender(LightSourceComponent& component, Entity entity);
		void MeshRendererComponentOnGuiRender(MeshRendererComponent& component, Entity entity);
		void StaticMeshRendererComponentOnGuiRender(StaticMeshRendererComponent& component, Entity entity);
		void SpriteRendererComponentOnGuiRender(SpriteRendererComponent& component, Entity entity);
		void CircleRendererComponentOnGuiRender(CircleRendererComponent& component, Entity entity);
		void ParticleEmitterComponentOnGuiRender(ParticleEmitterComponent& component, Entity entity);
		void TextMeshComponentOnGuiRender(TextMeshComponent& component, Entity entity);
		void AnimatorComponentOnGuiRender(AnimatorComponent& component, Entity entity);
		void AnimationComponentOnGuiRender(AnimationComponent& component, Entity entity);
		void AudioSourceComponentOnGuiRender(AudioSourceComponent& component, Entity entity);
		void AudioListenerComponentOnGuiRender(AudioListenerComponent& component, Entity entity);
		void RigidBodyComponentOnGuiRender(RigidBodyComponent& component, Entity entity);
		void CharacterControllerComponentOnGuiRender(CharacterControllerComponent& component, Entity entity);
		void FixedJointComponentOnGuiRender(FixedJointComponent& component, Entity entity);
		void BoxColliderComponentOnGuiRender(BoxColliderComponent& component, Entity entity);
		void SphereColliderComponentOnGuiRender(SphereColliderComponent& component, Entity entity);
		void CapsuleColliderComponentOnGuiRender(CapsuleColliderComponent& component, Entity entity);
		void MeshColliderComponentOnGuiRender(MeshColliderComponent& component, Entity entity);
		void RigidBody2DComponentOnGuiRender(RigidBody2DComponent& component, Entity entity);
		void BoxCollider2DComponentOnGuiRender(BoxCollider2DComponent& component, Entity entity);
		void CircleCollider2DComponentOnGuiRender(CircleCollider2DComponent& component, Entity entity);
		void NavMeshAgentComponentOnGuiRender(NavMeshAgentComponent& component, Entity entity);
		void ScriptComponentOnGuiRender(ScriptComponent& component, Entity entity);

	private:
		inline static bool s_ShowInspectorPanel = true;

	private:
		SharedReference<Scene> m_ContextScene = nullptr;
		
		SharedReference<Scene> m_CopyScene = nullptr;
		Entity m_CopyEntity = {};

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
