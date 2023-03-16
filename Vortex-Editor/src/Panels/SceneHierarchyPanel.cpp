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
			Gui::SetNextItemWidth(Gui::GetContentRegionAvail().x - Gui::CalcTextSize(" + ").x * 2.0f - 2.0f);
			bool isSearching = Gui::InputTextWithHint("##EntitySearch", "Search", m_EntitySearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_EntitySearchInputTextFilter.InputBuf));
			if (isSearching)
				m_EntitySearchInputTextFilter.Build();

			Gui::SameLine();

			if (Gui::Button(" + "))
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
				if (m_EntityShouldBeDestroyed && m_SelectedEntity)
				{
					Entity entityToBeDestroyed = m_SelectedEntity;

					m_SelectedEntity = {};
					m_EntityShouldBeDestroyed = false;

					// If we are hovering on the entity we must reset it otherwise entt will complain
					if (hoveredEntity == entityToBeDestroyed)
						hoveredEntity = Entity{};

					m_ContextScene->DestroyEntity(entityToBeDestroyed);
				}

				uint32_t searchDepth = 0;
				const bool entitySearchBarInUse = strlen(m_EntitySearchInputTextFilter.InputBuf) != 0;
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

				if (entitySearchBarInUse)
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
					m_SelectedEntity = {};
					m_EntityShouldBeRenamed = false;
					m_EntityShouldBeDestroyed = false;
				}

				// Right-click on blank space in scene hierarchy panel
				if (Gui::BeginPopupContextWindow(0, 1, false))
				{
					DisplayCreateEntityMenu(editorCamera);

					Gui::EndPopup();
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
		m_SelectedEntity = {};
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

	inline static void CreateDefaultMesh(const std::string& name, StaticMesh::Default defaultMesh, Entity& entity, SharedReference<Scene>& contextScene, const EditorCamera* editorCamera)
	{
		entity = contextScene->CreateEntity(name);
		StaticMeshRendererComponent& staticMeshRendererComponent = entity.AddComponent<StaticMeshRendererComponent>();
		staticMeshRendererComponent.Type = static_cast<MeshType>(defaultMesh);

		MeshImportOptions importOptions = MeshImportOptions();

		switch (defaultMesh)
		{
			case StaticMesh::Default::Cube:
				entity.AddComponent<RigidBodyComponent>();
				entity.AddComponent<BoxColliderComponent>();
				break;
			case StaticMesh::Default::Sphere:
				entity.AddComponent<RigidBodyComponent>();
				entity.AddComponent<SphereColliderComponent>();
				break;
			case StaticMesh::Default::Capsule:
				entity.AddComponent<RigidBodyComponent>();
				entity.AddComponent<CapsuleColliderComponent>();
				importOptions.MeshTransformation.SetRotationEuler({ 0.0f, 0.0f, 90.0f });
				break;
			case StaticMesh::Default::Cone:
				break;
			case StaticMesh::Default::Cylinder:
				break;
			case StaticMesh::Default::Plane:
				break;
			case StaticMesh::Default::Torus:
				break;
		}
		
		staticMeshRendererComponent.StaticMesh = StaticMesh::Create(defaultMesh, entity.GetTransform(), importOptions, (int)(entt::entity)entity);
		entity.GetTransform().Translation = editorCamera->GetFocalPoint();
	}

	void SceneHierarchyPanel::DisplayCreateEntityMenu(const EditorCamera* editorCamera)
	{
		if (Gui::MenuItem("Create Empty"))
		{
			m_SelectedEntity = m_ContextScene->CreateEntity("Empty Entity");
			m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
		}

		if (Gui::BeginMenu("Create 3D"))
		{
			if (Gui::MenuItem("Cube"))
				CreateDefaultMesh("Cube", StaticMesh::Default::Cube, m_SelectedEntity, m_ContextScene, editorCamera);

			if (Gui::MenuItem("Sphere"))
				CreateDefaultMesh("Sphere", StaticMesh::Default::Sphere, m_SelectedEntity, m_ContextScene, editorCamera);

			if (Gui::MenuItem("Capsule"))
				CreateDefaultMesh("Capsule", StaticMesh::Default::Capsule, m_SelectedEntity, m_ContextScene, editorCamera);

			if (Gui::MenuItem("Cone"))
				CreateDefaultMesh("Cone", StaticMesh::Default::Cone, m_SelectedEntity, m_ContextScene, editorCamera);

			if (Gui::MenuItem("Cylinder"))
				CreateDefaultMesh("Cylinder", StaticMesh::Default::Cylinder, m_SelectedEntity, m_ContextScene, editorCamera);

			if (Gui::MenuItem("Plane"))
				CreateDefaultMesh("Plane", StaticMesh::Default::Plane, m_SelectedEntity, m_ContextScene, editorCamera);

			if (Gui::MenuItem("Torus"))
				CreateDefaultMesh("Torus", StaticMesh::Default::Torus, m_SelectedEntity, m_ContextScene, editorCamera);

			Gui::EndMenu();
		}

		if (Gui::BeginMenu("Create 2D"))
		{
			if (Gui::MenuItem("Quad"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Quad");
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
				m_SelectedEntity.GetTransform().Translation.z = 0.0f;
				m_SelectedEntity.AddComponent<SpriteRendererComponent>();
				m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				m_SelectedEntity.AddComponent<BoxCollider2DComponent>();
			}

			if (Gui::MenuItem("Circle"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Circle");
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
				m_SelectedEntity.GetTransform().Translation.z = 0.0f;
				m_SelectedEntity.AddComponent<CircleRendererComponent>();
				m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				m_SelectedEntity.AddComponent<CircleCollider2DComponent>();
			}

			Gui::EndMenu();
		}

		if (Gui::BeginMenu("Camera"))
		{
			if (Gui::MenuItem("Perspective"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Camera");
				auto& cameraComponent = m_SelectedEntity.AddComponent<CameraComponent>();
				cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			if (Gui::MenuItem("Orthographic"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Camera");
				auto& cameraComponent = m_SelectedEntity.AddComponent<CameraComponent>();
				cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}
			Gui::EndMenu();
		}

		if (Gui::BeginMenu("Light"))
		{
			if (Gui::MenuItem("Directional"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Directional Light");
				LightSourceComponent& lightSourceComponent = m_SelectedEntity.AddComponent<LightSourceComponent>();
				lightSourceComponent.Type = LightType::Directional;
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
				Renderer::CreateShadowMap(lightSourceComponent.Type, lightSourceComponent.Source);
			}

			if (Gui::MenuItem("Point"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Point Light");
				LightSourceComponent& lightSourceComponent = m_SelectedEntity.AddComponent<LightSourceComponent>();
				lightSourceComponent.Type = LightType::Point;
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
				Renderer::CreateShadowMap(lightSourceComponent.Type, lightSourceComponent.Source);
			}

			if (Gui::MenuItem("Spot"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Spot Light");
				LightSourceComponent& lightSourceComponent = m_SelectedEntity.AddComponent<LightSourceComponent>();
				lightSourceComponent.Type = LightType::Spot;
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
				Renderer::CreateShadowMap(lightSourceComponent.Type, lightSourceComponent.Source);
			}

			Gui::EndMenu();
		}

		if (Gui::BeginMenu("Physics"))
		{
			if (Gui::MenuItem("Box Collider"))
			{
				CreateDefaultMesh("Box Collider", StaticMesh::Default::Cube, m_SelectedEntity, m_ContextScene, editorCamera);
				m_SelectedEntity.AddComponent<RigidBodyComponent>();
				m_SelectedEntity.AddComponent<BoxColliderComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			if (Gui::MenuItem("Sphere Collider"))
			{
				CreateDefaultMesh("Sphere Collider", StaticMesh::Default::Sphere, m_SelectedEntity, m_ContextScene, editorCamera);
				m_SelectedEntity.AddComponent<RigidBodyComponent>();
				m_SelectedEntity.AddComponent<SphereColliderComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			if (Gui::MenuItem("Capsule Collider"))
			{
				CreateDefaultMesh("Capsule Collider", StaticMesh::Default::Capsule, m_SelectedEntity, m_ContextScene, editorCamera);
				m_SelectedEntity.AddComponent<RigidBodyComponent>();
				m_SelectedEntity.AddComponent<CapsuleColliderComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			if (Gui::MenuItem("Mesh Collider"))
			{
				CreateDefaultMesh("Mesh Collider", StaticMesh::Default::Cube, m_SelectedEntity, m_ContextScene, editorCamera);
				m_SelectedEntity.AddComponent<RigidBodyComponent>();
				m_SelectedEntity.AddComponent<MeshColliderComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			if (Gui::MenuItem("Fixed Joint"))
			{
				CreateDefaultMesh("Fixed Joint", StaticMesh::Default::Cube, m_SelectedEntity, m_ContextScene, editorCamera);
				m_SelectedEntity.AddComponent<RigidBodyComponent>();
				m_SelectedEntity.AddComponent<FixedJointComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			if (Gui::MenuItem("Box Collider 2D"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Box Collider2D");
				m_SelectedEntity.AddComponent<SpriteRendererComponent>();
				m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				m_SelectedEntity.AddComponent<BoxCollider2DComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			if (Gui::MenuItem("Circle Collider 2D"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Circle Collider2D");
				m_SelectedEntity.AddComponent<CircleRendererComponent>();
				m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				m_SelectedEntity.AddComponent<CircleCollider2DComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}

		if (Gui::BeginMenu("Audio"))
		{
			if (Gui::MenuItem("Source Entity"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Audio Source");
				m_SelectedEntity.AddComponent<AudioSourceComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			if (Gui::MenuItem("Listener Entity"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Audio Listener");
				m_SelectedEntity.AddComponent<AudioListenerComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}

		if (Gui::BeginMenu("UI"))
		{
			if (Gui::MenuItem("Text"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Text");
				m_SelectedEntity.AddComponent<TextMeshComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}

		if (Gui::BeginMenu("Effects"))
		{
			if (Gui::MenuItem("Particles"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Particle Emitter");
				m_SelectedEntity.AddComponent<ParticleEmitterComponent>();
				m_SelectedEntity.GetTransform().Translation = editorCamera->GetFocalPoint();
			}

			Gui::EndMenu();
		}
	}

	void SceneHierarchyPanel::DisplayInsectorPanel(Entity hoveredEntity)
	{
		Gui::Begin("Inspector", &s_ShowInspectorPanel);

		if (m_SelectedEntity)
			DrawComponents(m_SelectedEntity);
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
					Gui::Text((const char*)VX_ICON_VIDEO_CAMERA);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<CameraComponent>("Camera");

					Gui::Text((const char*)VX_ICON_SKYATLAS);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<SkyboxComponent>("Skybox");

					Gui::Text((const char*)VX_ICON_LIGHTBULB_O);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<LightSourceComponent>("Light Source");

					Gui::Text((const char*)VX_ICON_HOME);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<MeshRendererComponent>("Mesh Renderer");

					Gui::Text((const char*)VX_ICON_CUBE);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<StaticMeshRendererComponent>("Static Mesh Renderer");

					Gui::Text((const char*)VX_ICON_LIGHTBULB_O);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<LightSource2DComponent>("Light Source 2D");

					Gui::Text((const char*)VX_ICON_SPINNER);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<SpriteRendererComponent>("Sprite Renderer");

					Gui::Text((const char*)VX_ICON_CIRCLE);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<CircleRendererComponent>("Circle Renderer");

					Gui::Text((const char*)VX_ICON_BOMB);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<ParticleEmitterComponent>("Particle Emitter");

					Gui::Text((const char*)VX_ICON_TEXT_HEIGHT);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<TextMeshComponent>("Text Mesh");

					Gui::Text((const char*)VX_ICON_CLOCK_O);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<AnimatorComponent>("Animator");

					Gui::Text((const char*)VX_ICON_ADJUST);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<AnimationComponent>("Animation");

					Gui::EndMenu();
				}

				if (Gui::BeginMenu("Physics"))
				{
					DisplayComponentMenuItem<RigidBodyComponent>("RigidBody");
					DisplayComponentMenuItem<CharacterControllerComponent>("Character Controller");
					DisplayComponentMenuItem<FixedJointComponent>("Fixed Joint");
					DisplayComponentMenuItem<BoxColliderComponent>("Box Collider");
					DisplayComponentMenuItem<SphereColliderComponent>("Sphere Collider");
					DisplayComponentMenuItem<CapsuleColliderComponent>("Capsule Collider");
					DisplayComponentMenuItem<MeshColliderComponent>("Mesh Collider");

					DisplayComponentMenuItem<RigidBody2DComponent>("RigidBody 2D");
					DisplayComponentMenuItem<BoxCollider2DComponent>("Box Collider 2D");
					DisplayComponentMenuItem<CircleCollider2DComponent>("Circle Collider 2D");

					Gui::EndMenu();
				}

				if (Gui::BeginMenu("Audio"))
				{
					Gui::Text((const char*)VX_ICON_VOLUME_UP);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<AudioSourceComponent>("Audio Source");

					Gui::Text((const char*)VX_ICON_HEADPHONES);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<AudioListenerComponent>("Audio Listener");

					Gui::EndMenu();
				}

				if (Gui::BeginMenu("AI"))
				{
					Gui::Text((const char*)VX_ICON_LAPTOP);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<NavMeshAgentComponent>("Nav Mesh Agent");

					Gui::EndMenu();
				}

				Gui::Text((const char*)VX_ICON_FILE_CODE_O);
				Gui::SameLine();
				Gui::AlignTextToFramePadding();
				DisplayComponentMenuItem<ScriptComponent>("Script");
			}
			else
			{
				if (const char* name = "Camera"; m_ComponentSearchInputTextFilter.PassFilter(name))
				{
					Gui::Text((const char*)VX_ICON_VIDEO_CAMERA);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<CameraComponent>(name);
				}

				if (const char* componentName = "Skybox"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_SKYATLAS);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<SkyboxComponent>(componentName);
				}

				if (const char* componentName = "Light Source"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_LIGHTBULB_O);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<LightSourceComponent>(componentName);
				}

				if (const char* componentName = "Mesh Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_HOME);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<MeshRendererComponent>(componentName);
				}

				if (const char* componentName = "Static Mesh Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_CUBE);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<StaticMeshRendererComponent>(componentName);
				}

				if (const char* componentName = "Light Source 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_LIGHTBULB_O);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<LightSource2DComponent>(componentName);
				}

				if (const char* componentName = "Sprite Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_SPINNER);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<SpriteRendererComponent>(componentName);
				}

				if (const char* componentName = "Circle Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_CIRCLE);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<CircleRendererComponent>(componentName);
				}
				
				if (const char* componentName = "Particle Emitter"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_BOMB);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<ParticleEmitterComponent>(componentName);
				}
				
				if (const char* componentName = "Text Mesh"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_TEXT_HEIGHT);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<TextMeshComponent>(componentName);
				}
				
				if (const char* componentName = "Animator"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_CLOCK_O);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<AnimatorComponent>(componentName);
				}
				
				if (const char* componentName = "Animation"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_ADJUST);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<AnimationComponent>(componentName);
				}

				if (const char* componentName = "RigidBody"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<RigidBodyComponent>(componentName);
				if (const char* componentName = "Character Controller"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<CharacterControllerComponent>(componentName);
				if (const char* componentName = "Fixed Joint"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<FixedJointComponent>(componentName);
				if (const char* componentName = "Box Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<BoxColliderComponent>(componentName);
				if (const char* componentName = "Sphere Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<SphereColliderComponent>(componentName);
				if (const char* componentName = "Capsule Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<CapsuleColliderComponent>(componentName);
				if (const char* componentName = "Mesh Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<MeshColliderComponent>(componentName);

				if (const char* componentName = "Audio Source"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_VOLUME_UP);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<AudioSourceComponent>(componentName);
				}

				if (const char* componentName = "Audio Listener"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_HEADPHONES);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<AudioListenerComponent>(componentName);
				}

				if (const char* componentName = "RigidBody 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<RigidBody2DComponent>(componentName);
				if (const char* componentName = "Box Collider 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<BoxCollider2DComponent>(componentName);
				if (const char* componentName = "Circle Collider 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
					DisplayComponentMenuItem<CircleCollider2DComponent>(componentName);

				if (const char* componentName = "Nav Mesh Agent"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_LAPTOP);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<NavMeshAgentComponent>(componentName);
				}

				if (const char* componentName = "Script"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				{
					Gui::Text((const char*)VX_ICON_FILE_CODE_O);
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					DisplayComponentMenuItem<ScriptComponent>(componentName);
				}
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
			buffer.reserve(10);
		}
		else
		{
			buffer.resize(markerSize);
			memcpy(buffer.data(), marker.data(), markerSize);
		}

		if (Gui::InputText("##Marker", buffer.data(), sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue) && !buffer.empty())
		{
			tagComponent.Marker = buffer;
			tagComponent.AddMarker(buffer);
			Gui::SetWindowFocus("Scene");
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, const EditorCamera* editorCamera)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		const auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0)
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
			m_SelectedEntity = entity;
			m_EntityShouldBeRenamed = false;
		}

		m_EntityShouldBeDestroyed = false;
		
		// Right-click on entity for utilities popup
		if (Gui::BeginPopupContextItem())
		{
			if (Gui::MenuItem("Rename", "F2"))
			{
				m_SelectedEntity = entity;
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
				SetSelectedEntity(childEntity);
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

			if (Gui::MenuItem("Delete Entity", "Del") && m_SelectedEntity)
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
		if (m_EntityShouldBeDestroyed && m_SelectedEntity == entity)
		{
			m_SelectedEntity = {};
			m_EntityShouldBeDestroyed = false;

			m_ContextScene->DestroyEntity(entity);
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
						component.StaticMesh = StaticMesh::Create(StaticMesh::Default::Cube, entity.GetTransform(), MeshImportOptions(), (int)(entt::entity)entity);
					}
					else if constexpr (std::is_same<TComponent, AudioSourceComponent>())
					{
						SharedRef<AudioSource> audioSource = component.Source;
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

			ImGuiInputTextFlags flags = (m_EntityShouldBeRenamed && m_SelectedEntity == entity) ?
				ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue : 0;

			if (m_SelectedEntity == entity && m_EntityShouldBeRenamed)
				Gui::SetKeyboardFocusHere();
			if (Gui::InputTextWithHint("##Tag", "Entity Name", buffer, sizeof(buffer), flags))
			{
				tag = std::string(buffer);

				// Set the focus to the scene panel otherwise the keyboard focus will still be on the input text box
				if (m_EntityShouldBeRenamed)
					Gui::SetWindowFocus("Scene");

				m_EntityShouldBeRenamed = false;
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
			if (auto currentMarkerIt = std::find(markers.begin(), markers.end(), tagComponent.Marker);
				currentMarkerIt != markers.end()
			) {
				const char* currentMarker = (*currentMarkerIt).c_str();

				if (Gui::BeginCombo("##Marker", currentMarker, ImGuiComboFlags_HeightLarge))
				{
					uint32_t arraySize = markers.size();

					for (uint32_t i = 0; i < arraySize; i++)
					{
						bool isSelected = strcmp(currentMarker, markers[i].c_str()) == 0;
						if (Gui::Selectable(markers[i].c_str(), isSelected))
						{
							currentMarker = markers[i].c_str();
							tagComponent.Marker = markers[i];
						}

						if (isSelected)
							Gui::SetItemDefaultFocus();

						if (i != arraySize - 1)
							UI::Draw::Underline();
						else // The last marker in the markers vector
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
				Math::ivec2 viewportSize = m_ContextScene->GetViewportSize();
				camera.SetViewportSize(viewportSize.x, viewportSize.y);
			}
		});

		DrawComponent<SkyboxComponent>("Skybox", entity, [](auto& component)
		{
			SharedRef<Skybox> skybox = component.Source;
			
			UI::BeginPropertyGrid();

			std::string relativeSkyboxPath;

			if (skybox->IsLoaded())
			{
				std::string skyboxSourcePath = skybox->GetFilepath();
				relativeSkyboxPath = FileSystem::Relative(skyboxSourcePath, Project::GetAssetDirectory()).string();
			}
			else
			{
				relativeSkyboxPath = "";
			}

			UI::Property("Source", relativeSkyboxPath, true);

			// Accept a Skybox Directory from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path skyboxPath = std::filesystem::path(path);

					// Make sure we are recieving an actual directory or hdr texture otherwise we will have trouble loading it
					if (skyboxPath.filename().extension() == ".hdr")
					{
						skybox->LoadFromFilepath(skyboxPath.string());
						skybox->SetShouldReload(true);
					}
					else
						VX_CONSOLE_LOG_WARN("Could not load skybox, not a '.hdr' - {}", skyboxPath.filename().string());
				}

				Gui::EndDragDropTarget();
			}

			UI::EndPropertyGrid();

			if (skybox->IsLoaded())
			{
				UI::BeginPropertyGrid();

				UI::Property("Rotation", component.Rotation);

				if (Gui::IsItemFocused())
				{
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
					if (castShadows && component.Type == LightType::Directional)
					{
						Renderer::CreateShadowMap(LightType::Directional, lightSource);
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

			std::string relativeMeshPath = "";
			
			if (component.Mesh)
			{
				std::string meshSourcePath = component.Mesh->GetPath();
				relativeMeshPath = FileSystem::Relative(meshSourcePath, Project::GetAssetDirectory()).string();
			}

			UI::Property("Mesh Source", relativeMeshPath, true);

			// Accept a Model File from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path modelFilepath = std::filesystem::path(path);

					// Make sure we are recieving an actual model file otherwise we will have trouble opening it
					if (modelFilepath.filename().extension() == ".obj" || modelFilepath.filename().extension() == ".fbx" || modelFilepath.filename().extension() == ".gltf" || modelFilepath.filename().extension() == ".dae" || modelFilepath.filename().extension() == ".glb")
					{
						component.Mesh = Mesh::Create(modelFilepath.string(), entity.GetTransform(), MeshImportOptions(), (int)(entt::entity)entity);

						if (entity.HasComponent<AnimatorComponent>() && entity.HasComponent<AnimationComponent>() && component.Mesh->HasAnimations())
						{
							AnimatorComponent& animatorComponent = entity.GetComponent<AnimatorComponent>();
							AnimationComponent& animationComponent = entity.GetComponent<AnimationComponent>();

							animationComponent.Animation = Animation::Create(modelFilepath.string(), component.Mesh);
							animatorComponent.Animator = Animator::Create(animationComponent.Animation);
						}
					}
					else
						VX_CONSOLE_LOG_WARN("Could not load model file - {}", modelFilepath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			UI::EndPropertyGrid();

			// TODO materials ///////////////////////////////////////////////////////////////////////////////////////////////////////
		});

		DrawComponent<StaticMeshRendererComponent>("Static Mesh Renderer", entity, [&](auto& component)
		{
			UI::BeginPropertyGrid();

			std::string meshSourcePath = "";

			if (component.StaticMesh)
			{
				meshSourcePath = component.StaticMesh->GetPath();
				if (StaticMesh::IsDefaultMesh(meshSourcePath))
					UI::Property("Mesh Source", meshSourcePath, true);
				else
				{
					std::string relativeMeshPath = FileSystem::Relative(meshSourcePath, Project::GetAssetDirectory()).string();
					UI::Property("Mesh Source", relativeMeshPath, true);
				}
			}

			// Accept a Model File from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path modelFilepath = std::filesystem::path(path);

					// Make sure we are recieving an actual model file otherwise we will have trouble opening it
					if (modelFilepath.filename().extension() == ".obj" || modelFilepath.filename().extension() == ".fbx" || modelFilepath.filename().extension() == ".gltf" || modelFilepath.filename().extension() == ".dae" || modelFilepath.filename().extension() == ".glb")
					{
						component.StaticMesh = StaticMesh::Create(modelFilepath.string(), entity.GetTransform(), MeshImportOptions(), (int)(entt::entity)entity);
						component.Type = MeshType::Custom;
					}
					else
						VX_CONSOLE_LOG_WARN("Could not load model file - {}", modelFilepath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			static const char* meshTypes[] = { "Cube", "Sphere", "Capsule", "Cone", "Cylinder", "Plane", "Torus", "Custom" };
			int32_t currentMeshType = (int32_t)component.Type;
			if (UI::PropertyDropdown("Mesh Type", meshTypes, VX_ARRAYCOUNT(meshTypes), currentMeshType))
			{
				component.Type = (MeshType)currentMeshType;

				if (component.Type == MeshType::Capsule)
				{
					MeshImportOptions importOptions = MeshImportOptions();
					importOptions.MeshTransformation.SetRotationEuler({ 0.0f, 0.0f, 90.0f });
					component.StaticMesh = StaticMesh::Create((StaticMesh::Default)currentMeshType, entity.GetTransform(), importOptions, (int)(entt::entity)entity);
				}
				else if (component.Type != MeshType::Custom)
					component.StaticMesh = StaticMesh::Create((StaticMesh::Default)currentMeshType, entity.GetTransform(), MeshImportOptions(), (int)(entt::entity)entity);
				else
					component.StaticMesh = StaticMesh::Create(meshSourcePath, entity.GetTransform(), MeshImportOptions(), (int)(entt::entity)entity);
			}

			UI::EndPropertyGrid();

			// TODO materials ///////////////////////////////////////////////////////////////////////////////////////////////////////
		});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [&](auto& component)
		{
			UI::BeginPropertyGrid();

			// Texutre
			{
				SharedReference<Texture2D> icon = EditorResources::CheckerboardIcon;

				if (component.Texture)
					icon = AssetManager::GetAsset<Texture2D>(component.Texture);
				ImVec4 tintColor = { component.SpriteColor.r, component.SpriteColor.g, component.SpriteColor.b, component.SpriteColor.a };

				if (UI::ImageButton("Texture", icon, { 64, 64 }, { 0, 0, 0, 0 }, tintColor))
					component.Texture = EditorResources::CheckerboardIcon->Handle;
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
						if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".jpeg" || texturePath.filename().extension() == ".tga" || texturePath.filename().extension() == ".psd")
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
							VX_CONSOLE_LOG_WARN("Could not load texture", texturePath.filename().string());
					}
					Gui::EndDragDropTarget();
				}
			}

			UI::Property("Color", &component.SpriteColor);
			UI::Property("UV", component.Scale, 0.05f);

			UI::EndPropertyGrid();
		});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
		{
			UI::BeginPropertyGrid();

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

			std::string fontFilepath = component.FontAsset->GetFontAtlas()->GetPath();

			if (Font::GetDefaultFont()->GetFontAtlas()->GetPath() == fontFilepath)
				UI::Property("Font Source", fontFilepath, true);
			else
			{
				std::string relativeFontPath = FileSystem::Relative(fontFilepath, Project::GetAssetDirectory()).string();
				UI::Property("Font Source", relativeFontPath, true);
			}

			// Accept a Font from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path fontPath = std::filesystem::path(path);

					// Make sure we are recieving an actual font otherwise we will have trouble opening it
					if (fontPath.filename().extension() == ".ttf" || fontPath.filename().extension() == ".TTF")
					{
						SharedRef<Font> font = Font::Create(fontPath.string());

						if (font->GetFontAtlas()->IsLoaded())
							component.FontAsset = font;
						else
							VX_CONSOLE_LOG_WARN("Could not load font {}", fontPath.filename().string());
					}
					else
						VX_CONSOLE_LOG_WARN("Could not load font, not a '.tff' - {}", fontPath.filename().string());
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
					if (audioSourcePath.filename().extension() == ".wav" || audioSourcePath.filename().extension() == ".mp3")
					{
						// If there is another file playing we need to stop it
						if (component.Source->IsPlaying())
							component.Source->Stop();

						SharedRef<AudioSource> audioSource = component.Source;
						audioSource->SetPath(audioSourcePath.string());
						audioSource->Reload();
					}
					else
						VX_CONSOLE_LOG_WARN("Could not load audio file, not a '.wav' or '.mp3' - {}", audioSourcePath.filename().string());
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
				AudioSource::SoundProperties& props = component.Source->GetProperties();

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

			bool sceneRunning = m_ContextScene->IsRunning();

			// Fields
			if (sceneRunning)
			{
				SharedRef<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());

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
					}
				}
			}
			else
			{
				if (scriptClassExists)
				{
					SharedRef<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
					const auto& fields = entityClass->GetFields();

					auto& entityClassFields = ScriptEngine::GetScriptFieldMap(entity);
					for (const auto& [name, field] : fields)
					{
						auto it = entityClassFields.find(name);

						// Field has been set in editor
						if (it != entityClassFields.end())
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
						}
						else
						{
							// Display controls to set it
							if (field.Type == ScriptFieldType::Float)
							{
								float data = 0.0f;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Double)
							{
								double data = 0.0f;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Vector2)
							{
								Math::vec2 data = Math::vec2(0.0f);
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Vector3)
							{
								Math::vec3 data = Math::vec3(0.0f);
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Vector4)
							{
								Math::vec4 data = Math::vec4(0.0f);
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Color3)
							{
								Math::vec3 data = Math::vec3(0.0f);
								if (UI::Property(name.c_str(), &data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Color4)
							{
								Math::vec4 data = Math::vec4(0.0f, 0.0f, 0.0f, 1.0f);
								if (UI::Property(name.c_str(), &data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Bool)
							{
								bool data = false;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Char)
							{
								char data = 0;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Short)
							{
								short data = 0;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Int)
							{
								int data = 0;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Long)
							{
								long long data = 0;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Byte)
							{
								unsigned char data = 0;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::UShort)
							{
								unsigned short data = 0;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::UInt)
							{
								unsigned int data = 0;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::ULong)
							{
								unsigned long long data = 0;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance& fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Entity)
							{
								uint64_t data;
								if (UI::Property(name.c_str(), data))
								{
									ScriptFieldInstance fieldInstance = entityClassFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}

								if (Gui::BeginDragDropTarget())
								{
									if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
									{
										Entity& droppedEntity = *((Entity*)payload->Data);
										ScriptFieldInstance fieldInstance = entityClassFields[name];
										fieldInstance.Field = field;
										fieldInstance.SetValue(droppedEntity.GetUUID());
									}

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
