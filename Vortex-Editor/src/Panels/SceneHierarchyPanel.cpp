#include "SceneHierarchyPanel.h"

#include <Vortex/Core/Buffer.h>

#include <Vortex/Audio/AudioUtils.h>

#include <Vortex/Scripting/ScriptEngine.h>

#include <Vortex/Editor/EditorResources.h>

#include <imgui_internal.h>

#include <codecvt>

namespace Vortex {

#define ACTOR_TAG_BUFFER_SIZE 256
#define ACTOR_MAX_MARKER_SIZE 64
#define ACTOR_MAX_CHILD_ACTOR_SEARCH_DEPTH 10

#define INSPECTOR_PANEL_NAME "Inspector"

	SceneHierarchyPanel::SceneHierarchyPanel()
	{
		// Create copy actor here so it will live the lifetime of the scene hierarchy panel
		// This will allow us to copy components to and from different scenes
		m_CopyScene = Scene::Create();
		m_CopyActor = m_CopyScene->CreateActor("Copy Actor");
	}

	void SceneHierarchyPanel::OnGuiRender(Actor hoveredActor, const EditorCamera* editorCamera)
	{
		if (IsOpen)
		{
			RenderSceneHierarchy(hoveredActor, editorCamera);
		}

		if (s_ShowInspectorPanel)
		{
			RenderInsectorPanel(hoveredActor);
		}
	}

	void SceneHierarchyPanel::RecursiveActorSearch(UUID rootActor, const EditorCamera* editorCamera, uint32_t& searchDepth)
	{
		if (searchDepth > ACTOR_MAX_CHILD_ACTOR_SEARCH_DEPTH)
			return;

		const Actor actor = m_ContextScene->TryGetActorWithUUID(rootActor);

		if (!actor || actor.Children().empty())
			return;

		const std::vector<UUID>& children = actor.Children();

		for (const auto& childUUID : children)
		{
			const Actor child = m_ContextScene->TryGetActorWithUUID(childUUID);
			if (!child)
				continue;

			const std::string& name = child.Name();

			if (m_ActorSearchInputTextFilter.PassFilter(name.c_str()))
			{
				DrawActorNode(child, editorCamera);
			}

			searchDepth++;

			RecursiveActorSearch(child.GetUUID(), editorCamera, searchDepth);
		}
	}

	bool SceneHierarchyPanel::RecursiveIsPrefabOrAncestorIsPrefab(Actor actor)
	{
		bool isPrefab = actor.HasComponent<PrefabComponent>();

		if (!isPrefab)
		{
			if (!actor.HasParent())
				return false;

			isPrefab = RecursiveIsPrefabOrAncestorIsPrefab(actor.GetParent());
		}

		return isPrefab;
	}

	void SceneHierarchyPanel::SetSceneContext(SharedReference<Scene> scene)
	{
		m_ContextScene = scene;
		SelectionManager::DeselectActor();
		m_ActorShouldBeRenamed = false;
		m_ActorShouldBeDestroyed = false;

		// Clear all search bars
		memset(m_ActorSearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_ActorSearchInputTextFilter.InputBuf));
		m_ActorSearchInputTextFilter.Build();

