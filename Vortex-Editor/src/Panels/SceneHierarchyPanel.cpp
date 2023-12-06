#include "SceneHierarchyPanel.h"

#include <Vortex/Core/Buffer.h>

#include <Vortex/Audio/AudioUtils.h>

#include <Vortex/Scripting/ScriptEngine.h>
#include <Vortex/Editor/EditorResources.h>

#include <imgui_internal.h>

#include <codecvt>

namespace Vortex {

#define MAX_MARKER_SIZE 64

#define MAX_CHILD_ENTITY_SEARCH_DEPTH 10

	SceneHierarchyPanel::SceneHierarchyPanel(const SharedReference<Scene>& context)
	{
		SetSceneContext(context);

		// Create copy entity here so it will live the lifetime of the scene hierarchy panel
		// This will allow us to copy components throughout different scenes
		m_CopyScene = Scene::Create();
		m_CopyEntity = m_CopyScene->CreateEntity("Copy Entity");
	}

	void SceneHierarchyPanel::OnGuiRender(Entity hoveredEntity, const EditorCamera* editorCamera)
	{
		if (IsOpen)
		{
			Gui::Begin(m_PanelName.c_str(), &IsOpen);

			ImRect windowRect = ImRect(Gui::GetWindowContentRegionMin(), Gui::GetWindowContentRegionMax());

			// Search Bar + Filtering
			Gui::SetNextItemWidth(Gui::GetContentRegionAvail().x - Gui::CalcTextSize((const char*)VX_ICON_PLUS).x * 2.0f - 4.0f);
			const bool isSearching = Gui::InputTextWithHint("##EntitySearch", "Search...", m_EntitySearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_EntitySearchInputTextFilter.InputBuf));
			if (isSearching)
				m_EntitySearchInputTextFilter.Build();

			Gui::SameLine();

			UI::ShiftCursorX(-4.0f);
			if (Gui::Button((const char*)VX_ICON_PLUS, { 30.0f, 0.0f }))
				Gui::OpenPopup("CreateEntity");

			if (Gui::BeginPopup("CreateEntity"))
			{
				DisplayCreateEntityMenu(editorCamera);

				Gui::EndPopup();
			}

			Gui::Spacing();
			UI::Draw::Underline();

			if (m_ContextScene)
			{
				uint32_t searchDepth = 0;
				const bool isSearching = strlen(m_EntitySearchInputTextFilter.InputBuf) != 0;
				std::vector<UUID> rootEntitiesInHierarchy;

				m_ContextScene->m_Registry.each([&](auto entityID)
				{
					const Entity entity{ entityID, m_ContextScene.Raw() };
					
					if (!entity)
						return;

					const bool isChildEntity = entity.GetParentUUID() != 0;

					if (isChildEntity)
						return;

					rootEntitiesInHierarchy.push_back(entity.GetUUID());

					const bool matchingSearch = m_EntitySearchInputTextFilter.PassFilter(entity.GetName().c_str());
					
					if (!matchingSearch)
						return;

					DrawEntityNode(entity, editorCamera);
				});

				if (isSearching)
				{
					for (const auto& rootEntity : rootEntitiesInHierarchy)
					{
						RecursiveEntitySearch(rootEntity, editorCamera, searchDepth);
					}
				}

				if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
				{
					const auto flags = ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM", flags);

					if (payload)
					{
						Entity& entity = *(Entity*)payload->Data;
						m_ContextScene->UnparentEntity(entity);
					}

					ImGui::EndDragDropTarget();
				}

				// Left click anywhere on the panel to deselect entity
				if (Gui::IsMouseDown(0) && Gui::IsWindowHovered())
				{
					SelectionManager::DeselectEntity();
					m_EntityShouldBeRenamed = false;
					m_EntityShouldBeDestroyed = false;
				}

				// Right-click on blank space in scene hierarchy panel
				if (Gui::BeginPopupContextWindow(0, 1, false))
				{
					DisplayCreateEntityMenu(editorCamera);

					Gui::EndPopup();
				}

				Entity selected = SelectionManager::GetSelectedEntity();

				// destroy if requested
				if (m_EntityShouldBeDestroyed && selected)
				{
					Entity entity = selected;

					SelectionManager::DeselectEntity();
					m_EntityShouldBeRenamed = false;
					m_EntityShouldBeDestroyed = false;

					m_ContextScene->SubmitToDestroyEntity(entity);
				}
			}

			Gui::End();
		}

