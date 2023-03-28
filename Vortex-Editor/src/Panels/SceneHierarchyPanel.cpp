#include "SceneHierarchyPanel.h"

#include <Vortex/Editor/EditorResources.h>
#include <Vortex/Scripting/ScriptEngine.h>
#include <Vortex/Core/Buffer.h>

#include <imgui_internal.h>

#include <codecvt>

namespace Vortex {

#define MAX_CHILD_ENTITY_SEARCH_DEPTH 10

	SceneHierarchyPanel::SceneHierarchyPanel(const SharedReference<Scene>& context)
	{
		SetSceneContext(context);
	}

	void SceneHierarchyPanel::OnGuiRender(Entity hoveredEntity, const EditorCamera* editorCamera)
	{
		if (s_ShowSceneHierarchyPanel)
		{
			Gui::Begin("Scene Hierarchy", &s_ShowSceneHierarchyPanel);

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

	void SceneHierarchyPanel::SetSceneContext(const SharedReference<Scene>& scene)
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

	inline static Entity CreateDefaultMesh(const std::string& entityName, DefaultMeshes::StaticMeshes defaultMesh, SharedReference<Scene>& contextScene, const EditorCamera* editorCamera)
	{
		Entity entity = contextScene->CreateEntity(entityName);
		StaticMeshRendererComponent& staticMeshRendererComponent = entity.AddComponent<StaticMeshRendererComponent>();
		staticMeshRendererComponent.Type = static_cast<MeshType>(defaultMesh);
		staticMeshRendererComponent.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(defaultMesh);
		entity.GetTransform().Translation = editorCamera->GetFocalPoint();

		entity.AddComponent<RigidBodyComponent>();

		switch (defaultMesh)
		{
			case DefaultMeshes::StaticMeshes::Cube:     entity.AddComponent<BoxColliderComponent>();     break;
			case DefaultMeshes::StaticMeshes::Sphere:   entity.AddComponent<SphereColliderComponent>();  break;
			case DefaultMeshes::StaticMeshes::Capsule:  entity.AddComponent<CapsuleColliderComponent>(); break;
			case DefaultMeshes::StaticMeshes::Cone:     entity.AddComponent<MeshColliderComponent>();    break;
			case DefaultMeshes::StaticMeshes::Cylinder: entity.AddComponent<MeshColliderComponent>();    break;
			case DefaultMeshes::StaticMeshes::Plane:    entity.AddComponent<MeshColliderComponent>();    break;
			case DefaultMeshes::StaticMeshes::Torus:    entity.AddComponent<MeshColliderComponent>();    break;
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
				CreateDefaultMesh("Cube", DefaultMeshes::StaticMeshes::Cube, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Sphere"))
			{
				CreateDefaultMesh("Sphere", DefaultMeshes::StaticMeshes::Sphere, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Capsule"))
			{
				CreateDefaultMesh("Capsule", DefaultMeshes::StaticMeshes::Capsule, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Cone"))
			{
				CreateDefaultMesh("Cone", DefaultMeshes::StaticMeshes::Cone, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Cylinder"))
			{
				CreateDefaultMesh("Cylinder", DefaultMeshes::StaticMeshes::Cylinder, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Plane"))
			{
				CreateDefaultMesh("Plane", DefaultMeshes::StaticMeshes::Plane, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Torus"))
			{
				CreateDefaultMesh("Torus", DefaultMeshes::StaticMeshes::Torus, m_ContextScene, editorCamera);
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
				CreateDefaultMesh("Box Collider", DefaultMeshes::StaticMeshes::Cube, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Sphere Collider"))
			{
				CreateDefaultMesh("Sphere Collider", DefaultMeshes::StaticMeshes::Sphere, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Capsule Collider"))
			{
				CreateDefaultMesh("Capsule Collider", DefaultMeshes::StaticMeshes::Capsule, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Mesh Collider"))
			{
				CreateDefaultMesh("Mesh Collider", DefaultMeshes::StaticMeshes::Cube, m_ContextScene, editorCamera);
			}
			UI::Draw::Underline();
			Gui::Spacing();

			if (Gui::MenuItem("Fixed Joint"))
			{
				CreateDefaultMesh("Fixed Joint", DefaultMeshes::StaticMeshes::Cube, m_ContextScene, editorCamera);
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

		Entity selected = SelectionManager::GetSelectedEntity();

		if (selected)
		{
			DrawComponents(selected);
		}
		else
		{
			const char* name = "None";

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
		if (Gui::BeginPopup("AddComponent"))
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

			if (!searchBarInUse)
			{
				if (Gui::BeginMenu("Rendering"))
				{
					DisplayComponentMenuItem<CameraComponent>("Camera", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<SkyboxComponent>("Skybox", (const char*)VX_ICON_SKYATLAS);
					DisplayComponentMenuItem<LightSourceComponent>("Light Source", (const char*)VX_ICON_LIGHTBULB_O);
					DisplayComponentMenuItem<MeshRendererComponent>("Mesh Renderer", (const char*)VX_ICON_HOME);
					DisplayComponentMenuItem<StaticMeshRendererComponent>("Static Mesh Renderer", (const char*)VX_ICON_CUBE);
					DisplayComponentMenuItem<LightSource2DComponent>("Light Source 2D", (const char*)VX_ICON_LIGHTBULB_O);
					DisplayComponentMenuItem<SpriteRendererComponent>("Sprite Renderer", (const char*)VX_ICON_SPINNER);
					DisplayComponentMenuItem<CircleRendererComponent>("Circle Renderer", (const char*)VX_ICON_CIRCLE);
					DisplayComponentMenuItem<ParticleEmitterComponent>("Particle Emitter", (const char*)VX_ICON_BOMB);
					DisplayComponentMenuItem<TextMeshComponent>("Text Mesh", (const char*)VX_ICON_TEXT_HEIGHT);
					DisplayComponentMenuItem<AnimatorComponent>("Animator", (const char*)VX_ICON_CLOCK_O);
					DisplayComponentMenuItem<AnimationComponent>("Animation", (const char*)VX_ICON_ADJUST);

					Gui::EndMenu();
				}
				UI::Draw::Underline();
				Gui::Spacing();

				if (Gui::BeginMenu("Physics"))
				{
					DisplayComponentMenuItem<RigidBodyComponent>("RigidBody", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<CharacterControllerComponent>("Character Controller", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<FixedJointComponent>("Fixed Joint", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<BoxColliderComponent>("Box Collider", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<SphereColliderComponent>("Sphere Collider", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<CapsuleColliderComponent>("Capsule Collider", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<MeshColliderComponent>("Mesh Collider", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<RigidBody2DComponent>("RigidBody 2D", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<BoxCollider2DComponent>("Box Collider 2D", (const char*)VX_ICON_VIDEO_CAMERA);
					DisplayComponentMenuItem<CircleCollider2DComponent>("Circle Collider 2D", (const char*)VX_ICON_VIDEO_CAMERA);

					Gui::EndMenu();
				}
				UI::Draw::Underline();
				Gui::Spacing();

				if (Gui::BeginMenu("Audio"))
				{
					DisplayComponentMenuItem<AudioSourceComponent>("Audio Source", (const char*)VX_ICON_VOLUME_UP);
					DisplayComponentMenuItem<AudioListenerComponent>("Audio Listener", (const char*)VX_ICON_HEADPHONES);

					Gui::EndMenu();
				}
				UI::Draw::Underline();
				Gui::Spacing();

				if (Gui::BeginMenu("AI"))
				{
					DisplayComponentMenuItem<NavMeshAgentComponent>("Nav Mesh Agent", (const char*)VX_ICON_LAPTOP);

					Gui::EndMenu();
				}
				UI::Draw::Underline();
				Gui::Spacing();

				DisplayComponentMenuItem<ScriptComponent>("Script", (const char*)VX_ICON_FILE_CODE_O);
			}
			else
			{
				if (const char* name = "Camera"; m_ComponentSearchInputTextFilter.PassFilter(name))
					DisplayComponentMenuItem<CameraComponent>(name, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Skybox"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<SkyboxComponent>(componentName, (const char*)VX_ICON_SKYATLAS);
				if (const char* componentName = "Light Source"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<LightSourceComponent>(componentName, (const char*)VX_ICON_LIGHTBULB_O);
				if (const char* componentName = "Mesh Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<MeshRendererComponent>(componentName, (const char*)VX_ICON_HOME);
				if (const char* componentName = "Static Mesh Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<StaticMeshRendererComponent>(componentName, (const char*)VX_ICON_CUBE);
				if (const char* componentName = "Light Source 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<LightSource2DComponent>(componentName, (const char*)VX_ICON_LIGHTBULB_O);
				if (const char* componentName = "Sprite Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<SpriteRendererComponent>(componentName, (const char*)VX_ICON_SPINNER);
				if (const char* componentName = "Circle Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<CircleRendererComponent>(componentName, (const char*)VX_ICON_CIRCLE);
				if (const char* componentName = "Particle Emitter"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<ParticleEmitterComponent>(componentName, (const char*)VX_ICON_BOMB);
				if (const char* componentName = "Text Mesh"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<TextMeshComponent>(componentName, (const char*)VX_ICON_TEXT_HEIGHT);
				if (const char* componentName = "Animator"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<AnimatorComponent>(componentName, (const char*)VX_ICON_CLOCK_O);
				if (const char* componentName = "Animation"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<AnimationComponent>(componentName, (const char*)VX_ICON_ADJUST);
				if (const char* componentName = "RigidBody"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<RigidBodyComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Character Controller"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<CharacterControllerComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Fixed Joint"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<FixedJointComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Box Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<BoxColliderComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Sphere Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<SphereColliderComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Capsule Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<CapsuleColliderComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Mesh Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<MeshColliderComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Audio Source"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<AudioSourceComponent>(componentName, (const char*)VX_ICON_VOLUME_UP);
				if (const char* componentName = "Audio Listener"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<AudioListenerComponent>(componentName, (const char*)VX_ICON_HEADPHONES);
				if (const char* componentName = "RigidBody 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<RigidBody2DComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Box Collider 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<BoxCollider2DComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Circle Collider 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<CircleCollider2DComponent>(componentName, (const char*)VX_ICON_VIDEO_CAMERA);
				if (const char* componentName = "Nav Mesh Agent"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<NavMeshAgentComponent>(componentName, (const char*)VX_ICON_LAPTOP);
				if (const char* componentName = "Script"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<ScriptComponent>(componentName, (const char*)VX_ICON_FILE_CODE_O);
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

		std::string buffer;
		std::string& marker = tagComponent.Marker;
		size_t markerSize = marker.size();

		if (marker.empty())
		{
			buffer.reserve(25);
		}
		else
		{
			buffer.resize(markerSize * 2);
			memcpy(buffer.data(), marker.data(), markerSize * 2);
		}

		if (Gui::InputText("##Marker", buffer.data(), sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue) && !buffer.empty())
		{
			tagComponent.Marker = buffer;
			TagComponent::AddMarker(buffer);
			Gui::SetWindowFocus("Scene");
		}
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

	template <typename TComponent, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiCallback, std::function<void(const TComponent&)> copyCallback = nullptr, std::function<void(TComponent&)> pasteCallback = nullptr, bool removeable = true)
	{
		if (entity.HasComponent<TComponent>())
		{
			auto& component = entity.GetComponent<TComponent>();
			ImVec2 contentRegionAvailable = Gui::GetContentRegionAvail();

			Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			UI::Draw::Underline();
			bool open = UI::PropertyGridHeader(name.c_str());
			Gui::PopStyleVar();
			Gui::SameLine(contentRegionAvailable.x - lineHeight * 0.6f);
			UI::ShiftCursorY(2.0f);
			if (Gui::Button((const char*)VX_ICON_COG, { lineHeight, lineHeight }))
				Gui::OpenPopup("ComponentSettings");

			bool componentShouldBeRemoved = false;
			if (Gui::BeginPopup("ComponentSettings"))
			{
				Gui::BeginDisabled(copyCallback == nullptr);
				if (Gui::MenuItem("Copy Component"))
				{
					// TODO: Copy Component
					if (copyCallback)
						copyCallback(component);

					Gui::CloseCurrentPopup();
				}
				Gui::EndDisabled();

				UI::Draw::Underline();

				Gui::BeginDisabled(pasteCallback == nullptr);
				if (Gui::MenuItem("Paste Component"))
				{
					if (pasteCallback)
						pasteCallback(component);

					Gui::CloseCurrentPopup();
				}
				Gui::EndDisabled();

				UI::Draw::Underline();

				if (Gui::MenuItem("Reset Component"))
				{
					if constexpr (std::is_same<TComponent, StaticMeshRendererComponent>())
					{
						component = StaticMeshRendererComponent();
						component.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(DefaultMeshes::StaticMeshes::Cube);
					}
					else if constexpr (std::is_same<TComponent, AudioSourceComponent>())
					{
						SharedReference<AudioSource> audioSource = component.Source;
						component = AudioSourceComponent();
						component.Source = audioSource;
					}
					else if constexpr (std::is_same<TComponent, LightSourceComponent>())
					{
						auto ResetLightFunc = [&](auto type)
						{
							component = LightSourceComponent();
							component.Source = LightSource::Create(LightSourceProperties());
							component.Type = type;
						};

						switch (component.Type)
						{
							case LightType::Directional: ResetLightFunc(LightType::Directional); break;
							case LightType::Point:       ResetLightFunc(LightType::Point);       break;
							case LightType::Spot:        ResetLightFunc(LightType::Spot);        break;
						}
					}
					else if constexpr (std::is_same<TComponent, CameraComponent>())
					{
						auto ResetCameraFunc = [&](auto type)
						{
							component = CameraComponent();
							component.Camera.SetProjectionType(type);
						};

						switch (component.Camera.GetProjectionType())
						{
							case SceneCamera::ProjectionType::Perspective:  ResetCameraFunc(SceneCamera::ProjectionType::Perspective);  break;
							case SceneCamera::ProjectionType::Orthographic: ResetCameraFunc(SceneCamera::ProjectionType::Orthographic); break;
						}
					}
					else
					{
						component = TComponent();
					}

					Gui::CloseCurrentPopup();
				}

				if (removeable)
					UI::Draw::Underline();

				if (removeable && Gui::MenuItem("Remove Component"))
					componentShouldBeRemoved = true;

				Gui::EndPopup();
			}

			if (open)
			{
				uiCallback(component);
				UI::EndTreeNode();
			}

			if (componentShouldBeRemoved)
				entity.RemoveComponent<TComponent>();
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

			Gui::SameLine();
			Gui::PushItemWidth(-1);

			bool controlPressed = Input::IsKeyDown(KeyCode::LeftControl) || Input::IsKeyDown(KeyCode::RightControl);
			bool shiftPressed = Input::IsKeyDown(KeyCode::LeftShift) || Input::IsKeyDown(KeyCode::RightShift);

			if (Gui::Button("Add Component") || (Input::IsKeyDown(KeyCode::A) && controlPressed && shiftPressed && Gui::IsWindowHovered()))
			{
				Gui::OpenPopup("AddComponent");

				// We should reset the search bar here
				memset(m_ComponentSearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
				m_ComponentSearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed
			}

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

							if (Gui::BeginPopup("AddMarker"))
							{
								DisplayAddMarkerPopup(tagComponent);

								Gui::EndPopup();
							}
						}
					}

					Gui::EndCombo();
				}
			}
			else
			{
				// Add to markers vector because it's not a default marker
				tagComponent.AddMarker(tagComponent.Marker);
			}
		}

		Gui::PopItemWidth();
		DisplayAddComponentPopup();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
		{
			UI::DrawVec3Controls("Translation", component.Translation);
			Math::vec3 rotation = Math::Rad2Deg(component.GetRotationEuler());
			UI::DrawVec3Controls("Rotation", rotation, 0.0f, 100.0f, [&]()
			{
				component.SetRotationEuler(Math::Deg2Rad(rotation));
			});
			UI::DrawVec3Controls("Scale", component.Scale, 1.0f);
		},
		[=](const auto& component)
		{
			m_TransformToCopy = (TransformComponent)component;
		},
		[=](auto& component)
		{
			component = m_TransformToCopy;
		}, false);

		DrawComponent<CameraComponent>("Camera", entity, [&](auto& component)
		{
			SceneCamera& camera = component.Camera;

			UI::BeginPropertyGrid();

			UI::Property("Primary", component.Primary);

			const char* projectionTypes[] = { "Perspective", "Othrographic" };
			int32_t currentProjectionType = (int32_t)camera.GetProjectionType();
			if (UI::PropertyDropdown("Projection", projectionTypes, VX_ARRAYCOUNT(projectionTypes), currentProjectionType))
				camera.SetProjectionType((SceneCamera::ProjectionType)currentProjectionType);

			bool modified = false;

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float perspectiveVerticalFOV = Math::Rad2Deg(camera.GetPerspectiveVerticalFOVRad());
				if (UI::Property("Field of View", perspectiveVerticalFOV))
				{
					camera.SetPerspectiveVerticalFOVRad(Math::Deg2Rad(perspectiveVerticalFOV));
					modified = true;
				}
				
				float nearClip = camera.GetPerspectiveNearClip();
				if (UI::Property("Near", nearClip))
				{
					camera.SetPerspectiveNearClip(nearClip);
					modified = true;
				}

				float farClip = camera.GetPerspectiveFarClip();
				if (UI::Property("Far", farClip))
				{
					camera.SetPerspectiveFarClip(farClip);
					modified = true;
				}
			}
			else
			{
				float orthoSize = camera.GetOrthographicSize();
				if (UI::Property("Size", orthoSize))
				{
					camera.SetOrthographicSize(orthoSize);
					modified = true;
				}

				float nearClip = camera.GetOrthographicNearClip();
				if (UI::Property("Near", nearClip))
				{
					camera.SetOrthographicNearClip(nearClip);
					modified = true;
				}

				float farClip = camera.GetOrthographicFarClip();
				if (UI::Property("Far", farClip))
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
		});

		DrawComponent<SkyboxComponent>("Skybox", entity, [](auto& component)
		{
			AssetHandle environmentHandle = component.Skybox;
			SharedReference<Skybox> skybox = nullptr;

			if (AssetManager::IsHandleValid(environmentHandle))
			{
				skybox = AssetManager::GetAsset<Skybox>(environmentHandle);
			}

			UI::BeginPropertyGrid();

			std::string relativeSkyboxPath = "None";

			if (skybox && skybox->IsLoaded())
			{
				const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.Skybox);
				relativeSkyboxPath = metadata.Filepath.string();
			}

			UI::Property("Environment Map", relativeSkyboxPath, true);

			// Accept a Skybox Directory from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path skyboxPath = std::filesystem::path(path);

					// Make sure we are recieving an actual directory or hdr texture otherwise we will have trouble loading it
					if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(skyboxPath); type == AssetType::EnvironmentAsset)
					{
						AssetHandle environmentHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(skyboxPath);
						if (AssetManager::IsHandleValid(environmentHandle))
						{
							component.Skybox = environmentHandle;
						}
					}
					else
					{
						VX_CONSOLE_LOG_WARN("Could not load skybox, not a '.hdr' - {}", skyboxPath.filename().string());
					}
				}

				Gui::EndDragDropTarget();
			}

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
		});

		DrawComponent<LightSourceComponent>("Light Source", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Visible", component.Visible);

			static const char* lightTypes[] = { "Directional", "Point", "Spot" };
			int32_t currentLightType = (int32_t)component.Type;
			if (UI::PropertyDropdown("Light Type", lightTypes, VX_ARRAYCOUNT(lightTypes), currentLightType))
				component.Type = (LightType)currentLightType;

			SharedRef<LightSource> lightSource = component.Source;

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
					float cutoff = lightSource->GetCutOff();
					if (UI::Property("CutOff", cutoff))
						lightSource->SetCutOff(cutoff);

					float outerCutoff = lightSource->GetOuterCutOff();
					if (UI::Property("Outer CutOff", outerCutoff))
						lightSource->SetOuterCutOff(outerCutoff);

					break;
				}
			}

			Math::vec3 radiance = lightSource->GetRadiance();
			if (UI::Property("Radiance", &radiance))
				lightSource->SetRadiance(radiance);

			float intensity = lightSource->GetIntensity();
			if (UI::Property("Intensity", intensity, 0.05f, 0.05f))
				lightSource->SetIntensity(intensity);

			UI::EndPropertyGrid();

			if (UI::PropertyGridHeader("Shadows", false))
			{
				UI::BeginPropertyGrid();

				bool castShadows = lightSource->GetCastShadows();
				if (UI::Property("Cast Shadows", castShadows))
				{
					lightSource->SetCastShadows(castShadows);

					if (castShadows)
					{
						Renderer::CreateShadowMap(component.Type);
					}
				}

				if (castShadows)
				{
					bool softShadows = lightSource->GetSoftShadows();
					if (UI::Property("Soft Shadows", softShadows))
						lightSource->SetSoftShadows(softShadows);

					float shadowBias = lightSource->GetShadowBias();
					if (UI::Property("Shadow Bias", shadowBias, 1.0f, 0.0f, 1000.0f))
						lightSource->SetShadowBias(shadowBias);
				}

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}
		});

		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [&](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Visible", component.Visible);
			
			std::string relativeMeshPath = "";

			if (AssetManager::IsHandleValid(component.Mesh))
			{
				const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.Mesh);
				relativeMeshPath = metadata.Filepath.string();
			}

			UI::Property("Mesh Source", relativeMeshPath, true);

			// Accept a Model File from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path meshFilepath = std::filesystem::path(path);

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

			UI::EndPropertyGrid();

			// TODO materials ///////////////////////////////////////////////////////////////////////////////////////////////////////
		});

		DrawComponent<StaticMeshRendererComponent>("Static Mesh Renderer", entity, [&](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Visible", component.Visible);

			std::string relativePath = "";

			if (AssetManager::IsHandleValid(component.StaticMesh))
			{
				const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.StaticMesh);
				relativePath = metadata.Filepath.string();
			}

			UI::Property("Mesh Source", relativePath, true);

			// Accept a Model File from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path staticMeshFilepath = std::filesystem::path(path);

					// Make sure we are recieving an actual model file otherwise we will have trouble opening it
					if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(staticMeshFilepath); type == AssetType::StaticMeshAsset || type == AssetType::MeshAsset)
					{
						AssetHandle staticMeshHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(staticMeshFilepath);
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
						VX_CONSOLE_LOG_WARN("Could not load model file - {}", staticMeshFilepath.filename().string());
					}
				}

				Gui::EndDragDropTarget();
			}

			static const char* meshTypes[] = { "Cube", "Sphere", "Capsule", "Cone", "Cylinder", "Plane", "Torus", "Custom" };
			int32_t currentMeshType = (int32_t)component.Type;
			if (UI::PropertyDropdown("Mesh Type", meshTypes, VX_ARRAYCOUNT(meshTypes), currentMeshType))
			{
				component.Type = (MeshType)currentMeshType;

				if (component.Type == MeshType::Custom)
				{
					// Temporary
					component.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(DefaultMeshes::StaticMeshes::Cube);
					
					if (AssetManager::IsHandleValid(component.StaticMesh))
					{
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
					component.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh((DefaultMeshes::StaticMeshes)component.Type);
					
					VX_CORE_ASSERT(AssetManager::IsHandleValid(component.StaticMesh), "Invalid Default Mesh Handle!");

					SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(component.StaticMesh);
					if (staticMesh)
					{
						component.Materials->Clear();
						staticMesh->LoadMaterialTable(component.Materials);
					}
				}
			}

			if (AssetManager::IsHandleValid(component.StaticMesh))
			{
				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(component.StaticMesh);

				SharedReference<MaterialTable>& materialTable = component.Materials;

				uint32_t submeshIndex = 0;
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
					SharedReference<Material> material = nullptr;
					if (AssetManager::IsHandleValid(materialHandle))
					{
						material = AssetManager::GetAsset<Material>(materialHandle);
					}

					const auto& allMaterials = AssetManager::GetAllAssetsWithType<Material>();
					std::vector<const char*> options;
					std::vector<std::string> filepaths;

					for (const auto& materialHandle : allMaterials)
					{
						if (!AssetManager::IsHandleValid(materialHandle))
							continue;

						SharedReference<Material> mat = AssetManager::GetAsset<Material>(materialHandle);
						if (!mat)
							continue;

						const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(mat->Handle);
						if (!metadata.IsValid())
							continue;

						options.emplace_back(mat->GetName().c_str());
						filepaths.emplace_back(metadata.Filepath.string());
					}

					std::string currentMaterialName = material ? material->GetName() : "Default Material";
					if (UI::PropertyDropdownSearch("Material", options.data(), options.size(), currentMaterialName, m_MaterialSearchInputTextFilter))
					{
						uint32_t index = std::find(options.begin(), options.end(), currentMaterialName) - options.begin();
						material->Handle = *std::next(allMaterials.begin(), index);
						materialTable->SetMaterial(submeshIndex, material->Handle);
					}

					if (Gui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path materialPath = std::filesystem::path(path);

							// Make sure we are recieving an actual material otherwise we will have trouble opening it
							if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(materialPath); type == AssetType::MaterialAsset)
							{
								AssetHandle materialHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(materialPath);

								if (AssetManager::IsHandleValid(materialHandle))
								{
									SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
									if (material)
									{
										materialTable->SetMaterial(submeshIndex, material->Handle);
										material->SetName(FileSystem::RemoveFileExtension(materialPath));
									}
								}
								else
								{
									VX_CONSOLE_LOG_WARN("Could not load material {}", materialPath.filename().string());
								}
							}
							else
							{
								VX_CONSOLE_LOG_WARN("Could not load material", materialPath.filename().string());
							}
						}

						Gui::EndDragDropTarget();
					}

					submeshIndex++;
				}
			}

			UI::EndPropertyGrid();
		});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [&](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Visible", component.Visible);

			ImVec4 tintColor = { component.SpriteColor.r, component.SpriteColor.g, component.SpriteColor.b, component.SpriteColor.a };

			// Texture
			{
				SharedReference<Texture2D> icon = EditorResources::CheckerboardIcon;

				if (AssetManager::IsHandleValid(component.Texture))
					icon = AssetManager::GetAsset<Texture2D>(component.Texture);

				if (UI::ImageButton("Texture", icon, { 64, 64 }, { 0, 0, 0, 0 }, tintColor))
				{
					component.Texture = 0;
				}
				else if (Gui::IsItemHovered())
				{
					Gui::BeginTooltip();
					Gui::Text(icon->GetPath().c_str());
					Gui::EndTooltip();
				}

				// Accept a Texture from the content browser
				if (Gui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = std::filesystem::path(path);

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
			}

			UI::Property("Color", &component.SpriteColor);
			UI::Property("UV", component.TextureUV, 0.05f);

			UI::EndPropertyGrid();
		});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Visible", component.Visible);

			UI::Property("Color", &component.Color);
			UI::Property("Thickness", component.Thickness, 0.025f, 0.0f, 1.0f);
			UI::Property("Fade", component.Fade, 0.00025f, 0.0f, 1.0f);

			UI::EndPropertyGrid();
		});

		DrawComponent<ParticleEmitterComponent>("Particle Emitter", entity, [](auto& component)
		{
			SharedRef<ParticleEmitter> particleEmitter = component.Emitter;

			ParticleEmitterProperties& emitterProperties = particleEmitter->GetProperties();

			if (Gui::Button("Start"))
				particleEmitter->Start();
			Gui::SameLine();

			if (Gui::Button("Stop"))
				particleEmitter->Stop();

			UI::BeginPropertyGrid();

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

			UI::EndPropertyGrid();
		},
		[&](const auto& component)
		{
			m_ParticleEmitterToCopy = component.Emitter->GetProperties();
		},
		[&](const auto& component)
		{
			component.Emitter->SetProperties(m_ParticleEmitterToCopy);
		});

		DrawComponent<TextMeshComponent>("Text Mesh", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Visible", component.Visible);

			std::string relativeFontPath = "Default Font";

			if (AssetManager::IsHandleValid(component.FontAsset))
			{
				const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(component.FontAsset);
				relativeFontPath = metadata.Filepath.string();
			}

			UI::Property("Font Source", relativeFontPath, true);

			// Accept a Font from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path fontPath = std::filesystem::path(path);

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
		});

		DrawComponent<AnimatorComponent>("Animator", entity, [&](auto& component)
		{
			SharedRef<Animator> animator = component.Animator;

			if (animator)
			{
				if (Gui::Button("Play"))
					animator->PlayAnimation();

				Gui::SameLine();

				if (Gui::Button("Stop"))
					animator->Stop();
			}
		});

		DrawComponent<AnimationComponent>("Animation", entity, [](auto& component)
		{
			SharedRef<Animation> animation = component.Animation;
		});

		DrawComponent<AudioSourceComponent>("Audio Source", entity, [](auto& component)
		{
			if (component.Source)
			{
				Gui::BeginDisabled(!component.Source->IsPlaying());
				Gui::ProgressBar(component.Source->GetAmountComplete());
				Gui::EndDisabled();
			}

			Gui::BeginDisabled(component.Source == nullptr);

			if (Gui::Button("Play"))
			{
				if (component.Source->GetProperties().PlayOneShot)
				{
					component.Source->PlayOneShot();
				}
				else
				{
					component.Source->Play();
				}
			}

			Gui::SameLine();

			{
				const bool disabled = component.Source != nullptr && !component.Source->IsPlaying();
				Gui::BeginDisabled(disabled);

				if (Gui::Button("Pause"))
					component.Source->Pause();

				Gui::SameLine();

				if (Gui::Button("Restart"))
					component.Source->Restart();

				Gui::EndDisabled();
			}

			Gui::SameLine();

			if (Gui::Button("Stop"))
			{
				component.Source->Stop();
			}

			Gui::EndDisabled();

			UI::BeginPropertyGrid();

			std::string audioSourcePath = "";
			if (component.Source)
			{
				audioSourcePath = component.Source->GetPath();

				std::string relativeAudioSourcePath = "";
				if (component.Source && !audioSourcePath.empty())
					relativeAudioSourcePath = FileSystem::Relative(audioSourcePath, Project::GetAssetDirectory()).string();

				UI::Property("Source", relativeAudioSourcePath, true);
			}

			// Accept a Audio File from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path audioSourcePath = std::filesystem::path(path);

					// Make sure we are recieving an actual audio file otherwise we will have trouble opening it
					if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(audioSourcePath); type == AssetType::AudioAsset)
					{
						// If there is another file playing we need to stop it
						if (component.Source->IsPlaying())
							component.Source->Stop();

						SharedReference<AudioSource> audioSource = component.Source;
						audioSource->SetPath(audioSourcePath.string());
						audioSource->Reload();
					}
					else
					{
						VX_CONSOLE_LOG_WARN("Could not load audio file, not a '.wav' or '.mp3' - {}", audioSourcePath.filename().string());
					}
				}

				Gui::EndDragDropTarget();
			}

			Gui::BeginDisabled(component.Source == nullptr);

			{
				Gui::BeginDisabled(true);
				const AudioClip& audioClip = component.Source->GetAudioClip();
				float length = audioClip.Length;
				UI::Property("Length", length);
				Gui::EndDisabled();
			}

			if (component.Source != nullptr)
			{
				SoundProperties& props = component.Source->GetProperties();

				if (UI::Property("Pitch", props.Pitch, 0.01f, 0.2f, 2.0f))
					component.Source->SetPitch(props.Pitch);

				if (UI::Property("Volume", props.Volume, 0.1f))
					component.Source->SetVolume(props.Volume);

				if (UI::Property("Play On Start", props.PlayOnStart))
					component.Source->SetPlayOnStart(props.PlayOnStart);

				if (UI::Property("Play One Shot", props.PlayOneShot))
					component.Source->SetPlayOneShot(props.PlayOneShot);

				if (UI::Property("Loop", props.Loop))
					component.Source->SetLoop(props.Loop);

				if (UI::Property("Spacialized", props.Spacialized))
					component.Source->SetSpacialized(props.Spacialized);

				UI::EndPropertyGrid();

				if (props.Spacialized && UI::PropertyGridHeader("Spatialization", false))
				{
					UI::BeginPropertyGrid();

					UI::DrawVec3Controls("Position", props.Position, 0.0f, 100.0f, [&]()
					{
						component.Source->SetPosition(props.Position);
					});

					UI::DrawVec3Controls("Direction", props.Direction, 0.0f, 100.0f, [&]()
					{
						component.Source->SetDirection(props.Direction);
					});

					UI::DrawVec3Controls("Veloctiy", props.Velocity, 0.0f, 100.0f, [&]()
					{
						component.Source->SetVelocity(props.Velocity);
					});

					UI::EndPropertyGrid();

					if (UI::PropertyGridHeader("Cone", false))
					{
						UI::BeginPropertyGrid();

						float innerAngle = Math::Rad2Deg(props.Cone.InnerAngle);
						if (UI::Property("Inner Angle", innerAngle, 0.5f))
						{
							props.Cone.InnerAngle = Math::Deg2Rad(innerAngle);
							component.Source->SetCone(props.Cone);
						}
						float outerAngle = Math::Rad2Deg(props.Cone.OuterAngle);
						if (UI::Property("Outer Angle", outerAngle, 0.5f))
						{
							props.Cone.OuterAngle = Math::Deg2Rad(outerAngle);
							component.Source->SetCone(props.Cone);
						}
						float outerGain = Math::Rad2Deg(props.Cone.OuterGain);
						if (UI::Property("Outer Gain", outerGain, 0.5f))
						{
							props.Cone.OuterGain = Math::Deg2Rad(outerGain);
							component.Source->SetCone(props.Cone);
						}

						if (UI::Property("Min Distance", props.MinDistance, 0.1f))
							component.Source->SetMinDistance(props.MinDistance);

						if (UI::Property("Max Distance", props.MaxDistance, 0.1f))
							component.Source->SetMaxDistance(props.MaxDistance);

						if (UI::Property("Doppler Factor", props.DopplerFactor, 0.1f))
							component.Source->SetDopplerFactor(props.DopplerFactor);

						UI::EndPropertyGrid();
						UI::EndTreeNode();
					}

					UI::EndTreeNode();
				}
			}

			Gui::EndDisabled();
		});

		DrawComponent<AudioListenerComponent>("Audio Listener", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			AudioListener::ListenerProperties& props = component.Listener->GetProperties();

			UI::DrawVec3Controls("Position", props.Position, 0.0f, 100.0f, [&]()
			{
				component.Listener->SetPosition(props.Position);
			});

			UI::DrawVec3Controls("Direction", props.Direction, 0.0f, 100.0f, [&]()
			{
				component.Listener->SetDirection(props.Direction);
			});

			UI::DrawVec3Controls("Veloctiy", props.Veloctiy, 0.0f, 100.0f, [&]()
			{
				component.Listener->SetVelocity(props.Veloctiy);
			});

			UI::EndPropertyGrid();

			if (UI::PropertyGridHeader("Cone", false))
			{
				UI::BeginPropertyGrid();

				float innerAngle = Math::Rad2Deg(props.Cone.InnerAngle);
				if (UI::Property("Inner Angle", innerAngle, 0.5f))
				{
					props.Cone.InnerAngle = Math::Deg2Rad(innerAngle);
					component.Listener->SetCone(props.Cone);
				}
				float outerAngle = Math::Rad2Deg(props.Cone.OuterAngle);
				if (UI::Property("Outer Angle", outerAngle, 0.5f))
				{
					props.Cone.OuterAngle = Math::Deg2Rad(outerAngle);
					component.Listener->SetCone(props.Cone);
				}
				float outerGain = Math::Rad2Deg(props.Cone.OuterGain);
				if (UI::Property("Outer Gain", outerGain, 0.5f))
				{
					props.Cone.OuterGain = Math::Deg2Rad(outerGain);
					component.Listener->SetCone(props.Cone);
				}

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}
		});

		DrawComponent<RigidBodyComponent>("RigidBody", entity, [&, scene = m_ContextScene](auto& component)
		{
			UI::BeginPropertyGrid();

			const char* bodyTypes[] = { "Static", "Dynamic" };
			int32_t currentBodyType = (int32_t)component.Type;
			if (UI::PropertyDropdown("Body Type", bodyTypes, VX_ARRAYCOUNT(bodyTypes), currentBodyType))
			{
				const bool recreateActor = component.Type != (RigidBodyType)currentBodyType;
				component.Type = (RigidBodyType)currentBodyType;

				if (scene->IsRunning() && recreateActor)
				{
					Physics::ReCreateActor(entity);
				}
			}

			if (component.Type == RigidBodyType::Static)
			{
				UI::EndPropertyGrid();
			}
			else
			{
				UI::Property("Mass", component.Mass, 0.01f, 0.01f, 1.0f);
				UI::Property("Linear Velocity", component.LinearVelocity);
				UI::Property("Max Linear Velocity", component.MaxLinearVelocity);
				UI::Property("Linear Drag", component.LinearDrag, 0.01f, 0.01f, 1.0f);
				UI::Property("Angular Velocity", component.AngularVelocity);
				UI::Property("Max Angular Velocity", component.MaxAngularVelocity);
				UI::Property("Angular Drag", component.AngularDrag, 0.01f, 0.01f, 1.0f, "%.2f");

				UI::Property("Disable Gravity", component.DisableGravity);
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

					bool modified = false;

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

					if (modified && m_ContextScene->IsRunning())
					{
						Physics::WakeUpActor(entity);
					}
				}
			}
		});

		DrawComponent<CharacterControllerComponent>("Character Controller", entity, [entity](auto& component)
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
		});

		DrawComponent<FixedJointComponent>("Fixed Joint", entity, [&](auto& component)
		{
			UI::BeginPropertyGrid();

			std::string connectedEntityName = "Null";
			if (Entity connectedEntity = m_ContextScene->TryGetEntityWithUUID(component.ConnectedEntity))
			{
				connectedEntityName = connectedEntity.GetName();
			}

			UI::Property("Connected Entity", connectedEntityName, true);

			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
				{
					Entity& droppedEntity = *((Entity*)payload->Data);
					component.ConnectedEntity = droppedEntity.GetUUID();
				}

				Gui::EndDragDropTarget();
			}

			UI::Property("Is Breakable", component.IsBreakable);
			
			if (component.IsBreakable)
			{
				UI::Property("Break Force", component.BreakForce);
				UI::Property("Break Torque", component.BreakTorque);
			}

			UI::Property("Enable Collision", component.EnableCollision);
			UI::Property("Enable PreProcessing", component.EnablePreProcessing);

			UI::EndPropertyGrid();
		});

		DrawComponent<BoxColliderComponent>("Box Collider", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Half Size", component.HalfSize, 0.01f);
			UI::Property("Offset", component.Offset, 0.01f);
			UI::Property("Is Trigger", component.IsTrigger);

			UI::EndPropertyGrid();
		});

		DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Radius", component.Radius, 0.01f);
			UI::Property("Offset", component.Offset, 0.01f);
			UI::Property("Is Trigger", component.IsTrigger);

			UI::EndPropertyGrid();
		});

		DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Radius", component.Radius, 0.01f);
			UI::Property("Height", component.Height, 0.01f);
			UI::Property("Offset", component.Offset, 0.01f);
			UI::Property("Is Trigger", component.IsTrigger);

			UI::EndPropertyGrid();
		});

		DrawComponent<MeshColliderComponent>("Mesh Collider", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			static const char* collisionComplexities[] = { "Default", "Use Complex as Simple", "Use Simple as Complex" };
			uint32_t currentCollisionComplexity = (uint32_t)component.CollisionComplexity;

			if (UI::PropertyDropdown("Collision Complexity", collisionComplexities, VX_ARRAYCOUNT(collisionComplexities), currentCollisionComplexity))
				component.CollisionComplexity = (ECollisionComplexity)currentCollisionComplexity;

			UI::Property("Is Trigger", component.IsTrigger);
			UI::Property("Use Shared Shape", component.UseSharedShape);

			UI::EndPropertyGrid();
		});

		DrawComponent<RigidBody2DComponent>("RigidBody 2D", entity, [](auto& component)
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
		});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Offset", component.Offset, 0.01f);
			UI::Property("Size", component.Size, 0.01f);
			UI::Property("Density", component.Density, 0.01f, 0.0f, 1.0f);
			UI::Property("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
			UI::Property("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
			UI::Property("Threshold", component.RestitutionThreshold, 0.1f, 0.0f);
			UI::Property("Is Tigger", component.IsTrigger);

			UI::EndPropertyGrid();
		});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

			UI::Property("Offset", component.Offset, 0.01f);
			UI::Property("Radius", component.Radius, 0.01, 0.01f);
			UI::Property("Density", component.Density, 0.01f, 0.0f, 1.0f);
			UI::Property("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
			UI::Property("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
			UI::Property("Threshold", component.RestitutionThreshold, 0.1f, 0.0f);

			UI::EndPropertyGrid();
		});

		DrawComponent<NavMeshAgentComponent>("Nav Mesh Agent", entity, [](auto& component)
		{
			
		});

		DrawComponent<ScriptComponent>("Script", entity, [&](auto& component)
		{
			UI::BeginPropertyGrid();

			std::vector<const char*> entityClassNameStrings;
			bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

			auto entityClasses = ScriptEngine::GetClasses();

			for (auto& [className, entityScriptClass] : entityClasses)
				entityClassNameStrings.push_back(className.c_str());

			std::string currentClassName = component.ClassName;

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

							if (Gui::BeginDragDropTarget())
							{
								if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
								{
									Entity& droppedEntity = *((Entity*)payload->Data);
									scriptInstance->SetFieldValue(name, data);
								}

								Gui::EndDragDropTarget();
							}
						}
						if (field.Type == ScriptFieldType::AssetHandle)
						{
							uint64_t data = scriptInstance->GetFieldValue<uint64_t>(name);
							if (UI::Property(name.c_str(), data))
							{
								scriptInstance->SetFieldValue(name, data);
							}

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

								if (Gui::BeginDragDropTarget())
								{
									if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
									{
										Entity& droppedEntity = *((Entity*)payload->Data);
										scriptField.SetValue(droppedEntity.GetUUID());
									}

									Gui::EndDragDropTarget();
								}
							}
							if (field.Type == ScriptFieldType::AssetHandle)
							{
								uint64_t data = scriptField.GetValue<uint64_t>();
								if (UI::Property(name.c_str(), data))
								{
									scriptField.SetValue(data);
								}

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
							}
						}
					}
				}
			}

			UI::EndPropertyGrid();
		});

		DrawComponent<NativeScriptComponent>("Native Script", entity, [](auto& component) {});
	}

}