		memset(m_ActorClassNameInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_ActorClassNameInputTextFilter.InputBuf));
		m_ActorClassNameInputTextFilter.Build();

		memset(m_ComponentSearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
		m_ComponentSearchInputTextFilter.Build();
	}

	inline static Actor CreateDefaultMesh(const std::string& actorName, DefaultMesh::StaticMeshType defaultMesh, SharedReference<Scene>& contextScene, const EditorCamera* editorCamera)
	{
		Actor actor = contextScene->CreateActor(actorName);
		StaticMeshRendererComponent& staticMeshRendererComponent = actor.AddComponent<StaticMeshRendererComponent>();
		staticMeshRendererComponent.Type = static_cast<MeshType>(defaultMesh);
		staticMeshRendererComponent.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(defaultMesh);
		actor.GetTransform().Translation = editorCamera->GetFocalPoint() + editorCamera->GetForwardDirection();

		actor.AddComponent<RigidBodyComponent>();

		switch (defaultMesh)
		{
			case DefaultMesh::StaticMeshType::Cube:     actor.AddComponent<BoxColliderComponent>();     break;
			case DefaultMesh::StaticMeshType::Sphere:   actor.AddComponent<SphereColliderComponent>();  break;
			case DefaultMesh::StaticMeshType::Capsule:  actor.AddComponent<CapsuleColliderComponent>(); break;
			case DefaultMesh::StaticMeshType::Cone:     actor.AddComponent<MeshColliderComponent>();    break;
			case DefaultMesh::StaticMeshType::Cylinder: actor.AddComponent<MeshColliderComponent>();    break;
			case DefaultMesh::StaticMeshType::Plane:    actor.AddComponent<MeshColliderComponent>();    break;
			case DefaultMesh::StaticMeshType::Torus:    actor.AddComponent<MeshColliderComponent>();    break;
		}

		return actor;
	}

    void SceneHierarchyPanel::FocusOnActorName(bool shouldFocus)
    {
		m_ActorShouldBeRenamed = shouldFocus;

		if (!m_ActorShouldBeRenamed)
		{
			return;
		}

		if (!IsInspectorOpen())
		{
			s_ShowInspectorPanel = true;
		}
		Gui::SetWindowFocus(INSPECTOR_PANEL_NAME);
    }

    void SceneHierarchyPanel::DisplayCreateActorMenu(const EditorCamera* editorCamera, Actor parent)
	{
		Actor actor;

		auto separator = []() {
			UI::Draw::Underline();
			Gui::Spacing();
		};

		const Math::vec3 relativeToEditorCamera = editorCamera->GetFocalPoint() + editorCamera->GetForwardDirection();

		if (Gui::MenuItem("Create Empty"))
		{
			actor = m_ContextScene->CreateActor("Empty Actor");
			actor.GetTransform().Translation = relativeToEditorCamera;
		}
		separator();
		
		Gui::Text((const char*)VX_ICON_CUBE);
		Gui::SameLine();
		if (Gui::BeginMenu("Create 3D"))
		{
			if (Gui::MenuItem("Cube"))
			{
				actor = CreateDefaultMesh("Cube", DefaultMesh::StaticMeshType::Cube, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Sphere"))
			{
				actor = CreateDefaultMesh("Sphere", DefaultMesh::StaticMeshType::Sphere, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Capsule"))
			{
				actor = CreateDefaultMesh("Capsule", DefaultMesh::StaticMeshType::Capsule, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Cone"))
			{
				actor = CreateDefaultMesh("Cone", DefaultMesh::StaticMeshType::Cone, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Cylinder"))
			{
				actor = CreateDefaultMesh("Cylinder", DefaultMesh::StaticMeshType::Cylinder, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Plane"))
			{
				actor = CreateDefaultMesh("Plane", DefaultMesh::StaticMeshType::Plane, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Torus"))
			{
				actor = CreateDefaultMesh("Torus", DefaultMesh::StaticMeshType::Torus, m_ContextScene, editorCamera);
			}

			Gui::EndMenu();
		}
		separator();

		Gui::Text((const char*)VX_ICON_SPINNER);
		Gui::SameLine();
		if (Gui::BeginMenu("Create Sprite"))
		{
			if (Gui::MenuItem("Sprite"))
			{
				actor = m_ContextScene->CreateActor("Sprite");
				actor.GetTransform().Translation = relativeToEditorCamera;
				actor.GetTransform().Translation.z = 0.0f;
				actor.AddComponent<SpriteRendererComponent>();
				actor.AddComponent<RigidBody2DComponent>();
				actor.AddComponent<BoxCollider2DComponent>();
			}
			separator();

			if (Gui::MenuItem("Circle"))
			{
				actor = m_ContextScene->CreateActor("Circle");
				actor.GetTransform().Translation = relativeToEditorCamera;
				actor.GetTransform().Translation.z = 0.0f;
				actor.AddComponent<CircleRendererComponent>();
				actor.AddComponent<RigidBody2DComponent>();
				actor.AddComponent<CircleCollider2DComponent>();
			}

			Gui::EndMenu();
		}
		separator();

		Gui::Text((const char*)VX_ICON_VIDEO_CAMERA);
		Gui::SameLine();
		if (Gui::BeginMenu("Camera"))
		{
			if (Gui::MenuItem("Perspective"))
			{
				actor = m_ContextScene->CreateActor("Camera");
				CameraComponent& cameraComponent = actor.AddComponent<CameraComponent>();
				cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
				actor.GetTransform().Translation = relativeToEditorCamera;
			}
			separator();

			if (Gui::MenuItem("Orthographic"))
			{
				actor = m_ContextScene->CreateActor("Camera");
				CameraComponent& cameraComponent = actor.AddComponent<CameraComponent>();
				cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
				actor.GetTransform().Translation = relativeToEditorCamera;
			}

			Gui::EndMenu();
		}
		separator();

		Gui::Text((const char*)VX_ICON_LIGHTBULB_O);
		Gui::SameLine();
		if (Gui::BeginMenu("Light"))
		{
			if (Gui::MenuItem("Directional"))
			{
				actor = m_ContextScene->CreateActor("Directional Light");
				LightSourceComponent& lightSourceComponent = actor.AddComponent<LightSourceComponent>();
				lightSourceComponent.Type = LightType::Directional;
				actor.GetTransform().Translation = relativeToEditorCamera;
			}
			separator();

			if (Gui::MenuItem("Point"))
			{
				actor = m_ContextScene->CreateActor("Point Light");
				LightSourceComponent& lightSourceComponent = actor.AddComponent<LightSourceComponent>();
				lightSourceComponent.Type = LightType::Point;
				actor.GetTransform().Translation = relativeToEditorCamera;
			}
			separator();

			if (Gui::MenuItem("Spot"))
			{
				actor = m_ContextScene->CreateActor("Spot Light");
				LightSourceComponent& lightSourceComponent = actor.AddComponent<LightSourceComponent>();
				lightSourceComponent.Type = LightType::Spot;
				actor.GetTransform().Translation = relativeToEditorCamera;
			}

			Gui::EndMenu();
		}
		separator();

		Gui::Text((const char*)VX_ICON_CALCULATOR);
		Gui::SameLine();
		if (Gui::BeginMenu("Physics"))
		{
			if (Gui::MenuItem("Box Collider"))
			{
				actor = CreateDefaultMesh("Box Collider", DefaultMesh::StaticMeshType::Cube, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Sphere Collider"))
			{
				actor = CreateDefaultMesh("Sphere Collider", DefaultMesh::StaticMeshType::Sphere, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Capsule Collider"))
			{
				actor = CreateDefaultMesh("Capsule Collider", DefaultMesh::StaticMeshType::Capsule, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Mesh Collider"))
			{
				actor = CreateDefaultMesh("Mesh Collider", DefaultMesh::StaticMeshType::Cube, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Trigger Box"))
			{
				actor = m_ContextScene->CreateActor("Trigger Box");
				actor.AddComponent<RigidBodyComponent>();
				BoxColliderComponent& boxCollider = actor.AddComponent<BoxColliderComponent>();
				boxCollider.Visible = true;
				boxCollider.IsTrigger = true;
				actor.GetTransform().Translation = relativeToEditorCamera;
			}
			separator();

			if (Gui::MenuItem("Trigger Sphere"))
			{
				actor = m_ContextScene->CreateActor("Trigger Sphere");
				actor.AddComponent<RigidBodyComponent>();
				SphereColliderComponent& sphereCollider = actor.AddComponent<SphereColliderComponent>();
				sphereCollider.Visible = true;
				sphereCollider.IsTrigger = true;
				actor.GetTransform().Translation = relativeToEditorCamera;
			}
			separator();

			if (Gui::MenuItem("Trigger Capsule"))
			{
				actor = m_ContextScene->CreateActor("Trigger Capsule");
				actor.AddComponent<RigidBodyComponent>();
				CapsuleColliderComponent& capsuleCollider = actor.AddComponent<CapsuleColliderComponent>();
				capsuleCollider.Visible = true;
				capsuleCollider.IsTrigger = true;
				actor.GetTransform().Translation = relativeToEditorCamera;
			}
			separator();

			if (Gui::MenuItem("Fixed Joint"))
			{
				actor = CreateDefaultMesh("Fixed Joint", DefaultMesh::StaticMeshType::Cube, m_ContextScene, editorCamera);
			}
			separator();

			if (Gui::MenuItem("Box Collider 2D"))
			{
				actor = m_ContextScene->CreateActor("Box Collider2D");
				actor.AddComponent<SpriteRendererComponent>();
				actor.AddComponent<RigidBody2DComponent>();
				actor.AddComponent<BoxCollider2DComponent>();
				actor.GetTransform().Translation = relativeToEditorCamera;
			}
			separator();

			if (Gui::MenuItem("Circle Collider 2D"))
			{
				actor = m_ContextScene->CreateActor("Circle Collider2D");
				actor.AddComponent<CircleRendererComponent>();
				actor.AddComponent<RigidBody2DComponent>();
				actor.AddComponent<CircleCollider2DComponent>();
				actor.GetTransform().Translation = relativeToEditorCamera;
			}

			Gui::EndMenu();
		}
		separator();

		Gui::Text((const char*)VX_ICON_VOLUME_UP);
		Gui::SameLine();
		if (Gui::BeginMenu("Audio"))
		{
			if (Gui::MenuItem("Source Actor"))
			{
				actor = m_ContextScene->CreateActor("Audio Source");
				actor.AddComponent<AudioSourceComponent>();
				actor.GetTransform().Translation = relativeToEditorCamera;
			}
			separator();

			if (Gui::MenuItem("Listener Actor"))
			{
				actor = m_ContextScene->CreateActor("Audio Listener");
				actor.AddComponent<AudioListenerComponent>();
				actor.GetTransform().Translation = relativeToEditorCamera;
			}

			Gui::EndMenu();
		}
		separator();

		Gui::Text((const char*)VX_ICON_FONT);
		Gui::SameLine();
		if (Gui::BeginMenu("UI"))
		{
			if (Gui::MenuItem("Text"))
			{
				actor = m_ContextScene->CreateActor("UI Text");
				actor.AddComponent<TextMeshComponent>();
				actor.GetTransform().Translation = relativeToEditorCamera;
			}
			separator();

			if (Gui::MenuItem("Button"))
			{
				actor = m_ContextScene->CreateActor("UI Button");
				actor.AddComponent<ButtonComponent>();
				actor.GetTransform().Translation = relativeToEditorCamera;
			}

			Gui::EndMenu();
		}
		separator();

		Gui::Text((const char*)VX_ICON_BOMB);
		Gui::SameLine();
		if (Gui::BeginMenu("Effects"))
		{
			if (Gui::MenuItem("Particles"))
			{
				actor = m_ContextScene->CreateActor("Particle Emitter");
				actor.AddComponent<ParticleEmitterComponent>();
				actor.GetTransform().Translation = relativeToEditorCamera;
			}

			Gui::EndMenu();
		}

		if (!actor)
		{
			return;
		}

		// Parent the child
		if (parent)
		{
			Actor child = actor;
			m_ContextScene->ParentActor(child, parent);

			// activate parent tree node
			// TODO this doesn't work
			const ImGuiID parentActorTreeNodeID = Gui::GetID((void*)(uint32_t)parent);
			Gui::ActivateItem(parentActorTreeNodeID);
		}

		SelectionManager::SetSelectedActor(actor);
		FocusOnActorName(true);
	}

	void SceneHierarchyPanel::RenderSceneHierarchy(Actor hoveredActor, const EditorCamera* editorCamera)
	{
		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		ImRect windowRect = ImRect(Gui::GetWindowContentRegionMin(), Gui::GetWindowContentRegionMax());

		// Search Bar + Filtering
		Gui::SetNextItemWidth(Gui::GetContentRegionAvail().x - Gui::CalcTextSize((const char*)VX_ICON_PLUS).x * 2.0f - 4.0f);
		const bool isSearching = Gui::InputTextWithHint("##ActorSearch", "Search...", m_ActorSearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_ActorSearchInputTextFilter.InputBuf));
		if (isSearching)
		{
			m_ActorSearchInputTextFilter.Build();
		}

		UI::DrawItemActivityOutline();

		Gui::SameLine();

		UI::ShiftCursorX(-4.0f);
		if (Gui::Button((const char*)VX_ICON_PLUS, { 30.0f, 0.0f }))
		{
			Gui::OpenPopup("CreateActor");
		}

		if (Gui::BeginPopup("CreateActor"))
		{
			DisplayCreateActorMenu(editorCamera);

			Gui::EndPopup();
		}

		Gui::Spacing();
		UI::Draw::Underline();

		if (m_ContextScene)
		{
			uint32_t searchDepth = 0;
			const bool isSearching = strlen(m_ActorSearchInputTextFilter.InputBuf) != 0;
			std::vector<UUID> rootActorsInHierarchy;

			m_ContextScene->m_Registry.each([&](auto actorID)
			{
				Actor actor{ actorID, m_ContextScene.Raw() };

				if (!actor)
					return;

				const bool isChild = actor.HasParent();

				if (isChild)
					return;

				rootActorsInHierarchy.push_back(actor.GetUUID());

				const bool matchingSearch = m_ActorSearchInputTextFilter.PassFilter(actor.Name().c_str());

				if (!matchingSearch)
					return;

				DrawActorNode(actor, editorCamera);
			});

			if (isSearching)
			{
				for (const auto& rootActor : rootActorsInHierarchy)
				{
					RecursiveActorSearch(rootActor, editorCamera, searchDepth);
				}
			}

			if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
			{
				const auto flags = ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM", flags);

				if (payload)
				{
					Actor& actor = *(Actor*)payload->Data;
					m_ContextScene->UnparentActor(actor);
				}

				ImGui::EndDragDropTarget();
			}

			// Left click anywhere on the panel to deselect actor
			if (Gui::IsMouseDown(0) && Gui::IsWindowHovered())
			{
				SelectionManager::DeselectActor();
				m_ActorShouldBeRenamed = false;
				m_ActorShouldBeDestroyed = false;
			}

			// Right-click on blank space in scene hierarchy panel
			if (Gui::BeginPopupContextWindow(0, 1, false))
			{
				DisplayCreateActorMenu(editorCamera);

				Gui::EndPopup();
			}

			Actor selected = SelectionManager::GetSelectedActor();

			// destroy if requested
			if (m_ActorShouldBeDestroyed && selected)
			{
				Actor actor = selected;

				SelectionManager::DeselectActor();
				m_ActorShouldBeRenamed = false;
				m_ActorShouldBeDestroyed = false;

				m_ContextScene->SubmitToDestroyActor(actor);
			}
		}

		Gui::End();
	}

	void SceneHierarchyPanel::RenderInsectorPanel(Actor hoveredActor)
	{
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

		if (Gui::Begin(INSPECTOR_PANEL_NAME, &s_ShowInspectorPanel, flags))
		{
			if (Actor selected = SelectionManager::GetSelectedActor())
			{
				DrawComponents(selected);
			}
			else
			{
				std::string name = "(null)";

				if (m_ContextScene && hoveredActor && hoveredActor.HasComponent<TagComponent>())
				{
					const std::string& tag = hoveredActor.GetComponent<TagComponent>().Tag;

					if (!tag.empty())
					{
						name = tag;
					}
				}

				Gui::SetCursorPosX(10.0f);
				Gui::Text("Hovered Actor: %s", name.c_str());
			}
		}

		Gui::End();
	}

	void SceneHierarchyPanel::DisplayAddComponentPopup()
	{
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;

		if (Gui::BeginPopup("AddComponent", flags))
		{
			// Search Bar + Filtering
			const bool isSearching = Gui::InputTextWithHint("##ComponentSearch", "Search...", m_ComponentSearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
			const bool searchBarInUse = strlen(m_ComponentSearchInputTextFilter.InputBuf) != 0;

			if (isSearching)
			{
				m_ComponentSearchInputTextFilter.Build();
			}

			Gui::Spacing();
			UI::Draw::Underline();

			if (searchBarInUse)
			{
				if (const char* componentName = "Camera"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<CameraComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Skybox"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<SkyboxComponent>(componentName, (const char*)VX_ICON_SKYATLAS);
				if (const char* componentName = "Light Source"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<LightSourceComponent>(componentName, (const char*)VX_ICON_LIGHTBULB_O);
				if (const char* componentName = "Mesh Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<MeshRendererComponent>(componentName, (const char*)VX_ICON_HOME);
				if (const char* componentName = "Static Mesh Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<StaticMeshRendererComponent>(componentName, (const char*)VX_ICON_CUBE);
				if (const char* componentName = "Light Source 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<LightSource2DComponent>(componentName, (const char*)VX_ICON_LIGHTBULB_O);
				if (const char* componentName = "Sprite Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<SpriteRendererComponent>(componentName, (const char*)VX_ICON_SPINNER);
				if (const char* componentName = "Circle Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<CircleRendererComponent>(componentName, (const char*)VX_ICON_CIRCLE);
				if (const char* componentName = "Particle Emitter"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<ParticleEmitterComponent>(componentName, (const char*)VX_ICON_BOMB);
				if (const char* componentName = "Animator"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<AnimatorComponent>(componentName, (const char*)VX_ICON_CLOCK_O);
				if (const char* componentName = "Animation"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<AnimationComponent>(componentName, (const char*)VX_ICON_ADJUST);
				if (const char* componentName = "Text Mesh"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<TextMeshComponent>(componentName, (const char*)VX_ICON_TEXT_HEIGHT);
				if (const char* componentName = "Button"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<ButtonComponent>(componentName, (const char*)VX_ICON_TEXT_WIDTH);
				if (const char* componentName = "RigidBody"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<RigidBodyComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Character Controller"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<CharacterControllerComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Fixed Joint"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<FixedJointComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Box Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<BoxColliderComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Sphere Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<SphereColliderComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Capsule Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<CapsuleColliderComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Mesh Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<MeshColliderComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Audio Source"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<AudioSourceComponent>(componentName, (const char*)VX_ICON_VOLUME_UP);
				if (const char* componentName = "Audio Listener"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<AudioListenerComponent>(componentName, (const char*)VX_ICON_HEADPHONES);
				if (const char* componentName = "RigidBody 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<RigidBody2DComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Box Collider 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<BoxCollider2DComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Circle Collider 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<CircleCollider2DComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Nav Mesh Agent"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<NavMeshAgentComponent>(componentName, (const char*)VX_ICON_LAPTOP);
				if (const char* componentName = "Script"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<ScriptComponent>(componentName, (const char*)VX_ICON_FILE_CODE_O, false);
			}
			else
			{
				if (Gui::BeginMenu("Rendering"))
				{
					DisplayAddComponentMenuItem<CameraComponent>("Camera", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<SkyboxComponent>("Skybox", (const char*)VX_ICON_SKYATLAS);
					DisplayAddComponentMenuItem<LightSourceComponent>("Light Source", (const char*)VX_ICON_LIGHTBULB_O);
					DisplayAddComponentMenuItem<MeshRendererComponent>("Mesh Renderer", (const char*)VX_ICON_HOME);
					DisplayAddComponentMenuItem<StaticMeshRendererComponent>("Static Mesh Renderer", (const char*)VX_ICON_CUBE);
					DisplayAddComponentMenuItem<LightSource2DComponent>("Light Source 2D", (const char*)VX_ICON_LIGHTBULB_O);
					DisplayAddComponentMenuItem<SpriteRendererComponent>("Sprite Renderer", (const char*)VX_ICON_SPINNER);
					DisplayAddComponentMenuItem<CircleRendererComponent>("Circle Renderer", (const char*)VX_ICON_CIRCLE);
					DisplayAddComponentMenuItem<ParticleEmitterComponent>("Particle Emitter", (const char*)VX_ICON_BOMB);
					DisplayAddComponentMenuItem<AnimatorComponent>("Animator", (const char*)VX_ICON_CLOCK_O);
					DisplayAddComponentMenuItem<AnimationComponent>("Animation", (const char*)VX_ICON_ADJUST, false);

					Gui::EndMenu();
				}
				UI::Draw::Underline();
				Gui::Spacing();

				if (Gui::BeginMenu("UI"))
				{
					DisplayAddComponentMenuItem<TextMeshComponent>("Text Mesh", (const char*)VX_ICON_TEXT_HEIGHT);
					DisplayAddComponentMenuItem<ButtonComponent>("Button", (const char*)VX_ICON_TEXT_WIDTH, false);

					Gui::EndMenu();
				}
				UI::Draw::Underline();
				Gui::Spacing();

				if (Gui::BeginMenu("Physics"))
				{
					DisplayAddComponentMenuItem<RigidBodyComponent>("RigidBody", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<CharacterControllerComponent>("Character Controller", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<FixedJointComponent>("Fixed Joint", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<BoxColliderComponent>("Box Collider", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<SphereColliderComponent>("Sphere Collider", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<CapsuleColliderComponent>("Capsule Collider", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<MeshColliderComponent>("Mesh Collider", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<RigidBody2DComponent>("RigidBody 2D", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<BoxCollider2DComponent>("Box Collider 2D", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayAddComponentMenuItem<CircleCollider2DComponent>("Circle Collider 2D", (const char*)VX_ICON_VIDEO_CAMERA, false);

					Gui::EndMenu();
				}
				UI::Draw::Underline();
				Gui::Spacing();

				if (Gui::BeginMenu("Audio"))
				{
					DisplayAddComponentMenuItem<AudioSourceComponent>("Audio Source", (const char*)VX_ICON_VOLUME_UP);
					DisplayAddComponentMenuItem<AudioListenerComponent>("Audio Listener", (const char*)VX_ICON_HEADPHONES, false);

					Gui::EndMenu();
				}
				UI::Draw::Underline();
				Gui::Spacing();

				if (Gui::BeginMenu("AI"))
				{
					DisplayAddComponentMenuItem<NavMeshAgentComponent>("Nav Mesh Agent", (const char*)VX_ICON_LAPTOP, false);

					Gui::EndMenu();
				}
				UI::Draw::Underline();
				Gui::Spacing();

				DisplayAddComponentMenuItem<ScriptComponent>("Script", (const char*)VX_ICON_FILE_CODE_O, false);
			}

			Gui::EndPopup();
		}
	}

	void SceneHierarchyPanel::DisplayAddMarkerPopup(TagComponent& tagComponent)
	{
		Gui::Spacing();
		Gui::TextCentered("New Marker", 5.0f);
		UI::Draw::Underline();
		Gui::Spacing();

		std::string tempBuffer;
		std::string& actorMarker = tagComponent.Marker;
		size_t markerSize = actorMarker.size();

		if (actorMarker.empty())
		{
			tempBuffer.reserve(ACTOR_MAX_MARKER_SIZE);
		}
		else
		{
			tempBuffer.resize(markerSize);
			memcpy(tempBuffer.data(), actorMarker.data(), markerSize);
		}

		auto OnMarkerAddedFn = [&]() {
			if (tempBuffer.empty())
			{
				return;
			}

			tagComponent.Marker = tempBuffer;
			TagComponent::AddMarker(tempBuffer);
			Gui::SetWindowFocus("Scene");
		};

		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

		if (Gui::InputText("##Marker", tempBuffer.data(), sizeof(tempBuffer), flags))
		{
			std::invoke(OnMarkerAddedFn);
		}

		UI::DrawItemActivityOutline();
	}

	void SceneHierarchyPanel::DrawActorNode(Actor actor, const EditorCamera* editorCamera)
	{
		const TagComponent& tagComponent = actor.GetComponent<TagComponent>();
		const std::string& tag = tagComponent.Tag;

		ImGuiTreeNodeFlags flags = ((SelectionManager::GetSelectedActor() == actor) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (actor.Children().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;

		const bool isPrefab = actor.HasComponent<PrefabComponent>();
		const bool actorInactive = !actor.IsActive();
		const bool ancestorIsPrefab = actor.HasParent() && RecursiveIsPrefabOrAncestorIsPrefab(actor.GetParent());

		if (isPrefab || ancestorIsPrefab)
			Gui::PushStyleColor(ImGuiCol_Text, ImVec4(0.32f, 0.7f, 0.87f, 1.0f));
		if (actorInactive)
			Gui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

		const bool opened = Gui::TreeNodeEx((void*)(uint32_t)actor, flags, tag.c_str());
		
		if (isPrefab || ancestorIsPrefab)
			Gui::PopStyleColor();
		if (actorInactive)
			Gui::PopStyleColor();

		// Allow dragging actors
		if (Gui::IsItemHovered() && Gui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			SelectionManager::SetSelectedActor(actor);
			m_ActorShouldBeRenamed = false;
		}

		m_ActorShouldBeDestroyed = false;
		
		// Right-click on actor for utilities popup
		if (Gui::BeginPopupContextItem())
		{
			auto separator = []() {
				UI::Draw::Underline();
				Gui::Spacing();
			};

			if (Gui::MenuItem("Rename", "F2"))
			{
				SelectionManager::SetSelectedActor(actor);
				FocusOnActorName(true);
				Gui::CloseCurrentPopup();
			}
			separator();

			if (Gui::BeginMenu("Add Child Actor"))
			{
				Actor parent = SelectionManager::GetSelectedActor();
				DisplayCreateActorMenu(editorCamera, parent);
				Gui::EndMenu();
			}
			separator();

			if (Gui::MenuItem("Unparent Actor"))
			{
				SelectionManager::SetSelectedActor(actor);
				m_ContextScene->UnparentActor(actor);
				Gui::CloseCurrentPopup();
			}
			separator();

			if (isPrefab)
			{
				if (Gui::MenuItem("Update Prefab"))
				{
					AssetHandle prefabHandle = actor.GetComponent<PrefabComponent>().Prefab;
					SharedReference<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabHandle);
					if (prefab)
						prefab->Create(actor);
					else
						VX_CONSOLE_LOG_ERROR("Prefab has invalid asset handle: {}", prefabHandle);
				}
				separator();
			}

			if (Gui::MenuItem("Duplicate Actor", "Ctrl+D"))
			{
				Actor duplicate = m_ContextScene->DuplicateActor(actor);
				SelectionManager::SetSelectedActor(duplicate);
				FocusOnActorName(true);
				Gui::CloseCurrentPopup();
			}
			separator();

			if (Gui::MenuItem("Delete Actor", "Del") && SelectionManager::GetSelectedActor())
			{
				m_ActorShouldBeDestroyed = true;
			}

			Gui::EndPopup();
		}

		if (Gui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Gui::Text(actor.Name().c_str());
			Gui::SetDragDropPayload("SCENE_HIERARCHY_ITEM", &actor, sizeof(Actor));
			Gui::EndDragDropSource();
		}

		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM", ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
			{
				Actor& droppedActor = *((Actor*)payload->Data);
				m_ContextScene->ParentActor(droppedActor, actor);
			}

			Gui::EndDragDropTarget();
		}

		if (opened)
		{
			const std::vector<UUID>& children = actor.Children();
			for (const auto& child : children)
			{
				Actor childActor = m_ContextScene->TryGetActorWithUUID(child);
				if (!childActor)
					continue;

				DrawActorNode(childActor, editorCamera);
			}

			Gui::TreePop();
		}

		// Destroy the actor if requested
		if (m_ActorShouldBeDestroyed && SelectionManager::GetSelectedActor() == actor)
		{
			m_ContextScene->SubmitToDestroyActor(actor);
		}
	}

	template <typename TComponent>
	struct ComponentUICallbacks
	{
		using ValueType = TComponent;
		using ReferenceType = ValueType&;
		using ReferenceFn = std::function<void(ReferenceType, Actor)>;
		using ConstFn = std::function<void(const ReferenceType, Actor)>;

		ReferenceFn OnGuiRenderFn = nullptr;
		ReferenceFn OnComponentResetFn = nullptr;
		ConstFn OnComponentCopiedFn = nullptr;
		ReferenceFn OnComponentPastedFn = nullptr;
		ReferenceFn OnComponentRemovedFn = nullptr;

		bool IsRemoveable = true;
	};

	template <typename TComponent>
	static void DrawComponent(const std::string& name, Actor actor, const ComponentUICallbacks<TComponent>& callbacks)
	{
		if (!actor.HasComponent<TComponent>())
			return;

		auto& component = actor.GetComponent<TComponent>();
		const ImVec2 contentRegionAvailable = Gui::GetContentRegionAvail();

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		UI::Draw::Underline();
		const bool propertyGridHeaderOpen = UI::PropertyGridHeader(name.c_str());
		Gui::PopStyleVar();
		Gui::SameLine(contentRegionAvailable.x - lineHeight * 0.6f);
		UI::ShiftCursorY(2.0f);
		if (Gui::Button((const char*)VX_ICON_COG, { lineHeight, lineHeight }))
			Gui::OpenPopup("ComponentSettings");

		bool removeComponent = false;
		if (Gui::BeginPopup("ComponentSettings"))
		{
			Gui::BeginDisabled(callbacks.OnComponentCopiedFn == nullptr);
			if (Gui::MenuItem("Copy Component"))
			{
				if (callbacks.OnComponentCopiedFn != nullptr)
				{
					std::invoke(callbacks.OnComponentCopiedFn, component, actor);
				}
				Gui::CloseCurrentPopup();
			}
			Gui::EndDisabled();

			UI::Draw::Underline();

			Gui::BeginDisabled(callbacks.OnComponentPastedFn == nullptr);
			if (Gui::MenuItem("Paste Component"))
			{
				if (callbacks.OnComponentPastedFn != nullptr)
				{
					std::invoke(callbacks.OnComponentPastedFn, component, actor);
				}
				Gui::CloseCurrentPopup();
			}
			Gui::EndDisabled();

			UI::Draw::Underline();

			Gui::BeginDisabled(callbacks.OnComponentResetFn == nullptr);
			if (Gui::MenuItem("Reset Component"))
			{
				if (callbacks.OnComponentResetFn != nullptr)
				{
					std::invoke(callbacks.OnComponentResetFn, component, actor);
				}
				Gui::CloseCurrentPopup();
			}
			Gui::EndDisabled();

			if (callbacks.IsRemoveable)
			{
				UI::Draw::Underline();

				if (Gui::MenuItem("Remove Component"))
				{
					removeComponent = true;
					Gui::CloseCurrentPopup();
				}
			}

			Gui::EndPopup();
		}

		if (propertyGridHeaderOpen)
		{
			VX_CORE_ASSERT(callbacks.OnGuiRenderFn != nullptr, "All components must have OnGuiRender callback!");
			std::invoke(callbacks.OnGuiRenderFn, component, actor);
			UI::EndTreeNode();
		}

		if (removeComponent)
		{
			if (callbacks.OnComponentRemovedFn != nullptr)
			{
				std::invoke(callbacks.OnComponentRemovedFn, component, actor);
			}

			actor.RemoveComponent<TComponent>();
		}
	}

	// warning C4996: 'std::codecvt_utf8<char32_t,1114111,(std::codecvt_mode)0>': warning STL4017: std::wbuffer_convert, std::wstring_convert, and the <codecvt> header
// (containing std::codecvt_mode, std::codecvt_utf8, std::codecvt_utf16, and std::codecvt_utf8_utf16) are deprecated in C++17. (The std::codecvt class template is NOT deprecated.)
// The C++ Standard doesn't provide equivalent non-deprecated functionality; consider using MultiByteToWideChar() and WideCharToMultiByte() from <Windows.h> instead.
// You can define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning.
#pragma warning(disable : 4996)

	// From https://stackoverflow.com/questions/31302506/stdu32string-conversion-to-from-stdstring-and-stdu16string
	static std::string To_UTF8(const std::u32string& s)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
		return conv.to_bytes(s);
	}

#pragma warning(default : 4996)

	void SceneHierarchyPanel::DrawComponents(Actor actor)
	{
		// Tag Component
		TagComponent& tagComponent = actor.GetComponent<TagComponent>();
		TagComponentOnGuiRender(tagComponent, actor);

		Gui::PopItemWidth();

		DisplayAddComponentPopup();

		ComponentUICallbacks<TransformComponent> transformComponentCallbacks;
		transformComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::TransformComponentOnGuiRender);
		transformComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = TransformComponent(); };
		transformComponentCallbacks.OnComponentCopiedFn = [&](const auto& component, auto actor) { m_CopyActor.AddOrReplaceComponent<TransformComponent>() = component; };
		transformComponentCallbacks.OnComponentPastedFn = [&](auto& component, auto actor) { component = m_CopyActor.GetComponent<TransformComponent>(); };
		transformComponentCallbacks.IsRemoveable = false;
		DrawComponent<TransformComponent>("Transform", actor, transformComponentCallbacks);

		ComponentUICallbacks<ScriptComponent> scriptComponentCallbacks;
		scriptComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::ScriptComponentOnGuiRender);
		scriptComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = ScriptComponent(); };
		DrawComponent<ScriptComponent>("Script", actor, scriptComponentCallbacks);

		ComponentUICallbacks<CameraComponent> cameraComponentCallbacks;
		cameraComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CameraComponentOnGuiRender);
		cameraComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) {
			auto ResetCameraFn = [&](auto type)
			{
				component = CameraComponent();
				component.Camera.SetProjectionType(type);
			};

			switch (component.Camera.GetProjectionType())
			{
				case SceneCamera::ProjectionType::Perspective:  ResetCameraFn(SceneCamera::ProjectionType::Perspective);  break;
				case SceneCamera::ProjectionType::Orthographic: ResetCameraFn(SceneCamera::ProjectionType::Orthographic); break;
			}
		};
		DrawComponent<CameraComponent>("Camera", actor, cameraComponentCallbacks);

		ComponentUICallbacks<SkyboxComponent> skyboxComponentCallbacks;
		skyboxComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::SkyboxComponentOnGuiRender);
		skyboxComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = SkyboxComponent(); };
		DrawComponent<SkyboxComponent>("Skybox", actor, skyboxComponentCallbacks);

		ComponentUICallbacks<LightSourceComponent> lightSourceComponentCallbacks;
		lightSourceComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::LightSourceComponentOnGuiRender);
		lightSourceComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) {
			switch (component.Type)
			{
				case LightType::Directional: component = LightSourceComponent(LightType::Directional); break;
				case LightType::Point:       component = LightSourceComponent(LightType::Point);       break;
				case LightType::Spot:        component = LightSourceComponent(LightType::Spot);        break;
			}
		};
		DrawComponent<LightSourceComponent>("Light Source", actor, lightSourceComponentCallbacks);

		ComponentUICallbacks<LightSource2DComponent> lightSource2DComponentCallbacks;
		DrawComponent<LightSource2DComponent>("Light Source 2D", actor, lightSource2DComponentCallbacks);

		ComponentUICallbacks<MeshRendererComponent> meshRendererComponentCallbacks;
		meshRendererComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::MeshRendererComponentOnGuiRender);
		meshRendererComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = MeshRendererComponent(); };
		DrawComponent<MeshRendererComponent>("Mesh Renderer", actor, meshRendererComponentCallbacks);

		ComponentUICallbacks<StaticMeshRendererComponent> staticMeshRendererComponentCallbacks;
		staticMeshRendererComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::StaticMeshRendererComponentOnGuiRender);
		staticMeshRendererComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) {
			component = StaticMeshRendererComponent();
			component.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(DefaultMesh::StaticMeshType::Cube);
			SharedReference<MaterialTable> materialTable = component.Materials;
			materialTable->SetMaterial(0, Material::GetDefaultMaterialHandle());
		};
		DrawComponent<StaticMeshRendererComponent>("Static Mesh Renderer", actor, staticMeshRendererComponentCallbacks);

		ComponentUICallbacks<SpriteRendererComponent> spriteRendererComponentCallbacks;
		spriteRendererComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::SpriteRendererComponentOnGuiRender);
		spriteRendererComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = SpriteRendererComponent(); };
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", actor, spriteRendererComponentCallbacks);

		ComponentUICallbacks<CircleRendererComponent> circleRendererComponentCallbacks;
		circleRendererComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CircleRendererComponentOnGuiRender);
		DrawComponent<CircleRendererComponent>("Circle Renderer", actor, circleRendererComponentCallbacks);

		ComponentUICallbacks<ParticleEmitterComponent> particleEmitterComponentCallbacks;
		particleEmitterComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::ParticleEmitterComponentOnGuiRender);
		particleEmitterComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = ParticleEmitterComponent(); };
		DrawComponent<ParticleEmitterComponent>("Particle Emitter", actor, particleEmitterComponentCallbacks);

		ComponentUICallbacks<AnimatorComponent> animatorComponentCallbacks;
		animatorComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::AnimatorComponentOnGuiRender);
		animatorComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = AnimatorComponent(); };
		DrawComponent<AnimatorComponent>("Animator", actor, animatorComponentCallbacks);

		ComponentUICallbacks<AnimationComponent> animationComponentCallbacks;
		animationComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::AnimationComponentOnGuiRender);
		animationComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = AnimationComponent(); };
		DrawComponent<AnimationComponent>("Animation", actor, animationComponentCallbacks);

		ComponentUICallbacks<TextMeshComponent> textMeshComponentCallbacks;
		textMeshComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::TextMeshComponentOnGuiRender);
		textMeshComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = TextMeshComponent(); };
		DrawComponent<TextMeshComponent>("Text Mesh", actor, textMeshComponentCallbacks);

		ComponentUICallbacks<ButtonComponent> buttonComponentCallbacks;
		buttonComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::ButtonComponentOnGuiRender);
		buttonComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = ButtonComponent(); };
		DrawComponent<ButtonComponent>("Button", actor, buttonComponentCallbacks);

		ComponentUICallbacks<AudioSourceComponent> audioSourceComponentCallbacks;
		audioSourceComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::AudioSourceComponentOnGuiRender);
		audioSourceComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = AudioSourceComponent(); };
		DrawComponent<AudioSourceComponent>("Audio Source", actor, audioSourceComponentCallbacks);

		ComponentUICallbacks<AudioListenerComponent> audioListenerComponentCallbacks;
		audioListenerComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::AudioListenerComponentOnGuiRender);
		audioListenerComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = AudioListenerComponent(); };
		DrawComponent<AudioListenerComponent>("Audio Listener", actor, audioListenerComponentCallbacks);

		ComponentUICallbacks<RigidBodyComponent> rigidBodyComponentCallbacks;
		rigidBodyComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::RigidBodyComponentOnGuiRender);
		rigidBodyComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = RigidBodyComponent(); };
		DrawComponent<RigidBodyComponent>("RigidBody", actor, rigidBodyComponentCallbacks);

		ComponentUICallbacks<CharacterControllerComponent> characterControllerComponentCallbacks;
		characterControllerComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CharacterControllerComponentOnGuiRender);
		characterControllerComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = CharacterControllerComponent(); };
		DrawComponent<CharacterControllerComponent>("Character Controller", actor, characterControllerComponentCallbacks);

		ComponentUICallbacks<FixedJointComponent> fixedJointComponentCallbacks;
		fixedJointComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::FixedJointComponentOnGuiRender);
		fixedJointComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = FixedJointComponent(); };
		DrawComponent<FixedJointComponent>("Fixed Joint", actor, fixedJointComponentCallbacks);

		ComponentUICallbacks<BoxColliderComponent> boxColliderComponentCallbacks;
		boxColliderComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::BoxColliderComponentOnGuiRender);
		boxColliderComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = BoxColliderComponent(); };
		DrawComponent<BoxColliderComponent>("Box Collider", actor, boxColliderComponentCallbacks);

		ComponentUICallbacks<SphereColliderComponent> sphereColliderComponentCallbacks;
		sphereColliderComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::SphereColliderComponentOnGuiRender);
		sphereColliderComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = SphereColliderComponent(); };
		DrawComponent<SphereColliderComponent>("Sphere Collider", actor, sphereColliderComponentCallbacks);

		ComponentUICallbacks<CapsuleColliderComponent> capsuleColliderComponentCallbacks;
		capsuleColliderComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CapsuleColliderComponentOnGuiRender);
		capsuleColliderComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = CapsuleColliderComponent(); };
		DrawComponent<CapsuleColliderComponent>("Capsule Collider", actor, capsuleColliderComponentCallbacks);

		ComponentUICallbacks<MeshColliderComponent> meshColliderComponentCallbacks;
		meshColliderComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::MeshColliderComponentOnGuiRender);
		meshColliderComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = MeshColliderComponent(); };
		DrawComponent<MeshColliderComponent>("Mesh Collider", actor, meshColliderComponentCallbacks);

		ComponentUICallbacks<RigidBody2DComponent> rigidBody2DComponentCallbacks;
		rigidBody2DComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::RigidBody2DComponentOnGuiRender);
		rigidBody2DComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = RigidBody2DComponent(); };
		DrawComponent<RigidBody2DComponent>("RigidBody 2D", actor, rigidBody2DComponentCallbacks);

		ComponentUICallbacks<BoxCollider2DComponent> boxCollider2DComponentCallbacks;
		boxCollider2DComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::BoxCollider2DComponentOnGuiRender);
		boxCollider2DComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = BoxCollider2DComponent(); };
		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", actor, boxCollider2DComponentCallbacks);

		ComponentUICallbacks<CircleCollider2DComponent> circleCollider2DComponentCallbacks;
		circleCollider2DComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CircleCollider2DComponentOnGuiRender);
		circleCollider2DComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = CircleCollider2DComponent(); };
		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", actor, circleCollider2DComponentCallbacks);

		ComponentUICallbacks<NavMeshAgentComponent> navMeshAgentComponentCallbacks;
		navMeshAgentComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::NavMeshAgentComponentOnGuiRender);
		navMeshAgentComponentCallbacks.OnComponentResetFn = [](auto& component, auto actor) { component = NavMeshAgentComponent(); };
		DrawComponent<NavMeshAgentComponent>("Nav Mesh Agent", actor, navMeshAgentComponentCallbacks);

		ComponentUICallbacks<NativeScriptComponent> nativeScriptComponentCallbacks;
		DrawComponent<NativeScriptComponent>("Native Script", actor, nativeScriptComponentCallbacks);
	}

	void SceneHierarchyPanel::TagComponentOnGuiRender(TagComponent& component, Actor actor)
	{
		std::string& tag = component.Tag;

		char buffer[ACTOR_TAG_BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), tag.c_str());

		const bool shouldRename = m_ActorShouldBeRenamed && SelectionManager::GetSelectedActor() == actor;
		const ImGuiInputTextFlags flags = shouldRename ? (ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue) : 0;

		if (shouldRename)
		{
			Gui::SetKeyboardFocusHere();
			m_IsEditingActorName = true;
		}
		if (Gui::InputTextWithHint("##Tag", "Actor Name", buffer, sizeof(buffer), flags))
		{
			tag = std::string(buffer);

			// Set the focus to the scene panel otherwise the keyboard focus will still be on the input text box
			if (m_ActorShouldBeRenamed)
				Gui::SetWindowFocus("Scene");

			FocusOnActorName(false);
			m_IsEditingActorName = false;
		}

		m_IsEditingActorName = Gui::IsItemActive();

		UI::DrawItemActivityOutline();

		Gui::SameLine();
		Gui::PushItemWidth(-1);

		const bool controlPressed = Input::IsKeyDown(KeyCode::LeftControl) || Input::IsKeyDown(KeyCode::RightControl);
		const bool shiftPressed = Input::IsKeyDown(KeyCode::LeftShift) || Input::IsKeyDown(KeyCode::RightShift);
		const bool aKeyPressed = Input::IsKeyDown(KeyCode::A);
		const bool shortcutPressed = controlPressed && shiftPressed && aKeyPressed;
		const bool windowHovered = Gui::IsWindowHovered();

		if (Gui::Button("Add Component") || (shortcutPressed && windowHovered))
		{
			Gui::OpenPopup("AddComponent");

			// We should reset the search bar here
			memset(m_ComponentSearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
			m_ComponentSearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed
		}

		UI::DrawItemActivityOutline();

		UI::BeginPropertyGrid(100.0f);

		bool active = component.IsActive;
		if (UI::Property("Active", active, "enables or disables the actor from being processed by the engine systems"))
		{
			actor.SetActive(active);
		}

		UI::EndPropertyGrid();

		Gui::SameLine();

		std::vector<std::string>& markers = component.Markers;
		std::string& currentMarker = component.Marker;

		auto it = std::find(markers.begin(), markers.end(), currentMarker);

		if (it != markers.end())
		{
			const char* currentCStr = (*it).c_str();

			if (Gui::BeginCombo("##Marker", currentCStr, ImGuiComboFlags_HeightLarge))
			{
				uint32_t arraySize = markers.size();

				for (uint32_t i = 0; i < arraySize; i++)
				{
					const bool isSelected = strcmp(currentCStr, markers[i].c_str()) == 0;

					if (Gui::Selectable(markers[i].c_str(), isSelected))
					{
						currentMarker = markers[i];
					}

					if (isSelected)
					{
						Gui::SetItemDefaultFocus();
					}

					// skip last item
					if (i != arraySize - 1)
					{
						UI::Draw::Underline();
						Gui::Spacing();
					}
					else
					{
						UI::Draw::Underline();

						const char* addMarkerButtonText = "Add Marker";
						if (Gui::Button(addMarkerButtonText, { Gui::GetContentRegionAvail().x, Gui::CalcTextSize(addMarkerButtonText).y * 1.5f }))
						{
							m_DisplayAddMarkerPopup = true;
						}

						if (m_DisplayAddMarkerPopup)
						{
							Gui::OpenPopup("AddMarker");
							m_DisplayAddMarkerPopup = false;
						}

						if (UI::BeginPopup("AddMarker", ImGuiWindowFlags_NoMove))
						{
							DisplayAddMarkerPopup(component);

							UI::EndPopup();
						}
					}
				}

				Gui::EndCombo();
			}

			UI::DrawItemActivityOutline();
		}
		else
		{
			// Add to markers vector because it's not a default marker
			component.AddMarker(component.Marker);
		}
	}

	void SceneHierarchyPanel::TransformComponentOnGuiRender(TransformComponent& component, Actor actor)
	{
		const float columnWidth = 100.0f;
		UI::BeginPropertyGrid();
		UI::DrawVec3Controls("Translation", component.Translation, 0.0f, columnWidth, -FLT_MIN, FLT_MAX);
		Math::vec3 rotation = Math::Rad2Deg(component.GetRotationEuler());
		UI::DrawVec3Controls("Rotation", rotation, 0.0f, columnWidth, -FLT_MIN, FLT_MAX, [&]()
		{
			const float maxRotationEuler = 360.0f;
			if (rotation.x > maxRotationEuler || rotation.x < -maxRotationEuler)
				rotation.x = 0.0f;
			if (rotation.y > maxRotationEuler || rotation.y < -maxRotationEuler)
				rotation.y = 0.0f;
			if (rotation.z > maxRotationEuler || rotation.z < -maxRotationEuler)
				rotation.z = 0.0f;
			component.SetRotationEuler(Math::Deg2Rad(rotation));
		});
		UI::DrawVec3Controls("Scale", component.Scale, 1.0f, columnWidth, FLT_MIN, FLT_MAX);
		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::CameraComponentOnGuiRender(CameraComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		SceneCamera& camera = component.Camera;

		UI::Property("Primary", component.Primary);

		const char* projectionTypes[] = { "Perspective", "Othrographic" };
		int32_t currentProjectionType = (int32_t)camera.GetProjectionType();
		if (UI::PropertyDropdown("Projection", projectionTypes, VX_ARRAYSIZE(projectionTypes), currentProjectionType))
		{
			camera.SetProjectionType((SceneCamera::ProjectionType)currentProjectionType);
		}

		switch (camera.GetProjectionType())
		{
			case SceneCamera::ProjectionType::Perspective:
			{
				float perspectiveVerticalFOV = Math::Rad2Deg(camera.GetPerspectiveFOV());
				if (UI::Property("Field of View", perspectiveVerticalFOV, 1.0f, FLT_MIN, 180.0f))
				{
					camera.SetPerspectiveFOV(Math::Deg2Rad(perspectiveVerticalFOV));
				}

				float nearClip = camera.GetPerspectiveNearClip();
				float farClip = camera.GetPerspectiveFarClip();

				if (UI::Property("Near Clip", nearClip, 0.1f, FLT_MIN, farClip))
				{
					camera.SetPerspectiveNearClip(nearClip);
				}

				if (UI::Property("Far Clip", farClip, 0.1f, nearClip, FLT_MAX))
				{
					camera.SetPerspectiveFarClip(farClip);
				}

				break;
			}
			case SceneCamera::ProjectionType::Orthographic:
			{
				float orthoSize = camera.GetOrthographicSize();
				if (UI::Property("Ortho Size", orthoSize, 0.1f, FLT_MIN, FLT_MAX))
				{
					camera.SetOrthographicSize(orthoSize);
				}

				float nearClip = camera.GetOrthographicNearClip();
				float farClip = camera.GetOrthographicFarClip();

				if (UI::Property("Near Clip", nearClip, 0.1f, FLT_MIN, farClip))
				{
					camera.SetOrthographicNearClip(nearClip);
				}

				if (UI::Property("Far Clip", farClip, 0.1f, nearClip, FLT_MAX))
				{
					camera.SetOrthographicFarClip(farClip);
				}

				UI::Property("Fixed Aspect Ratio", component.FixedAspectRatio);

				break;
			}
		}

		UI::Property("Clear Color", &component.ClearColor);
		
		if (UI::Property("Post Processing", component.PostProcessing.Enabled))
		{
			if (!component.PostProcessing.Enabled)
			{
				// turn off all post processing effects
				component.PostProcessing.Bloom.Enabled = false;
			}
		}

		UI::EndPropertyGrid();

		if (component.PostProcessing.Enabled && UI::PropertyGridHeader("Post Processing", false))
		{
			if (UI::PropertyGridHeader("Bloom", false))
			{
				UI::BeginPropertyGrid();
				UI::Property("Enabled", component.PostProcessing.Bloom.Enabled);

				if (component.PostProcessing.Bloom.Enabled)
				{
					UI::Property("Threshold", component.PostProcessing.Bloom.Threshold);
					UI::Property("Knee", component.PostProcessing.Bloom.Knee);
					UI::Property("Intensity", component.PostProcessing.Bloom.Intensity);

					static const char* bloomBlurSampleSizes[] = { "5", "10", "15", "20", "40" };
					uint32_t bloomSampleSize = Renderer::GetBloomSampleSize();

					uint32_t currentBloomBlurSamplesSize = 0;

					if (bloomSampleSize == 5)  currentBloomBlurSamplesSize = 0;
					if (bloomSampleSize == 10) currentBloomBlurSamplesSize = 1;
					if (bloomSampleSize == 15) currentBloomBlurSamplesSize = 2;
					if (bloomSampleSize == 20) currentBloomBlurSamplesSize = 3;
					if (bloomSampleSize == 40) currentBloomBlurSamplesSize = 4;

					if (UI::PropertyDropdown("Bloom Blur Samples", bloomBlurSampleSizes, VX_ARRAYSIZE(bloomBlurSampleSizes), currentBloomBlurSamplesSize))
					{
						switch (currentBloomBlurSamplesSize)
						{
							case 0: Renderer::SetBloomSampleSize(5);  break;
							case 1: Renderer::SetBloomSampleSize(10); break;
							case 2: Renderer::SetBloomSampleSize(15); break;
							case 3: Renderer::SetBloomSampleSize(20); break;
							case 4: Renderer::SetBloomSampleSize(40); break;
						}
					}
				}

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}

			UI::EndTreeNode();
		}

		if (camera.IsDirty())
		{
			const Math::uvec2& viewportSize = m_ContextScene->GetViewportSize();
			camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}
	}

	void SceneHierarchyPanel::SkyboxComponentOnGuiRender(SkyboxComponent& component, Actor actor)
	{
		AssetHandle environmentHandle = component.Skybox;
		SharedReference<Skybox> skybox = nullptr;

		if (AssetManager::IsHandleValid(environmentHandle))
		{
			skybox = AssetManager::GetAsset<Skybox>(environmentHandle);
		}

		std::string relativePath = "(null)";

		if (skybox && skybox->IsLoaded())
		{
			const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.Skybox);
			relativePath = metadata.Filepath.stem().string();
		}

		auto OnSkyboxDroppedFn = [&](const Fs::Path& filepath) {
			// Make sure we are recieving an actual directory or hdr texture otherwise we will have trouble loading it
			if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath); type == AssetType::EnvironmentAsset)
			{
				AssetHandle environmentHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);
				if (AssetManager::IsHandleValid(environmentHandle))
				{
					component.Skybox = environmentHandle;
				}
			}
			else
			{
				VX_CONSOLE_LOG_WARN("Failed to load skybox '{}'", filepath.filename().string());
			}
		};

		UI::BeginPropertyGrid();
		UI::PropertyAssetReference<Skybox>("Environment Map", relativePath, component.Skybox, OnSkyboxDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry());
		UI::EndPropertyGrid();

		if (skybox && skybox->IsLoaded())
		{
			UI::BeginPropertyGrid();

			UI::Property("Rotation", component.Rotation, 1.0f, FLT_MIN, FLT_MAX);
			if (Gui::IsItemFocused())
			{
				// Nasty hack to reload skybox
				if (Input::IsKeyPressed(KeyCode::Enter))
				{
					skybox->SetShouldReload(true);
				}
			}

			UI::Property("Intensity", component.Intensity, 0.01f, FLT_MIN, FLT_MAX);

			UI::EndPropertyGrid();
		}
	}

	void SceneHierarchyPanel::LightSourceComponentOnGuiRender(LightSourceComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		static const char* lightTypes[] = { "Directional", "Point", "Spot" };
		int32_t currentLightType = (int32_t)component.Type;
		if (UI::PropertyDropdown("Light Type", lightTypes, VX_ARRAYSIZE(lightTypes), currentLightType))
			component.Type = (LightType)currentLightType;

		switch (component.Type)
		{
			case LightType::Directional:
			{
				break;
			}
			case LightType::Point:
			{
				break;
			}
			case LightType::Spot:
			{
				UI::Property("CutOff", component.Cutoff, 0.5f, FLT_MIN, component.OuterCutoff);
				UI::Property("Outer CutOff", component.OuterCutoff, 0.5f, component.Cutoff, FLT_MAX);

				break;
			}
		}

		UI::Property("Radiance", &component.Radiance);
		UI::Property("Intensity", component.Intensity, 0.01f, FLT_MIN, FLT_MAX);

		UI::EndPropertyGrid();

		if (UI::PropertyGridHeader("Shadows", false))
		{
			UI::BeginPropertyGrid();

			if (UI::Property("Cast Shadows", component.CastShadows))
			{
				if (component.CastShadows)
				{
					Renderer::CreateShadowMap(component.Type);
				}
			}

			if (component.CastShadows)
			{
				UI::Property("Soft Shadows", component.SoftShadows);
				UI::Property("Shadow Bias", component.ShadowBias, 0.01f, FLT_MIN, FLT_MAX);
			}

			UI::EndPropertyGrid();
			UI::EndTreeNode();
		}
	}

	void SceneHierarchyPanel::MeshRendererComponentOnGuiRender(MeshRendererComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		std::string relativeMeshPath = "(null)";

		if (AssetManager::IsHandleValid(component.Mesh))
		{
			const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.Mesh);
			relativeMeshPath = metadata.Filepath.stem().string();
		}

		UI::Property("Mesh Source", relativeMeshPath, true);

		UI::EndPropertyGrid();

		// Accept a Model File from the content browser
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				Fs::Path meshFilepath = Fs::Path(path);

				// Make sure we are recieving an actual model file otherwise we will have trouble opening it
				if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(meshFilepath); type == AssetType::MeshAsset || type == AssetType::StaticMeshAsset)
				{
					AssetHandle meshHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(meshFilepath);
					if (AssetManager::IsHandleValid(meshHandle))
					{
						component.Mesh = meshHandle;

						if (actor.HasComponent<AnimatorComponent>() && actor.HasComponent<AnimationComponent>() && AssetManager::GetAsset<Mesh>(component.Mesh)->HasAnimations())
						{
							AnimatorComponent& animatorComponent = actor.GetComponent<AnimatorComponent>();
							AnimationComponent& animationComponent = actor.GetComponent<AnimationComponent>();

							animationComponent.Animation = Animation::Create(meshFilepath.string(), component.Mesh);
							animatorComponent.Animator = Animator::Create(animationComponent.Animation);
						}
					}
				}
				else
				{
					VX_CONSOLE_LOG_WARN("Failed to load mesh '{}'", meshFilepath.filename().string());
				}
			}

			Gui::EndDragDropTarget();
		}

		// TODO materials ///////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	void SceneHierarchyPanel::StaticMeshRendererComponentOnGuiRender(StaticMeshRendererComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		std::string relativePath = "(null)";

		if (AssetManager::IsHandleValid(component.StaticMesh))
		{
			const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.StaticMesh);
			relativePath = metadata.Filepath.stem().string();
		}

		auto OnStaticMeshDroppedFn = [&](const Fs::Path& filepath) {
			// Make sure we are recieving an actual model file otherwise we will have trouble opening it
			// TODO should we check for both mesh types or just static? (i.e. static and normal meshes)
			if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath); type == AssetType::StaticMeshAsset || type == AssetType::MeshAsset)
			{
				AssetHandle staticMeshHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);
				if (AssetManager::IsHandleValid(staticMeshHandle))
				{
					component.StaticMesh = staticMeshHandle;
					component.Type = MeshType::Custom;

					SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(component.StaticMesh);
					if (staticMesh)
					{
						component.Materials->Clear();
						staticMesh->LoadMaterialTable(component.Materials);
					}
				}
			}
			else
			{
				VX_CONSOLE_LOG_WARN("Failed to load static mesh '{}'", filepath.filename().string());
			}
		};

		if (UI::PropertyAssetReference<StaticMesh>("Mesh Source", relativePath, component.StaticMesh, OnStaticMeshDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry()))
		{
			if (AssetManager::IsHandleValid(component.StaticMesh))
			{
				const bool isDefaultStaticMesh = Project::GetEditorAssetManager()->IsDefaultStaticMesh(component.StaticMesh);

				if (isDefaultStaticMesh)
				{
					component.Type = (MeshType)DefaultMesh::GetStaticMeshType(component.StaticMesh);
				}
				else
				{
					component.Type = MeshType::Custom;
				}

				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(component.StaticMesh);
				if (staticMesh)
				{
					component.Materials->Clear();
					staticMesh->LoadMaterialTable(component.Materials);
				}
			}
		}

		UI::EndPropertyGrid();

		if (AssetManager::IsHandleValid(component.StaticMesh))
		{
			if (UI::PropertyGridHeader("Materials", false))
			{
				UI::BeginPropertyGrid();

				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(component.StaticMesh);

				SharedReference<MaterialTable>& materialTable = component.Materials;

				uint32_t submeshIndex = 0;

				auto OnMaterialDroppedFn = [&](const Fs::Path& filepath) {
					// Make sure we are recieving an actual material otherwise we will have trouble opening it
					if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath); type == AssetType::MaterialAsset)
					{
						AssetHandle materialHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);

						if (AssetManager::IsHandleValid(materialHandle))
						{
							SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
							if (material)
							{
								materialTable->SetMaterial(submeshIndex, material->Handle);
								material->SetName(FileSystem::RemoveFileExtension(filepath));
							}
						}
						else
						{
							VX_CONSOLE_LOG_WARN("Failed to load material '{}'", filepath.filename().string());
						}
					}
					else
					{
						VX_CONSOLE_LOG_WARN("Failed to load material '{}'", filepath.filename().string());
					}
				};

				for (;;)
				{
					if (!materialTable->HasMaterial(submeshIndex))
					{
						if (materialTable->GetMaterialCount() == 0)
						{
							if (!AssetManager::IsHandleValid(Renderer::GetWhiteMaterial()->Handle))
							{
								Renderer::GetWhiteMaterial()->Handle = AssetHandle();
								Project::GetEditorAssetManager()->AddMemoryOnlyAsset(Renderer::GetWhiteMaterial());
							}

							materialTable->SetMaterial(submeshIndex, Renderer::GetWhiteMaterial()->Handle);
						}

						break;
					}

					AssetHandle materialHandle = materialTable->GetMaterial(submeshIndex);
					std::string relativePath = Project::GetEditorAssetManager()->GetMetadata(materialHandle).Filepath.stem().string();
					if (UI::PropertyAssetReference<Material>("Material", relativePath, materialHandle, OnMaterialDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry()))
					{
						materialTable->SetMaterial(submeshIndex, materialHandle);
					}

					submeshIndex++;
				}

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}

			if (UI::PropertyGridHeader("Rendering", false))
			{
				UI::BeginPropertyGrid();

				UI::Property("Cast Shadows", component.CastShadows);

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}
		}
	}

	void SceneHierarchyPanel::SpriteRendererComponentOnGuiRender(SpriteRendererComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();
		UI::Property("Visible", component.Visible);
		UI::EndPropertyGrid();

		SharedReference<Texture2D> icon = EditorResources::CheckerboardIcon;

		if (AssetManager::IsHandleValid(component.Texture))
		{
			icon = AssetManager::GetAsset<Texture2D>(component.Texture);
		}

		auto OnTextureDroppedFn = [&](const Fs::Path& filepath) {
			// Make sure we are recieving an actual texture otherwise we will have trouble opening it
			if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath); type == AssetType::TextureAsset)
			{
				AssetHandle textureHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);
				if (AssetManager::IsHandleValid(textureHandle))
				{
					component.Texture = textureHandle;
				}
				else
				{
					VX_CONSOLE_LOG_WARN("Failed to load texture '{}'", filepath.filename().string());
				}
			}
			else
			{
				VX_CONSOLE_LOG_WARN("Failed to load texture '{}'", filepath.filename().string());
			}
		};

		const float imageSize = 64.0f;

		UI::PropertyAssetImageReferenceSettings settings;
		settings.Label = "Texture";
		std::string relativePath = icon == EditorResources::CheckerboardIcon ? "(null)" : Fs::Path(icon->GetPath()).stem().string().c_str();
		settings.CurrentFilepath = &relativePath;
		settings.CurrentHandle = &component.Texture;
		settings.CurrentImage.Texture = icon;
		settings.CurrentImage.Size = Math::vec2(imageSize);
		settings.CurrentImage.TintColor = component.SpriteColor;
		std::unordered_set<AssetHandle> textureHandles = Project::GetEditorAssetManager()->GetAllAssetsWithType(AssetType::TextureAsset);
		std::vector<UI::UIImage> images;
		for (AssetHandle handle : textureHandles)
		{
			if (!AssetManager::IsHandleValid(handle))
				continue;

			SharedReference<Texture2D> texture = AssetManager::GetAsset<Texture2D>(handle);
			if (!texture)
				continue;

			UI::UIImage image;
			image.Texture = texture;
			image.Size = Math::vec2(imageSize);
			image.TintColor = ColorToVec4(Color::White);

			images.push_back(image);
		}
		settings.AvailableImages = images;
		settings.OnAssetDroppedFn = OnTextureDroppedFn;
		settings.Registry = &Project::GetEditorAssetManager()->GetAssetRegistry();

		UI::BeginPropertyGrid();
		UI::PropertyAssetImageReference<Texture2D>(settings);
		UI::EndPropertyGrid();

		UI::BeginPropertyGrid();

		UI::Property("Color", &component.SpriteColor);
		UI::Property("UV", component.TextureUV, 0.01f, FLT_MIN, FLT_MAX);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::CircleRendererComponentOnGuiRender(CircleRendererComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		UI::Property("Color", &component.Color);
		UI::Property("Thickness", component.Thickness, 0.01f, FLT_MIN, 1.0f);
		UI::Property("Fade", component.Fade, 0.01f, FLT_MIN, 1.0f);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::ParticleEmitterComponentOnGuiRender(ParticleEmitterComponent& component, Actor actor)
	{
		SharedReference<ParticleEmitter> particleEmitter = nullptr;
		if (AssetManager::IsHandleValid(component.EmitterHandle))
			particleEmitter = AssetManager::GetAsset<ParticleEmitter>(component.EmitterHandle);

		if (particleEmitter == nullptr)
		{
			return;
		}

		const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(particleEmitter->Handle);
		if (metadata.IsMemoryOnly)
		{
			if (Gui::Button("Create Particle System"))
			{
				SystemManager::GetAssetSystem<ParticleSystem>()->CreateAsset(actor);
			}
		}

		auto OnParticleEmitterDroppedFn = [&](const Fs::Path& filepath) {
			// Make sure we are recieving an actual particle system otherwise we will have trouble opening it
			if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath); type == AssetType::ParticleAsset)
			{
				const std::string extension = FileSystem::GetFileExtension(filepath);
				if (extension == ".vparticle")
				{
					const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(filepath);
					component.EmitterHandle = metadata.Handle;
				}
				else
				{
					VX_CONSOLE_LOG_WARN("Failed to load particle system '{}'", filepath.filename().string());
				}
			}
			else
			{
				VX_CONSOLE_LOG_WARN("Failed to load particle system '{}'", filepath.filename().string());
			}
		};

		{
			Gui::BeginDisabled(metadata.IsMemoryOnly);
			Gui::BeginDisabled(component.IsActive);
			if (Gui::Button((const char*)VX_ICON_PLAY))
			{
				if (!component.IsActive)
				{
					component.IsActive = true;
				}
			}
			UI::SetTooltip("Play");
			Gui::EndDisabled();

			Gui::SameLine();

			Gui::BeginDisabled(!component.IsActive);
			if (Gui::Button((const char*)VX_ICON_STOP))
			{
				if (component.IsActive)
				{
					component.IsActive = false;
				}
			}
			UI::SetTooltip("Stop");
			Gui::EndDisabled();
			Gui::EndDisabled();
		}

		UI::BeginPropertyGrid();
		std::string emitterName = particleEmitter->GetName();
		UI::PropertyAssetReference<ParticleEmitter>("Source", emitterName, component.EmitterHandle, OnParticleEmitterDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry());
		UI::EndPropertyGrid();

		if (metadata.IsMemoryOnly)
		{
			return;
		}

		UI::BeginPropertyGrid();

		ParticleEmitterProperties& emitterProperties = particleEmitter->GetProperties();

		UI::Property("Velocity", emitterProperties.Velocity, 0.01f, -FLT_MAX, FLT_MAX);
		UI::Property("Velocity Variation", emitterProperties.VelocityVariation, 0.01f, FLT_MIN, FLT_MAX);
		UI::Property("Offset", emitterProperties.Offset, 0.01f, -FLT_MAX, FLT_MAX);
		UI::Property("Size Start", emitterProperties.SizeBegin, 0.01f, FLT_MIN, FLT_MAX);
		UI::Property("Size End", emitterProperties.SizeEnd, 0.01f, FLT_MIN, FLT_MAX);
		UI::Property("Size Variation", emitterProperties.SizeVariation, 0.01f, FLT_MIN, FLT_MAX);

		UI::Property("Generate Random Colors", emitterProperties.GenerateRandomColors);

		if (!emitterProperties.GenerateRandomColors)
		{
			UI::Property("Color Start", &emitterProperties.ColorBegin);
			UI::Property("Color End", &emitterProperties.ColorEnd);
		}

		UI::Property("Rotation", emitterProperties.Rotation, 0.1f, FLT_MIN, FLT_MAX);
		UI::Property("Lifetime", emitterProperties.LifeTime, 0.1f, FLT_MIN, FLT_MAX);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::AnimatorComponentOnGuiRender(AnimatorComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		SharedRef<Animator> animator = component.Animator;

		if (animator)
		{
			if (Gui::Button((const char*)VX_ICON_PLAY))
				animator->PlayAnimation();
			UI::SetTooltip("Play");

			Gui::SameLine();

			if (Gui::Button((const char*)VX_ICON_STOP))
				animator->Stop();
			UI::SetTooltip("Stop");
		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::AnimationComponentOnGuiRender(AnimationComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		SharedRef<Animation> animation = component.Animation;

		if (animation)
		{

		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::TextMeshComponentOnGuiRender(TextMeshComponent& component, Actor actor)
	{
		std::string relativePath = "Default Font";

		if (AssetManager::IsHandleValid(component.FontAsset))
		{
			const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.FontAsset);
			relativePath = metadata.Filepath.stem().string();
		}

		auto OnFontDroppedFn = [&](const Fs::Path& filepath) {
			// Make sure we are recieving an actual font otherwise we will have trouble opening it
			if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath); type == AssetType::FontAsset)
			{
				const std::string extension = FileSystem::GetFileExtension(filepath);
				if (extension == ".vfa")
				{
					const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(filepath);
					component.FontAsset = metadata.Handle;
				}
				else
				{
					VX_CONSOLE_LOG_WARN("Failed to load font '{}'", filepath.filename().string());
				}
			}
			else
			{
				VX_CONSOLE_LOG_WARN("Failed to load font '{}'", filepath.filename().string());
			}
		};

		UI::BeginPropertyGrid();
		UI::PropertyAssetReference<Font>("Font Source", relativePath, component.FontAsset, OnFontDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry());
		UI::EndPropertyGrid();

		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		if (UI::MultilineTextBox("Text", component.TextString))
		{
			component.TextHash = std::hash<std::string>()(component.TextString);
		}

		UI::Property("Color", &component.Color);
		UI::Property("Background Color", &component.BackgroundColor);
		UI::Property("Line Spacing", component.LineSpacing, 0.1f, FLT_MIN, FLT_MAX);
		UI::Property("Kerning", component.Kerning, 0.1f, FLT_MIN, FLT_MAX);
		UI::Property("Max Width", component.MaxWidth, 0.1f, FLT_MIN, FLT_MAX);

		UI::Property("Drop Shadow", component.DropShadow.Enabled);

		UI::EndPropertyGrid();

		if (component.DropShadow.Enabled && UI::PropertyGridHeader("Drop Shadow", false))
		{
			UI::BeginPropertyGrid();

			UI::Property("Color", &component.DropShadow.Color);
			UI::Property("Shadow Distance", component.DropShadow.ShadowDistance, 0.01f, -FLT_MIN, FLT_MAX);
			UI::Property("Shadow Scale", component.DropShadow.ShadowScale, 0.01f, FLT_MIN, FLT_MAX);

			UI::EndPropertyGrid();

			UI::EndTreeNode();
		}
	}

	void SceneHierarchyPanel::ButtonComponentOnGuiRender(ButtonComponent& component, Actor actor)
	{
		std::string relativePath = "Default Font";

		if (AssetManager::IsHandleValid(component.Font.FontAsset))
		{
			const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.Font.FontAsset);
			relativePath = metadata.Filepath.stem().string();
		}

		auto OnFontDroppedFn = [&](const Fs::Path& filepath) {
			// Make sure we are recieving an actual font otherwise we will have trouble opening it
			if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath); type == AssetType::FontAsset)
			{
				const std::string extension = FileSystem::GetFileExtension(filepath);
				if (extension == ".vfa")
				{
					const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(filepath);
					component.Font.FontAsset = metadata.Handle;
				}
				else
				{
					VX_CONSOLE_LOG_WARN("Failed to load font '{}'", filepath.filename().string());
				}
			}
			else
			{
				VX_CONSOLE_LOG_WARN("Failed to load font '{}'", filepath.filename().string());
			}
		};

		UI::BeginPropertyGrid();
		UI::PropertyAssetReference<Font>("Font Source", relativePath, component.Font.FontAsset, OnFontDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry());
		UI::EndPropertyGrid();

		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		if (UI::MultilineTextBox("Text", component.Font.TextString))
		{
			component.Font.TextHash = std::hash<std::string>()(component.Font.TextString);
		}

		UI::Property("Background Color", &component.BackgroundColor);
		UI::Property("Clicked Color", &component.OnClickedColor, "color when the button is clicked");
		
		UI::EndPropertyGrid();

		if (UI::PropertyGridHeader("Text", false))
		{
			UI::BeginPropertyGrid();
			
			UI::Property("Color", &component.Font.Color);
			UI::Property("Outline Color", &component.Font.BackgroundColor);
			UI::Property("Offset", component.Font.Offset, 0.01f);
			UI::Property("Scale", component.Font.Scale, 0.01f, FLT_MIN, FLT_MAX);
			UI::Property("Line Spacing", component.Font.LineSpacing, 0.1f, FLT_MIN, FLT_MAX);
			UI::Property("Kerning", component.Font.Kerning, 0.1f, FLT_MIN, FLT_MAX);
			UI::Property("Max Width", component.Font.MaxWidth, 0.1f, FLT_MIN, FLT_MAX);

			UI::EndPropertyGrid();
			UI::EndTreeNode();
		}
	}

	void SceneHierarchyPanel::AudioSourceComponentOnGuiRender(AudioSourceComponent& component, Actor actor)
	{
		SharedReference<AudioSource> audioSource = nullptr;
		if (AssetManager::IsHandleValid(component.AudioHandle))
			audioSource = AssetManager::GetAsset<AudioSource>(component.AudioHandle);

		if (audioSource == nullptr)
		{
			return;
		}

		PlaybackDevice device = audioSource->GetPlaybackDevice();
		const bool validEngineID = device.GetEngine().GetID() != Wave::ID::Invalid;
		if (validEngineID && (device.GetSound().IsPlaying() || device.GetSound().IsPaused()))
		{
			Gui::BeginDisabled(!device.GetSound().IsPlaying());
			const float fraction = device.GetSound().GetCursorInSeconds() / device.GetSound().GetLengthInSeconds();
			Gui::ProgressBar(fraction);
			Gui::EndDisabled();
		}

		if (validEngineID)
		{
			Gui::BeginDisabled(device.GetSound().IsPlaying());
			if (Gui::Button((const char*)VX_ICON_PLAY))
			{
				// TODO once Wave has PlayOneShot we need to handle it here
				//if (audioSource->GetProperties().PlayOneShot)
				//{
					//audioSource->PlayOneShot();
				//}

				device.Play();
			}
			UI::SetTooltip("Play");
			Gui::EndDisabled();

			Gui::SameLine();

			Gui::BeginDisabled(!device.GetSound().IsPlaying());

			if (Gui::Button((const char*)VX_ICON_PAUSE))
				device.Pause();
			UI::SetTooltip("Pause");

			Gui::SameLine();

			if (Gui::Button((const char*)VX_ICON_REPEAT))
				device.Restart();
			UI::SetTooltip("Restart");

			Gui::SameLine();

			if (Gui::Button((const char*)VX_ICON_STOP))
				device.Stop();
			UI::SetTooltip("Stop");

			Gui::EndDisabled();
		}

		std::string ascPath = audioSource->GetPath().string();
		std::string relativePath = "(null)";
		if (!ascPath.empty())
		{
			relativePath = FileSystem::Relative(ascPath, Project::GetAssetDirectory()).stem().string();
		}

		auto OnAudioSourceDroppedFn = [&](const Fs::Path& filepath) {
			// Make sure we are recieving an actual audio file otherwise we will have trouble opening it
			if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath); type == AssetType::AudioAsset)
			{
				if (device.GetSound().IsPlaying())
				{
					device.Stop();
				}

				if (FileSystem::GetFileExtension(filepath) != ".vsound")
				{
					std::string filename = FileSystem::RemoveFileExtension(filepath);
					filename += ".vsound";
					SharedReference<AudioSource> asset = Project::GetEditorAssetManager()->CreateNewAsset<AudioSource>("Audio", filename, filepath);
					component.AudioHandle = asset->Handle;
				}
				else
				{
					AssetHandle handle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);
					component.AudioHandle = handle;
				}
			}
			else
			{
				VX_CONSOLE_LOG_WARN("Failed to audio source '{}'", filepath.filename().string());
			}
		};

		UI::BeginPropertyGrid();
		if (UI::PropertyAssetReference<AudioSource>("Source", relativePath, component.AudioHandle, OnAudioSourceDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry()))
		{
			if (device.GetSound().IsPlaying())
			{
				device.Stop();
			}

			const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.AudioHandle);
			const std::string metadataFilepath = metadata.Filepath.string();

			if (component.AudioHandle != 0 && !metadataFilepath.ends_with(".vsound"))
			{
				// TODO
				// we need to check if theres another vsound before creating one, we don't need to keep creating them if one already exists
				std::string filename = FileSystem::RemoveFileExtension(metadata.Filepath);
				filename += ".vsound";
				audioSource = Project::GetEditorAssetManager()->CreateNewAsset<AudioSource>("Audio", filename, metadata.Filepath);
				component.AudioHandle = audioSource->Handle;
			}

			// Asset cleared
			if (component.AudioHandle == 0)
			{
				component.AudioHandle = AssetManager::CreateMemoryOnlyAsset<AudioSource>();
			}
		}
		UI::EndPropertyGrid();

		Gui::BeginDisabled(audioSource == nullptr);

		if (validEngineID && audioSource)
		{
			UI::BeginPropertyGrid();

			float pitch = device.GetSound().GetPitch();
			if (UI::Property("Pitch", pitch, 0.01f, FLT_MIN, FLT_MAX))
				device.GetSound().SetPitch(pitch);

			float volume = device.GetSound().GetVolume();
			if (UI::Property("Volume", volume, 0.1f, FLT_MIN, FLT_MAX))
				device.GetSound().SetVolume(volume);

			UI::Property("Play On Start", component.PlayOnStart);
			UI::Property("Play One Shot", component.PlayOneShot);

			bool loop = device.GetSound().IsLooping();
			if (UI::Property("Loop", loop))
				device.GetSound().SetLooping(loop);

			bool spacialized = device.GetSound().IsSpacialized();
			if (UI::Property("Spacialized", spacialized))
				device.GetSound().SetSpacialized(spacialized);

			UI::EndPropertyGrid();

			const bool isSpacialized = device.GetSound().IsSpacialized();

			if (isSpacialized && UI::PropertyGridHeader("Sound Transform", false))
			{
				UI::BeginPropertyGrid();

				Math::vec3 position = Utils::FromWaveVector(device.GetSound().GetPosition());
				UI::DrawVec3Controls("Position", position, 0.0f, 100.0f, 0.0f, 0.0f, [&]()
				{
					device.GetSound().SetPosition(Utils::ToWaveVector(position));
				});

				Math::vec3 direction = Utils::FromWaveVector(device.GetSound().GetDirection());
				UI::DrawVec3Controls("Direction", direction, 0.0f, 100.0f, 0.0f, 0.0f, [&]()
				{
					device.GetSound().SetDirection(Utils::ToWaveVector(direction));
				});

				Math::vec3 velocity = Utils::FromWaveVector(device.GetSound().GetVelocity());
				UI::DrawVec3Controls("Veloctiy", velocity, 0.0f, 100.0f, 0.0f, 0.0f, [&]()
				{
					device.GetSound().SetVelocity(Utils::ToWaveVector(velocity));
				});

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}

			if (isSpacialized && UI::PropertyGridHeader("Spacialization", false))
			{
				UI::BeginPropertyGrid();

				float minGain = device.GetSound().GetMinGain();
				if (UI::Property("Min Gain", minGain, 0.1f, FLT_MIN, FLT_MAX))
					device.GetSound().SetMinGain(minGain);

				float maxGain = device.GetSound().GetMaxGain();
				if (UI::Property("Max Gain", maxGain, 0.1f, FLT_MIN, FLT_MAX))
					device.GetSound().SetMaxGain(maxGain);

				static const char* attenuationModels[] = { "None", "Inverse", "Linear", "Exponential" };

				AttenuationModel currentAttenuationModel = Utils::FromWaveAttenuationModel(device.GetSound().GetAttenuationModel());
				if (UI::PropertyDropdown("Attenuation Model", attenuationModels, VX_ARRAYSIZE(attenuationModels), currentAttenuationModel))
					device.GetSound().SetAttenuationModel(Utils::ToWaveAttenuationModel(currentAttenuationModel));

				float pan = device.GetSound().GetPan();
				if (UI::Property("Pan", pan, 0.1f, FLT_MIN, FLT_MAX))
					device.GetSound().SetPan(pan);

				static const char* panModes[] = { "Balance", "Pan" };

				PanMode currentPanMode = Utils::FromWavePanMode(device.GetSound().GetPanMode());
				if (UI::PropertyDropdown("Pan Mode", panModes, VX_ARRAYSIZE(panModes), currentPanMode))
					device.GetSound().SetPanMode(Utils::ToWavePanMode(currentPanMode));

				static const char* positioningModes[] = { "Absolute", "Relative" };

				PositioningMode currentPositioningMode = Utils::FromWavePositioningMode(device.GetSound().GetPositioning());
				if (UI::PropertyDropdown("Positioning Mode", positioningModes, VX_ARRAYSIZE(positioningModes), currentPositioningMode))
					device.GetSound().SetPositioning(Utils::ToWavePositioningMode(currentPositioningMode));

				float falloff = device.GetSound().GetFalloff();
				if (UI::Property("Falloff", falloff, 0.1f, FLT_MIN, FLT_MAX))
					device.GetSound().SetFalloff(falloff);

				float minDistance = device.GetSound().GetMinDistance();
				if (UI::Property("Min Distance", minDistance, 0.1f, FLT_MIN, FLT_MAX))
					device.GetSound().SetMinDistance(minDistance);

				float maxDistance = device.GetSound().GetMaxDistance();
				if (UI::Property("Max Distance", maxDistance, 0.1f, FLT_MIN, FLT_MAX))
					device.GetSound().SetMaxDistance(maxDistance);

				float dopplerFactor = device.GetSound().GetDopplerFactor();
				if (UI::Property("Doppler Factor", dopplerFactor, 0.01f, FLT_MIN, FLT_MAX))
					device.GetSound().SetDopplerFactor(dopplerFactor);

				float directionalAttenuationFactor = device.GetSound().GetDirectionalAttenuationFactor();
				if (UI::Property("Directional Attenuation Factor", directionalAttenuationFactor))
					device.GetSound().SetDirectionalAttenuationFactor(directionalAttenuationFactor);

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}

			if (isSpacialized && UI::PropertyGridHeader("Cone", false))
			{
				UI::BeginPropertyGrid();
				Wave::AudioCone cone = device.GetSound().GetAudioCone();

				bool modified = false;

				float innerAngle = Math::Rad2Deg(cone.InnerAngle);
				if (UI::Property("Inner Angle", innerAngle, 1.0f, FLT_MIN, FLT_MAX))
				{
					cone.InnerAngle = innerAngle;
					modified = true;
				}

				float outerAngle = Math::Rad2Deg(cone.OuterAngle);
				if (UI::Property("Outer Angle", outerAngle, 1.0f, FLT_MIN, FLT_MAX))
				{
					cone.OuterAngle = outerAngle;
					modified = true;
				}

				float outerGain = cone.OuterGain;
				if (UI::Property("Outer Gain", outerGain, 1.0f, FLT_MIN, FLT_MAX))
				{
					cone.OuterGain = outerGain;
					modified = true;
				}

				if (modified) {
					device.GetSound().SetAudioCone(cone);
				}

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}

			if (!audioSource->GetPath().empty() && UI::PropertyGridHeader("Debug", false))
			{
				UI::BeginPropertyGrid();

				Gui::BeginDisabled(true);

				Math::vec3 directionToListener = Utils::FromWaveVector(device.GetSound().GetDirectionToListener());
				UI::Property("Direction To Listener", directionToListener);

				float currentFadeVolume = device.GetSound().GetCurrentFadeVolume();
				UI::Property("Current Fade Volume", currentFadeVolume);

				uint64_t one = device.GetSound().GetTimeInMilliseconds();
				UI::Property("Time in ms", one);
				uint64_t two = device.GetSound().GetTimeInPCMFrames();
				UI::Property("Time in pcm frames", two);

				uint32_t listenerIndex = device.GetSound().GetListenerIndex();
				UI::Property("Listener Index", listenerIndex);

				uint32_t pinnedListenerIndex = device.GetSound().GetPinnedListenerIndex();
				UI::Property("Pinned Index", pinnedListenerIndex);

				static bool showInSecondsNotPCMFrames = true;

				Gui::EndDisabled();
				std::string label = showInSecondsNotPCMFrames ? "Show PCM Frames" : "Show Seconds";
				UI::Property(label.c_str(), showInSecondsNotPCMFrames);
				Gui::BeginDisabled(true);
					
				if (showInSecondsNotPCMFrames)
				{
					float lengthInSeconds = device.GetSound().GetLengthInSeconds();
					UI::Property("Length", lengthInSeconds);

					float cursorInSeconds = device.GetSound().GetCursorInSeconds();
					UI::Property("Cursor", cursorInSeconds);
				}
				else
				{
					uint64_t lengthInPCMFrames = device.GetSound().GetLengthInPCMFrames();
					UI::Property("Length", lengthInPCMFrames);

					uint64_t cursorInPCMFrames = device.GetSound().GetCursorInPCMFrames();
					UI::Property("Cursor", cursorInPCMFrames);
				}

				Gui::EndDisabled();

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}
		}

		Gui::EndDisabled();
	}

	void SceneHierarchyPanel::AudioListenerComponentOnGuiRender(AudioListenerComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::RigidBodyComponentOnGuiRender(RigidBodyComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		const char* bodyTypes[] = { "Static", "Dynamic" };
		int32_t currentBodyType = (int32_t)component.Type;
		if (UI::PropertyDropdown("Body Type", bodyTypes, VX_ARRAYSIZE(bodyTypes), currentBodyType))
		{
			const bool bodyTypeChanged = component.Type != (RigidBodyType)currentBodyType;
			if (bodyTypeChanged)
			{
				component.Type = (RigidBodyType)currentBodyType;

				const bool isSimulating = PhysicsScene::GetScene();
				if (isSimulating)
				{
					Physics::ReCreateActor(actor);
				}
			}
		}

		bool modified = false;

		if (component.Type == RigidBodyType::Static)
		{
			UI::EndPropertyGrid();
		}
		else
		{
			if (UI::Property("Mass", component.Mass, 0.01f, FLT_MIN, FLT_MAX))
				modified = true;
			UI::Property("Linear Velocity", component.LinearVelocity);
			UI::Property("Max Linear Velocity", component.MaxLinearVelocity, 1.0f, FLT_MIN, FLT_MAX);
			UI::Property("Linear Drag", component.LinearDrag, 0.01f, FLT_MIN, FLT_MAX);
			UI::Property("Angular Velocity", component.AngularVelocity);
			UI::Property("Max Angular Velocity", component.MaxAngularVelocity, 1.0f, FLT_MIN, FLT_MAX);
			UI::Property("Angular Drag", component.AngularDrag, 0.01f, FLT_MIN, FLT_MAX, "%.2f");

			if (UI::Property("Disable Gravity", component.DisableGravity))
				modified = true;

			if (UI::Property("IsKinematic", component.IsKinematic))
				modified = true;

			const char* collisionDetectionTypes[] = { "Discrete", "Continuous", "Continuous Speclative" };
			int32_t currentCollisionDetectionType = (uint32_t)component.CollisionDetection;
			if (UI::PropertyDropdown("Collision Detection", collisionDetectionTypes, VX_ARRAYSIZE(collisionDetectionTypes), currentCollisionDetectionType))
			{
				component.CollisionDetection = (CollisionDetectionType)currentCollisionDetectionType;
				modified = true;
			}

			UI::EndPropertyGrid();

			if (UI::PropertyGridHeader("Constraints", false))
			{
				UI::BeginPropertyGrid();

				bool translationX = (component.LockFlags & (uint8_t)ActorLockFlag::TranslationX);
				bool translationY = (component.LockFlags & (uint8_t)ActorLockFlag::TranslationY);
				bool translationZ = (component.LockFlags & (uint8_t)ActorLockFlag::TranslationZ);
				bool rotationX = (component.LockFlags & (uint8_t)ActorLockFlag::RotationX);
				bool rotationY = (component.LockFlags & (uint8_t)ActorLockFlag::RotationY);
				bool rotationZ = (component.LockFlags & (uint8_t)ActorLockFlag::RotationZ);

				Gui::Text("Freeze Position");
				Gui::NextColumn();
				Gui::PushItemWidth(-1);

				Gui::Text("X");
				Gui::SameLine();

				if (Gui::Checkbox("##TranslationX", &translationX))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::TranslationX;
					modified = true;
				}
				UI::DrawItemActivityOutline();
				Gui::SameLine();

				Gui::Text("Y");
				Gui::SameLine();

				if (Gui::Checkbox("##TranslationY", &translationY))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::TranslationY;
					modified = true;
				}
				UI::DrawItemActivityOutline();
				Gui::SameLine();

				Gui::Text("Z");
				Gui::SameLine();

				if (Gui::Checkbox("##TranslationZ", &translationZ))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::TranslationZ;
					modified = true;
				}
				UI::DrawItemActivityOutline();

				Gui::PopItemWidth();
				Gui::NextColumn();
				UI::Draw::Underline();

				Gui::Text("Freeze Rotation");
				Gui::NextColumn();
				Gui::PushItemWidth(-1);

				Gui::Text("X");
				Gui::SameLine();

				if (Gui::Checkbox("##RotationX", &rotationX))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::RotationX;
					modified = true;
				}
				UI::DrawItemActivityOutline();
				Gui::SameLine();

				Gui::Text("Y");
				Gui::SameLine();

				if (Gui::Checkbox("##RotationY", &rotationY))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::RotationY;
					modified = true;
				}
				UI::DrawItemActivityOutline();
				Gui::SameLine();

				Gui::Text("Z");
				Gui::SameLine();

				if (Gui::Checkbox("##RotationZ", &rotationZ))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::RotationZ;
					modified = true;
				}
				UI::DrawItemActivityOutline();

				Gui::PopItemWidth();
				Gui::NextColumn();
				UI::Draw::Underline();

				UI::EndPropertyGrid();
				UI::EndTreeNode();

			}

			if (modified)
			{
				const bool isSimulating = PhysicsScene::GetScene();
				if (isSimulating)
				{
					Physics::WakeUpActor(actor);
				}
			}
		}
	}

	void SceneHierarchyPanel::CharacterControllerComponentOnGuiRender(CharacterControllerComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Slope Limit", component.SlopeLimitDegrees, 1.0f, FLT_MIN, FLT_MAX);
		UI::Property("Step Offset", component.StepOffset, 1.0f, FLT_MIN, FLT_MAX);
		UI::Property("Contact Offset", component.ContactOffset, 1.0f, FLT_MIN, FLT_MAX);
		UI::Property("Disable Gravity", component.DisableGravity);

		Gui::BeginDisabled();
		UI::Property("Speed Down", component.SpeedDown);
		Gui::EndDisabled();

		const char* nonWalkableModes[] = { "Prevent Climbing", "Prevent Climbing and Force Sliding" };
		int32_t currentNonWalkableMode = (uint32_t)component.NonWalkMode;
		if (UI::PropertyDropdown("Non Walkable Mode", nonWalkableModes, VX_ARRAYSIZE(nonWalkableModes), currentNonWalkableMode))
			component.NonWalkMode = (NonWalkableMode)currentNonWalkableMode;

		if (actor.HasComponent<CapsuleColliderComponent>())
		{
			const char* climbModes[] = { "Easy", "Constrained" };
			int32_t currentClimbMode = (uint32_t)component.ClimbMode;
			if (UI::PropertyDropdown("Capsule Climb Mode", climbModes, VX_ARRAYSIZE(climbModes), currentClimbMode))
				component.ClimbMode = (CapsuleClimbMode)currentClimbMode;
		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::FixedJointComponentOnGuiRender(FixedJointComponent& component, Actor actor)
	{
		std::string connectedActorName = "Null";
		if (Actor connectedActor = m_ContextScene->TryGetActorWithUUID(component.ConnectedActor))
		{
			connectedActorName = connectedActor.Name();
		}

		UI::BeginPropertyGrid();
		UI::Property("Connected Actor", connectedActorName, true);
		UI::EndPropertyGrid();

		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
			{
				Actor& droppedActor = *((Actor*)payload->Data);
				component.ConnectedActor = droppedActor.GetUUID();
			}

			Gui::EndDragDropTarget();
		}

		UI::BeginPropertyGrid();

		UI::Property("Is Breakable", component.IsBreakable);

		if (component.IsBreakable)
		{
			UI::Property("Break Force", component.BreakForce, 1.0f, FLT_MIN, FLT_MAX);
			UI::Property("Break Torque", component.BreakTorque, 1.0f, FLT_MIN, FLT_MAX);
		}

		UI::Property("Enable Collision", component.EnableCollision);
		UI::Property("Enable PreProcessing", component.EnablePreProcessing);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::BoxColliderComponentOnGuiRender(BoxColliderComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Half Size", component.HalfSize, 0.01f, FLT_MIN, FLT_MAX);
		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Visible", component.Visible);
		UI::Property("Is Trigger", component.IsTrigger);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::SphereColliderComponentOnGuiRender(SphereColliderComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Radius", component.Radius, 0.01f, FLT_MIN, FLT_MAX);
		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Visible", component.Visible);
		UI::Property("Is Trigger", component.IsTrigger);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::CapsuleColliderComponentOnGuiRender(CapsuleColliderComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Radius", component.Radius, 0.01f, FLT_MIN, FLT_MAX);
		UI::Property("Height", component.Height, 0.01f, FLT_MIN, FLT_MAX);
		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Visible", component.Visible);
		UI::Property("Is Trigger", component.IsTrigger);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::MeshColliderComponentOnGuiRender(MeshColliderComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		static const char* collisionComplexities[] = { "Default", "Use Complex as Simple", "Use Simple as Complex" };
		uint32_t currentCollisionComplexity = (uint32_t)component.CollisionComplexity;

		if (UI::PropertyDropdown("Collision Complexity", collisionComplexities, VX_ARRAYSIZE(collisionComplexities), currentCollisionComplexity))
		{
			component.CollisionComplexity = (ECollisionComplexity)currentCollisionComplexity;
		}

		UI::Property("Visible", component.Visible);
		UI::Property("Is Trigger", component.IsTrigger);
		UI::Property("Use Shared Shape", component.UseSharedShape);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::RigidBody2DComponentOnGuiRender(RigidBody2DComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
		int32_t currentBodyType = (uint32_t)component.Type;
		if (UI::PropertyDropdown("Body Type", bodyTypes, VX_ARRAYSIZE(bodyTypes), currentBodyType))
		{
			component.Type = (RigidBody2DType)currentBodyType;
		}

		if (component.Type == RigidBody2DType::Dynamic)
		{
			UI::Property("Velocity", component.Velocity, 0.01f);
			UI::Property("Drag", component.Drag, 0.01f, FLT_MIN, 1.0f);
			UI::Property("Angular Velocity", component.AngularVelocity);
			UI::Property("Angular Drag", component.AngularDrag, 0.01f, FLT_MIN, 1.0f);
			UI::Property("Gravity Scale", component.GravityScale, 0.01f, FLT_MIN, 1.0f);
			UI::Property("Freeze Rotation", component.FixedRotation);
		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::BoxCollider2DComponentOnGuiRender(BoxCollider2DComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Size", component.Size, 0.01f);
		UI::Property("Density", component.Density, 0.01f, FLT_MIN, 1.0f);
		UI::Property("Friction", component.Friction, 0.01f, FLT_MIN, 1.0f);
		UI::Property("Restitution", component.Restitution, 0.01f, FLT_MIN, 1.0f);
		UI::Property("Threshold", component.RestitutionThreshold, 0.1f, FLT_MIN, FLT_MAX);
		UI::Property("Visible", component.Visible);
		UI::Property("Is Tigger", component.IsTrigger);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::CircleCollider2DComponentOnGuiRender(CircleCollider2DComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Radius", component.Radius, 0.01, FLT_MIN, FLT_MAX);
		UI::Property("Density", component.Density, 0.01f, FLT_MIN, 1.0f);
		UI::Property("Friction", component.Friction, 0.01f, FLT_MIN, 1.0f);
		UI::Property("Restitution", component.Restitution, 0.01f, FLT_MIN, 1.0f);
		UI::Property("Threshold", component.RestitutionThreshold, 0.1f, FLT_MIN, FLT_MAX);
		UI::Property("Visible", component.Visible);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::NavMeshAgentComponentOnGuiRender(NavMeshAgentComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::ScriptComponentOnGuiRender(ScriptComponent& component, Actor actor)
	{
		UI::BeginPropertyGrid();

		UI::Property("Enabled", component.Enabled);

		std::vector<const char*> actorClassNameStrings;
		const bool scriptClassExists = ScriptEngine::ScriptClassExists(component.ClassName);

		const std::unordered_map<std::string, SharedReference<ScriptClass>>& actorClasses = ScriptEngine::GetScriptClasses();

		for (const auto& [className, actorScriptClass] : actorClasses)
		{
			actorClassNameStrings.push_back(className.c_str());
		}

		std::string currentClassName = component.ClassName.empty() ? "(null)" : component.ClassName;

		bool renderFields = true;

		auto OnClassClearedFn = [&]() {
			component.ClassName.clear();
			renderFields = false;
		};

		// Display available actor classes to choose from
		if (UI::PropertyDropdownSearch("Class", actorClassNameStrings.data(), actorClassNameStrings.size(), currentClassName, m_ActorClassNameInputTextFilter, OnClassClearedFn))
		{
			component.ClassName = currentClassName;
		}

		if (renderFields == false) { // We can't render fields the class name was cleared
			UI::EndPropertyGrid(); // cleanup
			return;
		}

		const bool sceneRunning = m_ContextScene->IsRunning();

		Gui::BeginDisabled(!component.Enabled);

		// Fields
		if (sceneRunning)
		{
			SharedReference<ScriptInstance> scriptInstance = ScriptEngine::GetScriptInstance(actor.GetUUID());

			if (scriptInstance)
			{
				const std::map<std::string, ScriptField>& fields = scriptInstance->GetScriptClass()->GetFields();

				for (const auto& [name, field] : fields)
				{
					switch (field.Type)
					{
						case ScriptFieldType::Float:
						{
							float data = scriptInstance->GetFieldValue<float>(name);
							if (UI::Property(name.c_str(), data, 0.01f))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Double:
						{
							double data = scriptInstance->GetFieldValue<double>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Vector2:
						{
							Math::vec2 data = scriptInstance->GetFieldValue<Math::vec2>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Vector3:
						{
							Math::vec3 data = scriptInstance->GetFieldValue<Math::vec3>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Vector4:
						{
							Math::vec4 data = scriptInstance->GetFieldValue<Math::vec4>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Color3:
						{
							Math::vec3 data = scriptInstance->GetFieldValue<Math::vec3>(name);
							if (UI::Property(name.c_str(), &data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Color4:
						{
							Math::vec4 data = scriptInstance->GetFieldValue<Math::vec4>(name);
							if (UI::Property(name.c_str(), &data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Bool:
						{
							bool data = scriptInstance->GetFieldValue<bool>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Char:
						{
							char data = scriptInstance->GetFieldValue<char>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::String:
						{
							// TODO
							break;
						}
						case ScriptFieldType::Short:
						{
							short data = scriptInstance->GetFieldValue<short>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Int:
						{
							int data = scriptInstance->GetFieldValue<int>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Long:
						{
							long long data = scriptInstance->GetFieldValue<long long>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Byte:
						{
							unsigned char data = scriptInstance->GetFieldValue<unsigned char>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::UShort:
						{
							unsigned short data = scriptInstance->GetFieldValue<unsigned short>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::UInt:
						{
							unsigned int data = scriptInstance->GetFieldValue<unsigned int>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::ULong:
						{
							unsigned long long data = scriptInstance->GetFieldValue<unsigned long long>(name);
							if (UI::Property(name.c_str(), data))
								scriptInstance->SetFieldValue(name, data);
							break;
						}
						case ScriptFieldType::Actor:
						{
							uint64_t data = scriptInstance->GetFieldValue<uint64_t>(name);
							if (UI::Property(name.c_str(), data))
							{
								scriptInstance->SetFieldValue(name, data);
							}

							UI::EndPropertyGrid();

							if (Gui::BeginDragDropTarget())
							{
								if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
								{
									Actor& droppedActor = *((Actor*)payload->Data);
									scriptInstance->SetFieldValue(name, data);
								}

								Gui::EndDragDropTarget();
							}

							UI::BeginPropertyGrid();
							break;
						}
						case ScriptFieldType::AssetHandle:
						{
							uint64_t data = scriptInstance->GetFieldValue<uint64_t>(name);
							if (UI::Property(name.c_str(), data))
							{
								scriptInstance->SetFieldValue(name, data);
							}

							UI::EndPropertyGrid();

							if (Gui::BeginDragDropTarget())
							{
								// TODO
								/*if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
								{
									Actor& droppedActor = *((Actor*)payload->Data);
									scriptInstance->SetFieldValue(name, data);
								}*/

								Gui::EndDragDropTarget();
							}

							UI::BeginPropertyGrid();
							break;
						}
					}
				}
			}
		}
		else
		{
			if (scriptClassExists)
			{
				SharedReference<ScriptClass> actorClass = ScriptEngine::GetScriptClass(component.ClassName);
				const std::map<std::string, ScriptField>& fields = actorClass->GetFields();

				ScriptFieldMap& actorScriptFields = ScriptEngine::GetMutableScriptFieldMap(actor);

				for (const auto& [name, field] : fields)
				{
					auto it = actorScriptFields.find(name);

					// Field has been set in editor
					if (it != actorScriptFields.end())
					{
						ScriptFieldInstance& scriptField = it->second;

						// Display controls to set it
						if (field.Type == ScriptFieldType::Float)
						{
							float data = scriptField.GetValue<float>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Double)
						{
							double data = scriptField.GetValue<double>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Vector2)
						{
							Math::vec2 data = scriptField.GetValue<Math::vec2>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Vector3)
						{
							Math::vec3 data = scriptField.GetValue<Math::vec3>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Vector4)
						{
							Math::vec4 data = scriptField.GetValue<Math::vec4>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Color3)
						{
							Math::vec3 data = scriptField.GetValue<Math::vec3>();
							if (UI::Property(name.c_str(), &data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Color4)
						{
							Math::vec4 data = scriptField.GetValue<Math::vec4>();
							if (UI::Property(name.c_str(), &data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Bool)
						{
							bool data = scriptField.GetValue<bool>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Char)
						{
							char data = scriptField.GetValue<char>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Short)
						{
							short data = scriptField.GetValue<short>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Int)
						{
							int data = scriptField.GetValue<int>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Long)
						{
							long long data = scriptField.GetValue<long long>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Byte)
						{
							unsigned char data = scriptField.GetValue<unsigned char>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::UShort)
						{
							unsigned short data = scriptField.GetValue<unsigned short>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::UInt)
						{
							unsigned int data = scriptField.GetValue<unsigned int>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::ULong)
						{
							unsigned long long data = scriptField.GetValue<unsigned long long>();
							if (UI::Property(name.c_str(), data))
								scriptField.SetValue(data);
						}
						if (field.Type == ScriptFieldType::Actor)
						{
							uint64_t data = scriptField.GetValue<uint64_t>();
							if (UI::Property(name.c_str(), data))
							{
								scriptField.SetValue(data);
							}

							UI::EndPropertyGrid();

							if (Gui::BeginDragDropTarget())
							{
								if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
								{
									Actor& droppedActor = *((Actor*)payload->Data);
									scriptField.SetValue(droppedActor.GetUUID());
								}

								Gui::EndDragDropTarget();
							}

							UI::BeginPropertyGrid();
						}
						if (field.Type == ScriptFieldType::AssetHandle)
						{
							uint64_t data = scriptField.GetValue<uint64_t>();
							if (UI::Property(name.c_str(), data))
							{
								scriptField.SetValue(data);
							}

							UI::EndPropertyGrid();

							if (Gui::BeginDragDropTarget())
							{
								// TODO
								/*if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
								{
									Actor& droppedActor = *((Actor*)payload->Data);
									scriptField.SetValue(droppedActor.GetUUID());
								}*/

								Gui::EndDragDropTarget();
							}

							UI::BeginPropertyGrid();
						}
					}
					else
					{
						// Display controls to set it
						if (field.Type == ScriptFieldType::Float)
						{
							float data = 0.0f;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Double)
						{
							double data = 0.0f;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Vector2)
						{
							Math::vec2 data = Math::vec2(0.0f);
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Vector3)
						{
							Math::vec3 data = Math::vec3(0.0f);
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Vector4)
						{
							Math::vec4 data = Math::vec4(0.0f);
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Color3)
						{
							Math::vec3 data = Math::vec3(0.0f);
							if (UI::Property(name.c_str(), &data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Color4)
						{
							Math::vec4 data = Math::vec4(0.0f, 0.0f, 0.0f, 1.0f);
							if (UI::Property(name.c_str(), &data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Bool)
						{
							bool data = false;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Char)
						{
							char data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Short)
						{
							short data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Int)
						{
							int data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Long)
						{
							long long data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Byte)
						{
							unsigned char data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::UShort)
						{
							unsigned short data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::UInt)
						{
							unsigned int data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::ULong)
						{
							unsigned long long data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Actor)
						{
							uint64_t data;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}

							UI::EndPropertyGrid();

							if (Gui::BeginDragDropTarget())
							{
								if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
								{
									Actor& droppedActor = *((Actor*)payload->Data);
									ScriptFieldInstance fieldInstance = actorScriptFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(droppedActor.GetUUID());
								}

								Gui::EndDragDropTarget();
							}

							UI::BeginPropertyGrid();
						}
						if (field.Type == ScriptFieldType::AssetHandle)
						{
							uint64_t data;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance fieldInstance = actorScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}

							UI::EndPropertyGrid();

							if (Gui::BeginDragDropTarget())
							{
								// TODO
								/*if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
								{
									Actor& droppedActor = *((Actor*)payload->Data);
									ScriptFieldInstance fieldInstance = actorScriptFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(droppedActor.GetUUID());
								}*/

								Gui::EndDragDropTarget();
							}

							UI::BeginPropertyGrid();
						}
					}
				}
			}
		}

		Gui::EndDisabled();

		UI::EndPropertyGrid();
	}

}