		if (s_ShowInspectorPanel)
		{
			DisplayInsectorPanel(hoveredEntity);
		}
	}

	void SceneHierarchyPanel::RecursiveEntitySearch(UUID rootEntity, const EditorCamera* editorCamera, uint32_t& searchDepth)
	{
		if (searchDepth > MAX_CHILD_ENTITY_SEARCH_DEPTH)
			return;

		const Entity entity = m_ContextScene->TryGetEntityWithUUID(rootEntity);

		if (!entity || entity.Children().empty())
			return;

		const auto& children = entity.Children();

		for (const auto& childUUID : children)
		{
			const Entity child = m_ContextScene->TryGetEntityWithUUID(childUUID);

			if (!child)
				continue;

			const std::string& name = child.GetName();

			if (m_EntitySearchInputTextFilter.PassFilter(name.c_str()))
			{
				DrawEntityNode(child, editorCamera);
			}

			searchDepth++;

			RecursiveEntitySearch(child.GetUUID(), editorCamera, searchDepth);
		}
	}

	void SceneHierarchyPanel::SetSceneContext(SharedReference<Scene> scene)
	{
		m_ContextScene = scene;
		SelectionManager::DeselectEntity();
		m_EntityShouldBeRenamed = false;
		m_EntityShouldBeDestroyed = false;

		// Clear all search bars
		memset(m_EntitySearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_EntitySearchInputTextFilter.InputBuf));
		m_EntitySearchInputTextFilter.Build();

		memset(m_EntityClassNameInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_EntityClassNameInputTextFilter.InputBuf));
		m_EntityClassNameInputTextFilter.Build();

		memset(m_ComponentSearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
		m_ComponentSearchInputTextFilter.Build();
	}

	inline static Entity CreateDefaultMesh(const std::string& entityName, DefaultMesh::StaticMeshType defaultMesh, SharedReference<Scene>& contextScene, const EditorCamera* editorCamera)
	{
		Entity entity = contextScene->CreateEntity(entityName);
		StaticMeshRendererComponent& staticMeshRendererComponent = entity.AddComponent<StaticMeshRendererComponent>();
		staticMeshRendererComponent.Type = static_cast<MeshType>(defaultMesh);
		staticMeshRendererComponent.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(defaultMesh);
		entity.GetTransform().Translation = editorCamera->GetFocalPoint();

		entity.AddComponent<RigidBodyComponent>();

		switch (defaultMesh)
		{
			case DefaultMesh::StaticMeshType::Cube:     entity.AddComponent<BoxColliderComponent>();     break;
			case DefaultMesh::StaticMeshType::Sphere:   entity.AddComponent<SphereColliderComponent>();  break;
			case DefaultMesh::StaticMeshType::Capsule:  entity.AddComponent<CapsuleColliderComponent>(); break;
			case DefaultMesh::StaticMeshType::Cone:     entity.AddComponent<MeshColliderComponent>();    break;
			case DefaultMesh::StaticMeshType::Cylinder: entity.AddComponent<MeshColliderComponent>();    break;
			case DefaultMesh::StaticMeshType::Plane:    entity.AddComponent<MeshColliderComponent>();    break;
			case DefaultMesh::StaticMeshType::Torus:    entity.AddComponent<MeshColliderComponent>();    break;
		}

		SelectionManager::SetSelectedEntity(entity);

		return entity;
	}

	void SceneHierarchyPanel::DisplayCreateEntityMenu(const EditorCamera* editorCamera)
	{
		if (Gui::MenuItem("Create Empty"))
		{
			SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Empty Entity"));
			SelectionManager::GetSelectedEntity().GetTransform().Translation = editorCamera->GetFocalPoint();
		}
		UI::Draw::Underline();
		Gui::Spacing();
		
		Gui::Text((const char*)VX_ICON_CUBE);
		Gui::SameLine();
		if (Gui::BeginMenu("Create 3D"))
		{
			if (Gui::MenuItem("Cube"))
			{
				CreateDefaultMesh("Cube", DefaultMesh::StaticMeshType::Cube, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Sphere"))
			{
				CreateDefaultMesh("Sphere", DefaultMesh::StaticMeshType::Sphere, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Capsule"))
			{
				CreateDefaultMesh("Capsule", DefaultMesh::StaticMeshType::Capsule, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Cone"))
			{
				CreateDefaultMesh("Cone", DefaultMesh::StaticMeshType::Cone, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Cylinder"))
			{
				CreateDefaultMesh("Cylinder", DefaultMesh::StaticMeshType::Cylinder, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Plane"))
			{
				CreateDefaultMesh("Plane", DefaultMesh::StaticMeshType::Plane, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Torus"))
			{
				CreateDefaultMesh("Torus", DefaultMesh::StaticMeshType::Torus, m_ContextScene, editorCamera);
			}

			Gui::EndMenu();
		}
		UI::Draw::Underline();
		Gui::Spacing();

		Gui::Text((const char*)VX_ICON_SPINNER);
		Gui::SameLine();
		if (Gui::BeginMenu("Create 2D"))
		{
			if (Gui::MenuItem("Quad"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Quad"));
				Entity selected = SelectionManager::GetSelectedEntity();
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
				selected.GetTransform().Translation.z = 0.0f;
				selected.AddComponent<SpriteRendererComponent>();
				selected.AddComponent<RigidBody2DComponent>();
				selected.AddComponent<BoxCollider2DComponent>();
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Circle"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Circle"));
				Entity selected = SelectionManager::GetSelectedEntity();
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
				selected.GetTransform().Translation.z = 0.0f;
				selected.AddComponent<CircleRendererComponent>();
				selected.AddComponent<RigidBody2DComponent>();
				selected.AddComponent<CircleCollider2DComponent>();
			}

			Gui::EndMenu();
		}
		UI::Draw::Underline();
		Gui::Spacing();

		Gui::Text((const char*)VX_ICON_VIDEO_CAMERA);
		Gui::SameLine();
		if (Gui::BeginMenu("Camera"))
		{
			if (Gui::MenuItem("Perspective"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Camera"));
				Entity selected = SelectionManager::GetSelectedEntity();
				auto& cameraComponent = selected.AddComponent<CameraComponent>();
				cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Orthographic"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Camera"));
				Entity selected = SelectionManager::GetSelectedEntity();
				auto& cameraComponent = selected.AddComponent<CameraComponent>();
				cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}
		UI::Draw::Underline();
		Gui::Spacing();

		Gui::Text((const char*)VX_ICON_LIGHTBULB_O);
		Gui::SameLine();
		if (Gui::BeginMenu("Light"))
		{
			if (Gui::MenuItem("Directional"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Directional Light"));
				Entity selected = SelectionManager::GetSelectedEntity();
				LightSourceComponent& lightSourceComponent = selected.AddComponent<LightSourceComponent>();
				lightSourceComponent.Type = LightType::Directional;
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Point"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Point Light"));
				Entity selected = SelectionManager::GetSelectedEntity();
				LightSourceComponent& lightSourceComponent = selected.AddComponent<LightSourceComponent>();
				lightSourceComponent.Type = LightType::Point;
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Spot"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Spot Light"));
				Entity selected = SelectionManager::GetSelectedEntity();
				LightSourceComponent& lightSourceComponent = selected.AddComponent<LightSourceComponent>();
				lightSourceComponent.Type = LightType::Spot;
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}
		UI::Draw::Underline();
		Gui::Spacing();

		Gui::Text((const char*)VX_ICON_CALCULATOR);
		Gui::SameLine();
		if (Gui::BeginMenu("Physics"))
		{
			if (Gui::MenuItem("Box Collider"))
			{
				CreateDefaultMesh("Box Collider", DefaultMesh::StaticMeshType::Cube, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Sphere Collider"))
			{
				CreateDefaultMesh("Sphere Collider", DefaultMesh::StaticMeshType::Sphere, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Capsule Collider"))
			{
				CreateDefaultMesh("Capsule Collider", DefaultMesh::StaticMeshType::Capsule, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Mesh Collider"))
			{
				CreateDefaultMesh("Mesh Collider", DefaultMesh::StaticMeshType::Cube, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Fixed Joint"))
			{
				CreateDefaultMesh("Fixed Joint", DefaultMesh::StaticMeshType::Cube, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Box Collider 2D"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Box Collider2D"));
				Entity selected = SelectionManager::GetSelectedEntity();
				selected.AddComponent<SpriteRendererComponent>();
				selected.AddComponent<RigidBody2DComponent>();
				selected.AddComponent<BoxCollider2DComponent>();
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Circle Collider 2D"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Circle Collider2D"));
				Entity selected = SelectionManager::GetSelectedEntity();
				selected.AddComponent<CircleRendererComponent>();
				selected.AddComponent<RigidBody2DComponent>();
				selected.AddComponent<CircleCollider2DComponent>();
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}
		UI::Draw::Underline();
		Gui::Spacing();

		Gui::Text((const char*)VX_ICON_VOLUME_UP);
		Gui::SameLine();
		if (Gui::BeginMenu("Audio"))
		{
			if (Gui::MenuItem("Source Entity"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Audio Source"));
				Entity selected = SelectionManager::GetSelectedEntity();
				selected.AddComponent<AudioSourceComponent>();
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Listener Entity"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Audio Listener"));
				Entity selected = SelectionManager::GetSelectedEntity();
				selected.AddComponent<AudioListenerComponent>();
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}
		UI::Draw::Underline();
		Gui::Spacing();

		Gui::Text((const char*)VX_ICON_FONT);
		Gui::SameLine();
		if (Gui::BeginMenu("UI"))
		{
			if (Gui::MenuItem("Text"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Text"));
				Entity selected = SelectionManager::GetSelectedEntity();
				selected.AddComponent<TextMeshComponent>();
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}
		UI::Draw::Underline();
		Gui::Spacing();

		Gui::Text((const char*)VX_ICON_BOMB);
		Gui::SameLine();
		if (Gui::BeginMenu("Effects"))
		{
			if (Gui::MenuItem("Particles"))
			{
				SelectionManager::SetSelectedEntity(m_ContextScene->CreateEntity("Particle Emitter"));
				Entity selected = SelectionManager::GetSelectedEntity();
				selected.AddComponent<ParticleEmitterComponent>();
				selected.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}
	}

	void SceneHierarchyPanel::DisplayInsectorPanel(Entity hoveredEntity)
	{
		Gui::Begin("Inspector", &s_ShowInspectorPanel);

		if (Entity selected = SelectionManager::GetSelectedEntity())
		{
			DrawComponents(selected);
		}
		else
		{
			const char* name = "(null)";

			if (m_ContextScene && hoveredEntity && hoveredEntity.HasComponent<TagComponent>())
			{
				const auto& tag = hoveredEntity.GetComponent<TagComponent>().Tag;

				if (!tag.empty())
					name = tag.c_str();
			}

			Gui::SetCursorPosX(10.0f);
			Gui::Text("Hovered Entity: %s", name);
		}

		Gui::End();
	}

	void SceneHierarchyPanel::DisplayAddComponentPopup()
	{
		if (Gui::BeginPopup("AddComponent", ImGuiWindowFlags_NoMove))
		{
			// Search Bar + Filtering
			const bool isSearching = Gui::InputTextWithHint("##ComponentSearch", "Search", m_ComponentSearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
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
				if (const char* componentName = "Text Mesh"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<TextMeshComponent>(componentName, (const char*)VX_ICON_TEXT_HEIGHT);
				if (const char* componentName = "Animator"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<AnimatorComponent>(componentName, (const char*)VX_ICON_CLOCK_O);
				if (const char* componentName = "Animation"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayAddComponentMenuItem<AnimationComponent>(componentName, (const char*)VX_ICON_ADJUST);
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
					DisplayAddComponentMenuItem<ScriptComponent>(componentName, (const char*)VX_ICON_FILE_CODE_O);	
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
					DisplayAddComponentMenuItem<TextMeshComponent>("Text Mesh", (const char*)VX_ICON_TEXT_HEIGHT);
					DisplayAddComponentMenuItem<AnimatorComponent>("Animator", (const char*)VX_ICON_CLOCK_O);
					DisplayAddComponentMenuItem<AnimationComponent>("Animation", (const char*)VX_ICON_ADJUST, false);

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
		std::string& entityMarker = tagComponent.Marker;
		size_t markerSize = entityMarker.size();

		if (entityMarker.empty())
		{
			tempBuffer.reserve(MAX_MARKER_SIZE);
		}
		else
		{
			tempBuffer.resize(markerSize);
			memcpy(tempBuffer.data(), entityMarker.data(), markerSize);
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

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, const EditorCamera* editorCamera)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		const auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((SelectionManager::GetSelectedEntity() == entity) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (entity.Children().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;

		const bool isPrefab = entity.HasComponent<PrefabComponent>();
		const bool entityActive = entity.IsActive();

		if (isPrefab)
			Gui::PushStyleColor(ImGuiCol_Text, ImVec4(0.32f, 0.7f, 0.87f, 1.0f));
		if (!entityActive)
			Gui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

		const bool opened = Gui::TreeNodeEx((void*)(uint32_t)entity, flags, tag.c_str());
		
		if (isPrefab)
			Gui::PopStyleColor();
		if (!entityActive)
			Gui::PopStyleColor();

		// Allow dragging entities
		if (Gui::IsItemHovered() && Gui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			SelectionManager::SetSelectedEntity(entity);
			m_EntityShouldBeRenamed = false;
		}

		m_EntityShouldBeDestroyed = false;
		
		// Right-click on entity for utilities popup
		if (Gui::BeginPopupContextItem())
		{
			if (Gui::MenuItem("Rename", "F2"))
			{
				SelectionManager::SetSelectedEntity(entity);
				m_EntityShouldBeRenamed = true;
				Gui::CloseCurrentPopup();
			}
			UI::Draw::Underline();

			if (Gui::MenuItem("Add Empty Child"))
			{
				Entity childEntity = m_ContextScene->CreateEntity("Empty Entity");
				m_ContextScene->ParentEntity(childEntity, entity);
				// Set child to origin of parent
				childEntity.GetTransform().Translation = Math::vec3(0.0f);
				SelectionManager::SetSelectedEntity(childEntity);
				Gui::CloseCurrentPopup();
			}
			UI::Draw::Underline();

			if (Gui::MenuItem("Unparent Entity"))
			{
				m_ContextScene->UnparentEntity(entity);
				Gui::CloseCurrentPopup();
			}
			UI::Draw::Underline();

			if (Gui::MenuItem("Duplicate Entity", "Ctrl+D"))
			{
				m_ContextScene->DuplicateEntity(entity);
				Gui::CloseCurrentPopup();
			}
			UI::Draw::Underline();

			if (Gui::MenuItem("Delete Entity", "Del") && SelectionManager::GetSelectedEntity())
				m_EntityShouldBeDestroyed = true;

			Gui::EndPopup();
		}

		if (Gui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Gui::Text(entity.GetName().c_str());
			Gui::SetDragDropPayload("SCENE_HIERARCHY_ITEM", &entity, sizeof(Entity));
			Gui::EndDragDropSource();
		}

		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM", ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
			{
				Entity& droppedEntity = *((Entity*)payload->Data);
				m_ContextScene->ParentEntity(droppedEntity, entity);
			}

			Gui::EndDragDropTarget();
		}

		if (opened)
		{
			const auto& children = entity.Children();
			for (const auto& child : children)
			{
				Entity childEntity = m_ContextScene->TryGetEntityWithUUID(child);
				if (childEntity && std::find(children.begin(), children.end(), child) != children.end());
				{
					DrawEntityNode(childEntity, editorCamera);
				}
			}

			Gui::TreePop();
		}

		// Destroy the entity if requested
		if (m_EntityShouldBeDestroyed && SelectionManager::GetSelectedEntity() == entity)
		{
			SelectionManager::DeselectEntity();
			m_ContextScene->SubmitToDestroyEntity(entity);
		}
	}

	template <typename TComponent>
	struct ComponentUICallbacks
	{
		using ValueType = TComponent&;
		using ReferenceType = std::function<void(ValueType, Entity)>;
		using ConstType = std::function<void(const ValueType, Entity)>;

		ReferenceType OnGuiRenderFn = nullptr;
		ReferenceType OnComponentResetFn = nullptr;
		ConstType OnComponentCopiedFn = nullptr;
		ReferenceType OnComponentPastedFn = nullptr;
		ReferenceType OnComponentRemovedFn = nullptr;

		bool IsRemoveable = true;
	};

	template <typename TComponent>
	static void DrawComponent(const std::string& name, Entity entity, const ComponentUICallbacks<TComponent>& callbacks)
	{
		if (entity.HasComponent<TComponent>())
		{
			auto& component = entity.GetComponent<TComponent>();
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
						std::invoke(callbacks.OnComponentCopiedFn, component, entity);
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
						std::invoke(callbacks.OnComponentPastedFn, component, entity);
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
						std::invoke(callbacks.OnComponentResetFn, component, entity);
					}
					Gui::CloseCurrentPopup();
				}
				Gui::EndDisabled();

				if (callbacks.IsRemoveable)
					UI::Draw::Underline();

				if (callbacks.IsRemoveable)
				{
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
				std::invoke(callbacks.OnGuiRenderFn, component, entity);
				UI::EndTreeNode();
			}

			if (removeComponent)
			{
				if (callbacks.OnComponentRemovedFn != nullptr)
				{
					std::invoke(callbacks.OnComponentRemovedFn, component, entity);
				}

				entity.RemoveComponent<TComponent>();
			}
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

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		// Tag Component
		{
			auto& tagComponent = entity.GetComponent<TagComponent>();
			auto& tag = tagComponent.Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());

			const bool shouldRename = m_EntityShouldBeRenamed && SelectionManager::GetSelectedEntity() == entity;
			ImGuiInputTextFlags flags = shouldRename ? ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue : 0;

			if (shouldRename)
			{
				Gui::SetKeyboardFocusHere();
				m_IsEditingEntityName = true;
			}
			if (Gui::InputTextWithHint("##Tag", "Entity Name", buffer, sizeof(buffer), flags))
			{
				tag = std::string(buffer);

				// Set the focus to the scene panel otherwise the keyboard focus will still be on the input text box
				if (m_EntityShouldBeRenamed)
					Gui::SetWindowFocus("Scene");

				m_EntityShouldBeRenamed = false;
				m_IsEditingEntityName = false;
			}

			UI::DrawItemActivityOutline();

			Gui::SameLine();
			Gui::PushItemWidth(-1);

			const bool controlPressed = Input::IsKeyDown(KeyCode::LeftControl) || Input::IsKeyDown(KeyCode::RightControl);
			const bool shiftPressed = Input::IsKeyDown(KeyCode::LeftShift) || Input::IsKeyDown(KeyCode::RightShift);

			if (Gui::Button("Add Component") || (Input::IsKeyDown(KeyCode::A) && controlPressed && shiftPressed && Gui::IsWindowHovered()))
			{
				Gui::OpenPopup("AddComponent");

				// We should reset the search bar here
				memset(m_ComponentSearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
				m_ComponentSearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed
			}

			UI::DrawItemActivityOutline();

			UI::BeginPropertyGrid(100.0f);

			bool active = tagComponent.IsActive;
			if (UI::Property("Active", active))
			{
				entity.SetActive(active);
			}

			UI::EndPropertyGrid();

			Gui::SameLine();

			auto& markers = tagComponent.Markers;
			auto& current = tagComponent.Marker;

			if (auto it = std::find(markers.begin(), markers.end(), current); it != markers.end())
			{
				const char* currentIt = (*it).c_str();

				if (Gui::BeginCombo("##Marker", currentIt, ImGuiComboFlags_HeightLarge))
				{
					uint32_t arraySize = markers.size();

					for (uint32_t i = 0; i < arraySize; i++)
					{
						const bool isSelected = strcmp(currentIt, markers[i].c_str()) == 0;

						if (Gui::Selectable(markers[i].c_str(), isSelected))
						{
							currentIt = markers[i].c_str();
							current = markers[i];
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
								DisplayAddMarkerPopup(tagComponent);

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
				tagComponent.AddMarker(tagComponent.Marker);
			}
		}

		Gui::PopItemWidth();

		DisplayAddComponentPopup();

		ComponentUICallbacks<TransformComponent> transformComponentCallbacks;
		transformComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::TransformComponentOnGuiRender);
		transformComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = TransformComponent(); };
		// TODO!
		transformComponentCallbacks.OnComponentCopiedFn = [&](auto& component, auto entity) {
			if (!m_CopyEntity.HasComponent<TransformComponent>()) {
				m_CopyEntity.AddComponent<TransformComponent>(component);
			}
		};
		// DITTO!
		transformComponentCallbacks.OnComponentPastedFn = [&](auto& component, auto entity) {
			component = m_CopyEntity.GetComponent<TransformComponent>();
		};
		transformComponentCallbacks.IsRemoveable = false;
		DrawComponent<TransformComponent>("Transform", entity, transformComponentCallbacks);

		ComponentUICallbacks<CameraComponent> cameraComponentCallbacks;
		cameraComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CameraComponentOnGuiRender);
		cameraComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) {
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
		DrawComponent<CameraComponent>("Camera", entity, cameraComponentCallbacks);

		ComponentUICallbacks<SkyboxComponent> skyboxComponentCallbacks;
		skyboxComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::SkyboxComponentOnGuiRender);
		skyboxComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = SkyboxComponent(); };
		DrawComponent<SkyboxComponent>("Skybox", entity, skyboxComponentCallbacks);

		ComponentUICallbacks<LightSourceComponent> lightSourceComponentCallbacks;
		lightSourceComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::LightSourceComponentOnGuiRender);
		lightSourceComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) {
			switch (component.Type)
			{
				case LightType::Directional: component = LightSourceComponent(LightType::Directional); break;
				case LightType::Point:       component = LightSourceComponent(LightType::Point);       break;
				case LightType::Spot:        component = LightSourceComponent(LightType::Spot);        break;
			}
		};
		DrawComponent<LightSourceComponent>("Light Source", entity, lightSourceComponentCallbacks);

		ComponentUICallbacks<LightSource2DComponent> lightSource2DComponentCallbacks;
		DrawComponent<LightSource2DComponent>("Light Source 2D", entity, lightSource2DComponentCallbacks);

		ComponentUICallbacks<MeshRendererComponent> meshRendererComponentCallbacks;
		meshRendererComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::MeshRendererComponentOnGuiRender);
		meshRendererComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = MeshRendererComponent(); };
		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, meshRendererComponentCallbacks);

		ComponentUICallbacks<StaticMeshRendererComponent> staticMeshRendererComponentCallbacks;
		staticMeshRendererComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::StaticMeshRendererComponentOnGuiRender);
		staticMeshRendererComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) {
			component = StaticMeshRendererComponent();
			component.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(DefaultMesh::StaticMeshType::Cube);
			SharedReference<MaterialTable> materialTable = component.Materials;
			materialTable->SetMaterial(0, Material::GetDefaultMaterialHandle());
		};
		DrawComponent<StaticMeshRendererComponent>("Static Mesh Renderer", entity, staticMeshRendererComponentCallbacks);

		ComponentUICallbacks<SpriteRendererComponent> spriteRendererComponentCallbacks;
		spriteRendererComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::SpriteRendererComponentOnGuiRender);
		spriteRendererComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = SpriteRendererComponent(); };
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, spriteRendererComponentCallbacks);

		ComponentUICallbacks<CircleRendererComponent> circleRendererComponentCallbacks;
		circleRendererComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CircleRendererComponentOnGuiRender);
		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, circleRendererComponentCallbacks);

		ComponentUICallbacks<ParticleEmitterComponent> particleEmitterComponentCallbacks;
		particleEmitterComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::ParticleEmitterComponentOnGuiRender);
		particleEmitterComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = ParticleEmitterComponent(); };
		DrawComponent<ParticleEmitterComponent>("Particle Emitter", entity, particleEmitterComponentCallbacks);

		ComponentUICallbacks<TextMeshComponent> textMeshComponentCallbacks;
		textMeshComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::TextMeshComponentOnGuiRender);
		textMeshComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = TextMeshComponent(); };
		DrawComponent<TextMeshComponent>("Text Mesh", entity, textMeshComponentCallbacks);

		ComponentUICallbacks<AnimatorComponent> animatorComponentCallbacks;
		animatorComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::AnimatorComponentOnGuiRender);
		animatorComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = AnimatorComponent(); };
		DrawComponent<AnimatorComponent>("Animator", entity, animatorComponentCallbacks);

		ComponentUICallbacks<AnimationComponent> animationComponentCallbacks;
		animationComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::AnimationComponentOnGuiRender);
		animationComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = AnimationComponent(); };
		DrawComponent<AnimationComponent>("Animation", entity, animationComponentCallbacks);

		ComponentUICallbacks<AudioSourceComponent> audioSourceComponentCallbacks;
		audioSourceComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::AudioSourceComponentOnGuiRender);
		audioSourceComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = AudioSourceComponent(); };
		DrawComponent<AudioSourceComponent>("Audio Source", entity, audioSourceComponentCallbacks);

		ComponentUICallbacks<AudioListenerComponent> audioListenerComponentCallbacks;
		audioListenerComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::AudioListenerComponentOnGuiRender);
		audioListenerComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = AudioListenerComponent(); };
		DrawComponent<AudioListenerComponent>("Audio Listener", entity, audioListenerComponentCallbacks);

		ComponentUICallbacks<RigidBodyComponent> rigidBodyComponentCallbacks;
		rigidBodyComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::RigidBodyComponentOnGuiRender);
		rigidBodyComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = RigidBodyComponent(); };
		DrawComponent<RigidBodyComponent>("RigidBody", entity, rigidBodyComponentCallbacks);

		ComponentUICallbacks<CharacterControllerComponent> characterControllerComponentCallbacks;
		characterControllerComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CharacterControllerComponentOnGuiRender);
		characterControllerComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = CharacterControllerComponent(); };
		DrawComponent<CharacterControllerComponent>("Character Controller", entity, characterControllerComponentCallbacks);

		ComponentUICallbacks<FixedJointComponent> fixedJointComponentCallbacks;
		fixedJointComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::FixedJointComponentOnGuiRender);
		fixedJointComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = FixedJointComponent(); };
		DrawComponent<FixedJointComponent>("Fixed Joint", entity, fixedJointComponentCallbacks);

		ComponentUICallbacks<BoxColliderComponent> boxColliderComponentCallbacks;
		boxColliderComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::BoxColliderComponentOnGuiRender);
		boxColliderComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = BoxColliderComponent(); };
		DrawComponent<BoxColliderComponent>("Box Collider", entity, boxColliderComponentCallbacks);

		ComponentUICallbacks<SphereColliderComponent> sphereColliderComponentCallbacks;
		sphereColliderComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::SphereColliderComponentOnGuiRender);
		sphereColliderComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = SphereColliderComponent(); };
		DrawComponent<SphereColliderComponent>("Sphere Collider", entity, sphereColliderComponentCallbacks);

		ComponentUICallbacks<CapsuleColliderComponent> capsuleColliderComponentCallbacks;
		capsuleColliderComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CapsuleColliderComponentOnGuiRender);
		capsuleColliderComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = CapsuleColliderComponent(); };
		DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, capsuleColliderComponentCallbacks);

		ComponentUICallbacks<MeshColliderComponent> meshColliderComponentCallbacks;
		meshColliderComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::MeshColliderComponentOnGuiRender);
		meshColliderComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = MeshColliderComponent(); };
		DrawComponent<MeshColliderComponent>("Mesh Collider", entity, meshColliderComponentCallbacks);

		ComponentUICallbacks<RigidBody2DComponent> rigidBody2DComponentCallbacks;
		rigidBody2DComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::RigidBody2DComponentOnGuiRender);
		rigidBody2DComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = RigidBody2DComponent(); };
		DrawComponent<RigidBody2DComponent>("RigidBody 2D", entity, rigidBody2DComponentCallbacks);

		ComponentUICallbacks<BoxCollider2DComponent> boxCollider2DComponentCallbacks;
		boxCollider2DComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::BoxCollider2DComponentOnGuiRender);
		boxCollider2DComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = BoxCollider2DComponent(); };
		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, boxCollider2DComponentCallbacks);

		ComponentUICallbacks<CircleCollider2DComponent> circleCollider2DComponentCallbacks;
		circleCollider2DComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::CircleCollider2DComponentOnGuiRender);
		circleCollider2DComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = CircleCollider2DComponent(); };
		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, circleCollider2DComponentCallbacks);

		ComponentUICallbacks<NavMeshAgentComponent> navMeshAgentComponentCallbacks;
		navMeshAgentComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::NavMeshAgentComponentOnGuiRender);
		navMeshAgentComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = NavMeshAgentComponent(); };
		DrawComponent<NavMeshAgentComponent>("Nav Mesh Agent", entity, navMeshAgentComponentCallbacks);

		ComponentUICallbacks<ScriptComponent> scriptComponentCallbacks;
		scriptComponentCallbacks.OnGuiRenderFn = VX_BIND_CALLBACK(SceneHierarchyPanel::ScriptComponentOnGuiRender);
		scriptComponentCallbacks.OnComponentResetFn = [](auto& component, auto entity) { component = ScriptComponent(); };
		DrawComponent<ScriptComponent>("Script", entity, scriptComponentCallbacks);

		ComponentUICallbacks<NativeScriptComponent> nativeScriptComponentCallbacks;
		DrawComponent<NativeScriptComponent>("Native Script", entity, nativeScriptComponentCallbacks);
	}

	void SceneHierarchyPanel::TransformComponentOnGuiRender(TransformComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();
		UI::DrawVec3Controls("Translation", component.Translation);
		Math::vec3 rotation = Math::Rad2Deg(component.GetRotationEuler());
		UI::DrawVec3Controls("Rotation", rotation, 0.0f, 100.0f, [&]()
		{
			component.SetRotationEuler(Math::Deg2Rad(rotation));
		});
		UI::DrawVec3Controls("Scale", component.Scale, 1.0f);
		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::CameraComponentOnGuiRender(CameraComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		SceneCamera& camera = component.Camera;

		UI::Property("Primary", component.Primary);

		const char* projectionTypes[] = { "Perspective", "Othrographic" };
		int32_t currentProjectionType = (int32_t)camera.GetProjectionType();
		if (UI::PropertyDropdown("Projection", projectionTypes, VX_ARRAYCOUNT(projectionTypes), currentProjectionType))
			camera.SetProjectionType((SceneCamera::ProjectionType)currentProjectionType);

		bool modified = false;

		if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
		{
			float perspectiveVerticalFOV = Math::Rad2Deg(camera.GetPerspectiveVerticalFOVRad());
			if (UI::Property("Field of View", perspectiveVerticalFOV, 1.0f, 1.0f))
			{
				camera.SetPerspectiveVerticalFOVRad(Math::Deg2Rad(perspectiveVerticalFOV));
				modified = true;
			}

			float nearClip = camera.GetPerspectiveNearClip();
			if (UI::Property("Near", nearClip, 1.0f, 1.0f))
			{
				camera.SetPerspectiveNearClip(nearClip);
				modified = true;
			}

			float farClip = camera.GetPerspectiveFarClip();
			if (UI::Property("Far", farClip, 1.0f, 1.0f))
			{
				camera.SetPerspectiveFarClip(farClip);
				modified = true;
			}
		}
		else
		{
			float orthoSize = camera.GetOrthographicSize();
			if (UI::Property("Size", orthoSize, 1.0f, 1.0f))
			{
				camera.SetOrthographicSize(orthoSize);
				modified = true;
			}

			float nearClip = camera.GetOrthographicNearClip();
			if (UI::Property("Near", nearClip, 1.0f, 1.0f))
			{
				camera.SetOrthographicNearClip(nearClip);
				modified = true;
			}

			float farClip = camera.GetOrthographicFarClip();
			if (UI::Property("Far", farClip, 1.0f, 1.0f))
			{
				camera.SetOrthographicFarClip(farClip);
				modified = true;
			}

			UI::Property("Fixed Aspect Ratio", component.FixedAspectRatio);
		}

		UI::Property("Clear Color", &component.ClearColor);

		UI::EndPropertyGrid();

		if (modified)
		{
			Math::uvec2 viewportSize = m_ContextScene->GetViewportSize();
			camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}
	}

	void SceneHierarchyPanel::SkyboxComponentOnGuiRender(SkyboxComponent& component, Entity entity)
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
				VX_CONSOLE_LOG_WARN("Could not load skybox, not a '.hdr' - {}", filepath.filename().string());
			}
		};

		UI::BeginPropertyGrid();
		UI::PropertyAssetReference<Skybox>("Environment Map", relativePath, component.Skybox, OnSkyboxDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry());
		UI::EndPropertyGrid();

		if (skybox && skybox->IsLoaded())
		{
			UI::BeginPropertyGrid();

			UI::Property("Rotation", component.Rotation);
			if (Gui::IsItemFocused())
			{
				// Nasty hack to reload skybox
				if (Input::IsKeyPressed(KeyCode::Enter))
				{
					skybox->SetShouldReload(true);
				}
			}

			UI::Property("Intensity", component.Intensity, 0.05f, 0.05f);

			UI::EndPropertyGrid();
		}
	}

	void SceneHierarchyPanel::LightSourceComponentOnGuiRender(LightSourceComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		static const char* lightTypes[] = { "Directional", "Point", "Spot" };
		int32_t currentLightType = (int32_t)component.Type;
		if (UI::PropertyDropdown("Light Type", lightTypes, VX_ARRAYCOUNT(lightTypes), currentLightType))
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
				UI::Property("CutOff", component.Cutoff, 0.5f, 0.5f, component.OuterCutoff);
				UI::Property("Outer CutOff", component.OuterCutoff, 0.5f, component.Cutoff, 100.0f);

				break;
			}
		}

		UI::Property("Radiance", &component.Radiance);
		UI::Property("Intensity", component.Intensity, 0.05f, 0.05f);

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
				UI::Property("Shadow Bias", component.ShadowBias, 1.0f, 0.0f, 1000.0f);
			}

			UI::EndPropertyGrid();
			UI::EndTreeNode();
		}
	}

	void SceneHierarchyPanel::MeshRendererComponentOnGuiRender(MeshRendererComponent& component, Entity entity)
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

						if (entity.HasComponent<AnimatorComponent>() && entity.HasComponent<AnimationComponent>() && AssetManager::GetAsset<Mesh>(component.Mesh)->HasAnimations())
						{
							AnimatorComponent& animatorComponent = entity.GetComponent<AnimatorComponent>();
							AnimationComponent& animationComponent = entity.GetComponent<AnimationComponent>();

							animationComponent.Animation = Animation::Create(meshFilepath.string(), component.Mesh);
							animatorComponent.Animator = Animator::Create(animationComponent.Animation);
						}
					}
				}
				else
				{
					VX_CONSOLE_LOG_WARN("Could not load model file - {}", meshFilepath.filename().string());
				}
			}

			Gui::EndDragDropTarget();
		}

		// TODO materials ///////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	void SceneHierarchyPanel::StaticMeshRendererComponentOnGuiRender(StaticMeshRendererComponent& component, Entity entity)
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
				VX_CONSOLE_LOG_WARN("Could not load model file - {}", filepath.filename().string());
			}
		};

		UI::PropertyAssetReference<StaticMesh>("Mesh Source", relativePath, component.StaticMesh, OnStaticMeshDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry());

		if (AssetManager::IsHandleValid(component.StaticMesh))
		{
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
						VX_CONSOLE_LOG_WARN("Could not load material {}", filepath.filename().string());
					}
				}
				else
				{
					VX_CONSOLE_LOG_WARN("Could not load material", filepath.filename().string());
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

						materialTable->SetMaterial(0, Renderer::GetWhiteMaterial()->Handle);
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
		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::SpriteRendererComponentOnGuiRender(SpriteRendererComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		ImVec4 tintColor = { component.SpriteColor.r, component.SpriteColor.g, component.SpriteColor.b, component.SpriteColor.a };

		SharedReference<Texture2D> icon = EditorResources::CheckerboardIcon;

		if (AssetManager::IsHandleValid(component.Texture))
		{
			icon = AssetManager::GetAsset<Texture2D>(component.Texture);
		}

		if (UI::ImageButton("Texture", icon, { 64, 64 }, { 0, 0, 0, 0 }, tintColor))
		{
			component.Texture = 0;
		}
		else if (Gui::IsItemHovered())
		{
			Gui::BeginTooltip();
			Gui::Text(Fs::Path(icon->GetPath()).stem().string().c_str());
			Gui::EndTooltip();
		}

		UI::EndPropertyGrid();

		// Accept a Texture from the content browser
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				Fs::Path texturePath = Fs::Path(path);

				// Make sure we are recieving an actual texture otherwise we will have trouble opening it
				if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(texturePath); type == AssetType::TextureAsset)
				{
					AssetHandle textureHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(texturePath);
					if (AssetManager::IsHandleValid(textureHandle))
					{
						component.Texture = textureHandle;
					}
					else
					{
						VX_CONSOLE_LOG_WARN("Could not load texture {}", texturePath.filename().string());
					}
				}
				else
				{
					VX_CONSOLE_LOG_WARN("Could not load texture", texturePath.filename().string());
				}
			}

			Gui::EndDragDropTarget();
		}

		UI::BeginPropertyGrid();

		UI::Property("Color", &component.SpriteColor);
		UI::Property("UV", component.TextureUV, 0.05f);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::CircleRendererComponentOnGuiRender(CircleRendererComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		UI::Property("Color", &component.Color);
		UI::Property("Thickness", component.Thickness, 0.025f, 0.0f, 1.0f);
		UI::Property("Fade", component.Fade, 0.00025f, 0.0f, 1.0f);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::ParticleEmitterComponentOnGuiRender(ParticleEmitterComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		SharedReference<ParticleEmitter> particleEmitter = nullptr;

		std::string emitterName = "Default";

		if (AssetManager::IsHandleValid(component.EmitterHandle))
		{
			particleEmitter = AssetManager::GetAsset<ParticleEmitter>(component.EmitterHandle);
			emitterName = particleEmitter->GetName();
		}

		UI::Property("Name", emitterName);

		if (particleEmitter)
		{
			ParticleEmitterProperties& emitterProperties = particleEmitter->GetProperties();

			if (Gui::Button("Start"))
				particleEmitter->Start();
			Gui::SameLine();

			if (Gui::Button("Stop"))
				particleEmitter->Stop();

			UI::Property("Velocity", emitterProperties.Velocity, 0.25f, 0.1f);
			UI::Property("Velocity Variation", emitterProperties.VelocityVariation, 0.25f, 0.1f);
			UI::Property("Offset", emitterProperties.Offset, 0.25f);
			UI::Property("Size Start", emitterProperties.SizeBegin, 0.25f, 0.1f);
			UI::Property("Size End", emitterProperties.SizeEnd, 0.25f, 0.1f);
			UI::Property("Size Variation", emitterProperties.SizeVariation, 0.25f, 0.1f);

			UI::Property("Generate Random Colors", emitterProperties.GenerateRandomColors);

			if (!emitterProperties.GenerateRandomColors)
			{
				UI::Property("Color Start", &emitterProperties.ColorBegin);
				UI::Property("Color End", &emitterProperties.ColorEnd);
			}

			UI::Property("Rotation", emitterProperties.Rotation, 0.1f, 0.0f);
			UI::Property("Lifetime", emitterProperties.LifeTime, 0.25f, 0.1f);
		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::TextMeshComponentOnGuiRender(TextMeshComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Visible", component.Visible);

		std::string relativePath = "Default Font";

		if (AssetManager::IsHandleValid(component.FontAsset))
		{
			const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.FontAsset);
			relativePath = metadata.Filepath.stem().string();
		}

		UI::Property("Font Source", relativePath, true);

		UI::EndPropertyGrid();

		// Accept a Font from the content browser
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				Fs::Path fontPath = Fs::Path(path);

				// Make sure we are recieving an actual font otherwise we will have trouble opening it
				if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(fontPath); type == AssetType::FontAsset)
				{
					AssetHandle fontAssetHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(fontPath);
					if (AssetManager::IsHandleValid(fontAssetHandle))
					{
						component.FontAsset = fontAssetHandle;
					}
					else
					{
						VX_CONSOLE_LOG_WARN("Could not load font {}", fontPath.filename().string());
					}
				}
				else
				{
					VX_CONSOLE_LOG_WARN("Could not load font, not a '.tff' - {}", fontPath.filename().string());
				}
			}

			Gui::EndDragDropTarget();
		}

		UI::BeginPropertyGrid();

		std::string textString = component.TextString;
		if (UI::MultilineTextBox("Text", textString))
		{
			component.TextString = textString;
			component.TextHash = std::hash<std::string>()(component.TextString);
		}

		UI::Property("Color", &component.Color);
		UI::Property("Background Color", &component.BgColor);
		UI::Property("Line Spacing", component.LineSpacing, 1.0f);
		UI::Property("Kerning", component.Kerning, 1.0f);
		UI::Property("Max Width", component.MaxWidth, 1.0f);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::AnimatorComponentOnGuiRender(AnimatorComponent& component, Entity entity)
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

	void SceneHierarchyPanel::AnimationComponentOnGuiRender(AnimationComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		SharedRef<Animation> animation = component.Animation;

		if (animation)
		{

		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::AudioSourceComponentOnGuiRender(AudioSourceComponent& component, Entity entity)
	{
		SharedReference<AudioSource> audioSource = nullptr;
		if (AssetManager::IsHandleValid(component.AudioHandle))
			audioSource = AssetManager::GetAsset<AudioSource>(component.AudioHandle);

		if (audioSource)
		{
			PlaybackDevice device = audioSource->GetPlaybackDevice();
			if (device.GetEngine().GetID() != Wave::ID::Invalid && (device.GetSound().IsPlaying() || device.GetSound().IsPaused()))
			{
				Gui::BeginDisabled(!device.GetSound().IsPlaying());
				const float fraction = device.GetSound().GetCursorInSeconds() / device.GetSound().GetLengthInSeconds();
				Gui::ProgressBar(fraction);
				Gui::EndDisabled();
			}

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
						device.Stop();

					if (FileSystem::GetFileExtension(filepath) != ".vsound")
					{
						std::string filename = FileSystem::RemoveFileExtension(filepath);
						filename += ".vsound";
						auto asset = Project::GetEditorAssetManager()->CreateNewAsset<AudioSource>("Audio", filename, filepath);

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
					VX_CONSOLE_LOG_WARN("Could not load audio file, not a '.wav' or '.mp3' - {}", filepath.filename().string());
				}
			};

			UI::BeginPropertyGrid();
			if (UI::PropertyAssetReference<AudioSource>("Source", relativePath, component.AudioHandle, OnAudioSourceDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry()))
			{
				const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.AudioHandle);
				if (component.AudioHandle != 0 && !metadata.Filepath.string().ends_with(".vsound"))
				{
					// TODO
					// we need to check if theres another vsound before creating one, we don't need to keep creating them if one alreay exists
					std::string filename = FileSystem::RemoveFileExtension(metadata.Filepath);
					filename += ".vsound";
					audioSource = Project::GetEditorAssetManager()->CreateNewAsset<AudioSource>("Audio", filename, metadata.Filepath);

					component.AudioHandle = audioSource->Handle;
				}
			}
			UI::EndPropertyGrid();

			Gui::BeginDisabled(audioSource == nullptr);

			if (audioSource)
			{
				UI::BeginPropertyGrid();

				float pitch = device.GetSound().GetPitch();
				if (UI::Property("Pitch", pitch, 0.01f, 0.2f, 2.0f))
					device.GetSound().SetPitch(pitch);

				float volume = device.GetSound().GetVolume();
				if (UI::Property("Volume", volume, 0.1f))
					device.GetSound().SetVolume(volume);

				//if (UI::Property("Play On Start", props.PlayOnStart))
					//audioSource->SetPlayOnStart(props.PlayOnStart);

				//if (UI::Property("Play One Shot", props.PlayOneShot))
					//audioSource->SetPlayOneShot(props.PlayOneShot);

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
					UI::DrawVec3Controls("Position", position, 0.0f, 100.0f, [&]()
					{
						device.GetSound().SetPosition(Utils::ToWaveVector(position));
					});

					Math::vec3 direction = Utils::FromWaveVector(device.GetSound().GetDirection());
					UI::DrawVec3Controls("Direction", direction, 0.0f, 100.0f, [&]()
					{
						device.GetSound().SetDirection(Utils::ToWaveVector(direction));
					});

					Math::vec3 velocity = Utils::FromWaveVector(device.GetSound().GetVelocity());
					UI::DrawVec3Controls("Veloctiy", velocity, 0.0f, 100.0f, [&]()
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
					if (UI::Property("Min Gain", minGain))
						device.GetSound().SetMinGain(minGain);

					float maxGain = device.GetSound().GetMaxGain();
					if (UI::Property("Max Gain", maxGain))
						device.GetSound().SetMaxGain(maxGain);

					static const char* attenuationModels[] = { "None", "Inverse", "Linear", "Exponential" };

					AttenuationModel currentAttenuationModel = Utils::FromWaveAttenuationModel(device.GetSound().GetAttenuationModel());
					if (UI::PropertyDropdown("Attenuation Model", attenuationModels, VX_ARRAYCOUNT(attenuationModels), currentAttenuationModel))
						device.GetSound().SetAttenuationModel(Utils::ToWaveAttenuationModel(currentAttenuationModel));

					float pan = device.GetSound().GetPan();
					if (UI::Property("Pan", pan))
						device.GetSound().SetPan(pan);

					static const char* panModes[] = { "Balance", "Pan" };

					PanMode currentPanMode = Utils::FromWavePanMode(device.GetSound().GetPanMode());
					if (UI::PropertyDropdown("Pan Mode", panModes, VX_ARRAYCOUNT(panModes), currentPanMode))
						device.GetSound().SetPanMode(Utils::ToWavePanMode(currentPanMode));

					static const char* positioningModes[] = { "Absolute", "Relative" };

					PositioningMode currentPositioningMode = Utils::FromWavePositioningMode(device.GetSound().GetPositioning());
					if (UI::PropertyDropdown("Positioning Mode", positioningModes, VX_ARRAYCOUNT(positioningModes), currentPositioningMode))
						device.GetSound().SetPositioning(Utils::ToWavePositioningMode(currentPositioningMode));

					float falloff = device.GetSound().GetFalloff();
					if (UI::Property("Falloff", falloff))
						device.GetSound().SetFalloff(falloff);

					float minDistance = device.GetSound().GetMinDistance();
					if (UI::Property("Min Distance", minDistance, 0.1f))
						device.GetSound().SetMinDistance(minDistance);

					float maxDistance = device.GetSound().GetMaxDistance();
					if (UI::Property("Max Distance", maxDistance, 0.1f))
						device.GetSound().SetMaxDistance(maxDistance);

					float dopplerFactor = device.GetSound().GetDopplerFactor();
					if (UI::Property("Doppler Factor", dopplerFactor, 0.1f))
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
					if (UI::Property("Inner Angle", innerAngle, 0.5f))
					{
						cone.InnerAngle = innerAngle;
						modified = true;
					}

					float outerAngle = Math::Rad2Deg(cone.OuterAngle);
					if (UI::Property("Outer Angle", outerAngle, 0.5f))
					{
						cone.OuterAngle = outerAngle;
						modified = true;
					}

					float outerGain = cone.OuterGain;
					if (UI::Property("Outer Gain", outerGain, 0.5f))
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

					auto one = device.GetSound().GetTimeInMilliseconds();
					UI::Property("Time in ms", one);
					auto two = device.GetSound().GetTimeInPCMFrames();
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
	}

	void SceneHierarchyPanel::AudioListenerComponentOnGuiRender(AudioListenerComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::RigidBodyComponentOnGuiRender(RigidBodyComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		const char* bodyTypes[] = { "Static", "Dynamic" };
		int32_t currentBodyType = (int32_t)component.Type;
		if (UI::PropertyDropdown("Body Type", bodyTypes, VX_ARRAYCOUNT(bodyTypes), currentBodyType))
		{
			const bool recreateActor = component.Type != (RigidBodyType)currentBodyType;
			component.Type = (RigidBodyType)currentBodyType;

			if (m_ContextScene->IsRunning() && recreateActor)
			{
				Physics::ReCreateActor(entity);
			}
		}

		bool modified = false;

		if (component.Type == RigidBodyType::Static)
		{
			UI::EndPropertyGrid();
		}
		else
		{
			if (UI::Property("Mass", component.Mass, 0.01f, 0.01f, 1.0f))
				modified = true;
			UI::Property("Linear Velocity", component.LinearVelocity);
			UI::Property("Max Linear Velocity", component.MaxLinearVelocity);
			UI::Property("Linear Drag", component.LinearDrag, 0.01f, 0.01f, 1.0f);
			UI::Property("Angular Velocity", component.AngularVelocity);
			UI::Property("Max Angular Velocity", component.MaxAngularVelocity);
			UI::Property("Angular Drag", component.AngularDrag, 0.01f, 0.01f, 1.0f, "%.2f");

			if (UI::Property("Disable Gravity", component.DisableGravity))
				modified = true;

			UI::Property("IsKinematic", component.IsKinematic);

			const char* collisionDetectionTypes[] = { "Discrete", "Continuous", "Continuous Speclative" };
			int32_t currentCollisionDetectionType = (uint32_t)component.CollisionDetection;
			if (UI::PropertyDropdown("Collision Detection", collisionDetectionTypes, VX_ARRAYCOUNT(collisionDetectionTypes), currentCollisionDetectionType))
				component.CollisionDetection = (CollisionDetectionType)currentCollisionDetectionType;

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

				Gui::SameLine();

				Gui::Text("Y");
				Gui::SameLine();

				if (Gui::Checkbox("##TranslationY", &translationY))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::TranslationY;
					modified = true;
				}

				Gui::SameLine();

				Gui::Text("Z");
				Gui::SameLine();

				if (Gui::Checkbox("##TranslationZ", &translationZ))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::TranslationZ;
					modified = true;
				}

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

				Gui::SameLine();

				Gui::Text("Y");
				Gui::SameLine();

				if (Gui::Checkbox("##RotationY", &rotationY))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::RotationY;
					modified = true;
				}

				Gui::SameLine();

				Gui::Text("Z");
				Gui::SameLine();

				if (Gui::Checkbox("##RotationZ", &rotationZ))
				{
					component.LockFlags ^= (uint8_t)ActorLockFlag::RotationZ;
					modified = true;
				}

				Gui::PopItemWidth();
				Gui::NextColumn();
				UI::Draw::Underline();

				UI::EndPropertyGrid();
				UI::EndTreeNode();

			}

			if (modified && m_ContextScene->IsRunning())
			{
				Physics::WakeUpActor(entity);
			}
		}
	}

	void SceneHierarchyPanel::CharacterControllerComponentOnGuiRender(CharacterControllerComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Slope Limit", component.SlopeLimitDegrees);
		UI::Property("Step Offset", component.StepOffset);
		UI::Property("Contact Offset", component.ContactOffset);
		UI::Property("Disable Gravity", component.DisableGravity);

		Gui::BeginDisabled();
		UI::Property("Speed Down", component.SpeedDown);
		Gui::EndDisabled();

		const char* nonWalkableModes[] = { "Prevent Climbing", "Prevent Climbing and Force Sliding" };
		int32_t currentNonWalkableMode = (uint32_t)component.NonWalkMode;
		if (UI::PropertyDropdown("Non Walkable Mode", nonWalkableModes, VX_ARRAYCOUNT(nonWalkableModes), currentNonWalkableMode))
			component.NonWalkMode = (NonWalkableMode)currentNonWalkableMode;

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			const char* climbModes[] = { "Easy", "Constrained" };
			int32_t currentClimbMode = (uint32_t)component.ClimbMode;
			if (UI::PropertyDropdown("Capsule Climb Mode", climbModes, VX_ARRAYCOUNT(climbModes), currentClimbMode))
				component.ClimbMode = (CapsuleClimbMode)currentClimbMode;
		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::FixedJointComponentOnGuiRender(FixedJointComponent& component, Entity entity)
	{
		std::string connectedEntityName = "Null";
		if (Entity connectedEntity = m_ContextScene->TryGetEntityWithUUID(component.ConnectedEntity))
		{
			connectedEntityName = connectedEntity.GetName();
		}

		UI::BeginPropertyGrid();
		UI::Property("Connected Entity", connectedEntityName, true);
		UI::EndPropertyGrid();

		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
			{
				Entity& droppedEntity = *((Entity*)payload->Data);
				component.ConnectedEntity = droppedEntity.GetUUID();
			}

			Gui::EndDragDropTarget();
		}

		UI::BeginPropertyGrid();

		UI::Property("Is Breakable", component.IsBreakable);

		if (component.IsBreakable)
		{
			UI::Property("Break Force", component.BreakForce);
			UI::Property("Break Torque", component.BreakTorque);
		}

		UI::Property("Enable Collision", component.EnableCollision);
		UI::Property("Enable PreProcessing", component.EnablePreProcessing);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::BoxColliderComponentOnGuiRender(BoxColliderComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Half Size", component.HalfSize, 0.01f);
		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Visible", component.Visible);
		UI::Property("Is Trigger", component.IsTrigger);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::SphereColliderComponentOnGuiRender(SphereColliderComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Radius", component.Radius, 0.01f);
		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Visible", component.Visible);
		UI::Property("Is Trigger", component.IsTrigger);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::CapsuleColliderComponentOnGuiRender(CapsuleColliderComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Radius", component.Radius, 0.01f);
		UI::Property("Height", component.Height, 0.01f);
		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Visible", component.Visible);
		UI::Property("Is Trigger", component.IsTrigger);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::MeshColliderComponentOnGuiRender(MeshColliderComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		static const char* collisionComplexities[] = { "Default", "Use Complex as Simple", "Use Simple as Complex" };
		uint32_t currentCollisionComplexity = (uint32_t)component.CollisionComplexity;

		if (UI::PropertyDropdown("Collision Complexity", collisionComplexities, VX_ARRAYCOUNT(collisionComplexities), currentCollisionComplexity))
			component.CollisionComplexity = (ECollisionComplexity)currentCollisionComplexity;

		UI::Property("Visible", component.Visible);
		UI::Property("Is Trigger", component.IsTrigger);
		UI::Property("Use Shared Shape", component.UseSharedShape);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::RigidBody2DComponentOnGuiRender(RigidBody2DComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
		int32_t currentBodyType = (uint32_t)component.Type;
		if (UI::PropertyDropdown("Body Type", bodyTypes, VX_ARRAYCOUNT(bodyTypes), currentBodyType))
			component.Type = (RigidBody2DType)currentBodyType;

		if (component.Type == RigidBody2DType::Dynamic)
		{
			UI::Property("Velocity", component.Velocity, 0.01f);
			UI::Property("Drag", component.Drag, 0.01f, 0.01f, 1.0f);
			UI::Property("Angular Velocity", component.AngularVelocity);
			UI::Property("Angular Drag", component.AngularDrag, 0.01f, 0.01f, 1.0f);
			UI::Property("Gravity Scale", component.GravityScale, 0.01f, 0.01f, 1.0f);
			UI::Property("Freeze Rotation", component.FixedRotation);
		}

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::BoxCollider2DComponentOnGuiRender(BoxCollider2DComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Size", component.Size, 0.01f);
		UI::Property("Density", component.Density, 0.01f, 0.0f, 1.0f);
		UI::Property("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
		UI::Property("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
		UI::Property("Threshold", component.RestitutionThreshold, 0.1f, 0.0f);
		UI::Property("Visible", component.Visible);
		UI::Property("Is Tigger", component.IsTrigger);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::CircleCollider2DComponentOnGuiRender(CircleCollider2DComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::Property("Offset", component.Offset, 0.01f);
		UI::Property("Radius", component.Radius, 0.01, 0.01f);
		UI::Property("Density", component.Density, 0.01f, 0.0f, 1.0f);
		UI::Property("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
		UI::Property("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
		UI::Property("Threshold", component.RestitutionThreshold, 0.1f, 0.0f);
		UI::Property("Visible", component.Visible);

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::NavMeshAgentComponentOnGuiRender(NavMeshAgentComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		UI::EndPropertyGrid();
	}

	void SceneHierarchyPanel::ScriptComponentOnGuiRender(ScriptComponent& component, Entity entity)
	{
		UI::BeginPropertyGrid();

		std::vector<const char*> entityClassNameStrings;
		const bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

		std::unordered_map<std::string, SharedReference<ScriptClass>> entityClasses = ScriptEngine::GetClasses();

		for (auto& [className, entityScriptClass] : entityClasses)
			entityClassNameStrings.push_back(className.c_str());

		std::string currentClassName = "(null)";
		if (!component.ClassName.empty())
		{
			currentClassName = component.ClassName;
		}

		// Display available entity classes to choose from
		if (UI::PropertyDropdownSearch("Class", entityClassNameStrings.data(), entityClassNameStrings.size(), currentClassName, m_EntityClassNameInputTextFilter))
			component.ClassName = currentClassName;

		const bool sceneRunning = m_ContextScene->IsRunning();

		// Fields
		if (sceneRunning)
		{
			SharedReference<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());

			if (scriptInstance)
			{
				const auto& fields = scriptInstance->GetScriptClass()->GetFields();

				for (const auto& [name, field] : fields)
				{
					if (field.Type == ScriptFieldType::Float)
					{
						float data = scriptInstance->GetFieldValue<float>(name);
						if (UI::Property(name.c_str(), data, 0.01f))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Double)
					{
						double data = scriptInstance->GetFieldValue<double>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Vector2)
					{
						Math::vec2 data = scriptInstance->GetFieldValue<Math::vec2>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Vector3)
					{
						Math::vec3 data = scriptInstance->GetFieldValue<Math::vec3>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Vector4)
					{
						Math::vec4 data = scriptInstance->GetFieldValue<Math::vec4>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Color3)
					{
						Math::vec3 data = scriptInstance->GetFieldValue<Math::vec3>(name);
						if (UI::Property(name.c_str(), &data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Color4)
					{
						Math::vec4 data = scriptInstance->GetFieldValue<Math::vec4>(name);
						if (UI::Property(name.c_str(), &data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Bool)
					{
						bool data = scriptInstance->GetFieldValue<bool>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Char)
					{
						char data = scriptInstance->GetFieldValue<char>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Short)
					{
						short data = scriptInstance->GetFieldValue<short>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Int)
					{
						int data = scriptInstance->GetFieldValue<int>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Long)
					{
						long long data = scriptInstance->GetFieldValue<long long>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Byte)
					{
						unsigned char data = scriptInstance->GetFieldValue<unsigned char>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::UShort)
					{
						unsigned short data = scriptInstance->GetFieldValue<unsigned short>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::UInt)
					{
						unsigned int data = scriptInstance->GetFieldValue<unsigned int>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::ULong)
					{
						unsigned long long data = scriptInstance->GetFieldValue<unsigned long long>(name);
						if (UI::Property(name.c_str(), data))
							scriptInstance->SetFieldValue(name, data);
					}
					if (field.Type == ScriptFieldType::Entity)
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
								Entity& droppedEntity = *((Entity*)payload->Data);
								scriptInstance->SetFieldValue(name, data);
							}

							Gui::EndDragDropTarget();
						}

						UI::BeginPropertyGrid();
					}
					if (field.Type == ScriptFieldType::AssetHandle)
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
								Entity& droppedEntity = *((Entity*)payload->Data);
								scriptInstance->SetFieldValue(name, data);
							}*/

							Gui::EndDragDropTarget();
						}

						UI::BeginPropertyGrid();
					}
				}
			}
		}
		else
		{
			if (scriptClassExists)
			{
				SharedReference<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
				const auto& fields = entityClass->GetFields();

				ScriptFieldMap& entityScriptFields = ScriptEngine::GetMutableScriptFieldMap(entity);

				for (const auto& [name, field] : fields)
				{
					auto it = entityScriptFields.find(name);

					// Field has been set in editor
					if (it != entityScriptFields.end())
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
						if (field.Type == ScriptFieldType::Entity)
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
									Entity& droppedEntity = *((Entity*)payload->Data);
									scriptField.SetValue(droppedEntity.GetUUID());
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
									Entity& droppedEntity = *((Entity*)payload->Data);
									scriptField.SetValue(droppedEntity.GetUUID());
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
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Double)
						{
							double data = 0.0f;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Vector2)
						{
							Math::vec2 data = Math::vec2(0.0f);
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Vector3)
						{
							Math::vec3 data = Math::vec3(0.0f);
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Vector4)
						{
							Math::vec4 data = Math::vec4(0.0f);
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Color3)
						{
							Math::vec3 data = Math::vec3(0.0f);
							if (UI::Property(name.c_str(), &data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Color4)
						{
							Math::vec4 data = Math::vec4(0.0f, 0.0f, 0.0f, 1.0f);
							if (UI::Property(name.c_str(), &data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Bool)
						{
							bool data = false;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Char)
						{
							char data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Short)
						{
							short data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Int)
						{
							int data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Long)
						{
							long long data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Byte)
						{
							unsigned char data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::UShort)
						{
							unsigned short data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::UInt)
						{
							unsigned int data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::ULong)
						{
							unsigned long long data = 0;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance& fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}
						}
						if (field.Type == ScriptFieldType::Entity)
						{
							uint64_t data;
							if (UI::Property(name.c_str(), data))
							{
								ScriptFieldInstance fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}

							UI::EndPropertyGrid();

							if (Gui::BeginDragDropTarget())
							{
								if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
								{
									Entity& droppedEntity = *((Entity*)payload->Data);
									ScriptFieldInstance fieldInstance = entityScriptFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(droppedEntity.GetUUID());
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
								ScriptFieldInstance fieldInstance = entityScriptFields[name];
								fieldInstance.Field = field;
								fieldInstance.SetValue(data);
							}

							UI::EndPropertyGrid();

							if (Gui::BeginDragDropTarget())
							{
								// TODO
								/*if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
								{
									Entity& droppedEntity = *((Entity*)payload->Data);
									ScriptFieldInstance fieldInstance = entityScriptFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(droppedEntity.GetUUID());
								}*/

								Gui::EndDragDropTarget();
							}

							UI::BeginPropertyGrid();
						}
					}
				}
			}
		}

		UI::EndPropertyGrid();
	}

}
