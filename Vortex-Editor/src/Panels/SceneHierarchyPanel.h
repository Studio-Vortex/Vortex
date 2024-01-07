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

		void OnGuiRender(Actor hoveredActor, const EditorCamera* editorCamera);
		void OnGuiRender() override {}
		void SetSceneContext(SharedReference<Scene> scene) override;

		inline bool IsFocusedOnActorName() const { return m_IsEditingActorName; }
		void FocusOnActorName(bool shouldFocus);

		void DisplayCreateActorMenu(const EditorCamera* editorCamera, Actor parent = {});

		inline bool& IsInspectorOpen() { return s_ShowInspectorPanel; }

		EDITOR_PANEL_TYPE(SceneHierarchy)

	private:
		void RenderSceneHierarchy(Actor hoveredActor, const EditorCamera* editorCamera);
		void RenderInsectorPanel(Actor hoveredActor);

		template <typename TComponent>
		void DisplayAddComponentMenuItem(const std::string& name, const char* icon, bool drawUnderline = true)
		{
			Actor& selectedActor = SelectionManager::GetSelectedActor();

			if (!selectedActor.HasComponent<TComponent>())
			{
				Gui::Text(icon);
				Gui::SameLine();
				Gui::AlignTextToFramePadding();

				if (Gui::MenuItem(name.c_str()) || (Gui::IsItemFocused() && Gui::IsKeyPressed(ImGuiKey_Enter)))
				{
					selectedActor.AddComponent<TComponent>();
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

		void DrawActorNode(Actor actor, const EditorCamera* editorCamera);
		void DrawComponents(Actor actor);

		void RecursiveActorSearch(UUID rootActor, const EditorCamera* editorCamera, uint32_t& searchDepth);

	private:
		void TagComponentOnGuiRender(TagComponent& component, Actor actor);
		void TransformComponentOnGuiRender(TransformComponent& component, Actor actor);
		void CameraComponentOnGuiRender(CameraComponent& component, Actor actor);
		void SkyboxComponentOnGuiRender(SkyboxComponent& component, Actor actor);
		void LightSourceComponentOnGuiRender(LightSourceComponent& component, Actor actor);
		void MeshRendererComponentOnGuiRender(MeshRendererComponent& component, Actor actor);
		void StaticMeshRendererComponentOnGuiRender(StaticMeshRendererComponent& component, Actor actor);
		void SpriteRendererComponentOnGuiRender(SpriteRendererComponent& component, Actor actor);
		void CircleRendererComponentOnGuiRender(CircleRendererComponent& component, Actor actor);
		void ParticleEmitterComponentOnGuiRender(ParticleEmitterComponent& component, Actor actor);
		void AnimatorComponentOnGuiRender(AnimatorComponent& component, Actor actor);
		void AnimationComponentOnGuiRender(AnimationComponent& component, Actor actor);
		void TextMeshComponentOnGuiRender(TextMeshComponent& component, Actor actor);
		void ButtonComponentOnGuiRender(ButtonComponent& component, Actor actor);
		void AudioSourceComponentOnGuiRender(AudioSourceComponent& component, Actor actor);
		void AudioListenerComponentOnGuiRender(AudioListenerComponent& component, Actor actor);
		void RigidBodyComponentOnGuiRender(RigidBodyComponent& component, Actor actor);
		void CharacterControllerComponentOnGuiRender(CharacterControllerComponent& component, Actor actor);
		void FixedJointComponentOnGuiRender(FixedJointComponent& component, Actor actor);
		void BoxColliderComponentOnGuiRender(BoxColliderComponent& component, Actor actor);
		void SphereColliderComponentOnGuiRender(SphereColliderComponent& component, Actor actor);
		void CapsuleColliderComponentOnGuiRender(CapsuleColliderComponent& component, Actor actor);
		void MeshColliderComponentOnGuiRender(MeshColliderComponent& component, Actor actor);
		void RigidBody2DComponentOnGuiRender(RigidBody2DComponent& component, Actor actor);
		void BoxCollider2DComponentOnGuiRender(BoxCollider2DComponent& component, Actor actor);
		void CircleCollider2DComponentOnGuiRender(CircleCollider2DComponent& component, Actor actor);
		void NavMeshAgentComponentOnGuiRender(NavMeshAgentComponent& component, Actor actor);
		void ScriptComponentOnGuiRender(ScriptComponent& component, Actor actor);

	private:
		inline static bool s_ShowInspectorPanel = true;

	private:
		SharedReference<Scene> m_ContextScene = nullptr;
		
		SharedReference<Scene> m_CopyScene = nullptr;
		Actor m_CopyActor = {};

		ImGuiTextFilter m_ActorSearchInputTextFilter;
		ImGuiTextFilter m_ComponentSearchInputTextFilter;
		ImGuiTextFilter m_ActorClassNameInputTextFilter;
		ImGuiTextFilter m_ActorFieldSearchInputTextFilter;
		ImGuiTextFilter m_MaterialSearchInputTextFilter;

		bool m_ActorShouldBeRenamed = false;
		bool m_IsEditingActorName = false;
		bool m_ActorShouldBeDestroyed = false;
		bool m_DisplayAddMarkerPopup = false;
	};

}
