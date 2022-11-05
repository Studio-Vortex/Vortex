#include "SceneHierarchyPanel.h"

#include "Sparky/Scripting/ScriptEngine.h"
#include "Sparky/Audio/AudioSource.h"
#include "Sparky/Scene/Scene.h"
#include "Sparky/Renderer/LightSource.h"
#include "Sparky/Renderer/ParticleEmitter.h"
#include "Sparky/Renderer/Skybox.h"

#include <imgui_internal.h>

namespace Sparky {

	extern const std::filesystem::path g_AssetPath;

	SceneHierarchyPanel::SceneHierarchyPanel(const SharedRef<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const SharedRef<Scene>& context)
	{
		m_ContextScene = context;
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

	void SceneHierarchyPanel::OnGuiRender(Entity hoveredEntity)
	{
		if (s_ShowSceneHierarchyPanel)
		{
			Gui::Begin("Scene Hierarchy", &s_ShowSceneHierarchyPanel);

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
				DisplayCreateEntityMenu();

				Gui::EndPopup();
			}

			Gui::Spacing();
			Gui::Separator();

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

				m_ContextScene->m_Registry.each([&](auto entityID)
					{
						Entity entity{ entityID, m_ContextScene.get() };

						// If the name lines up with the search box we can show it
						if (m_EntitySearchInputTextFilter.PassFilter(entity.GetName().c_str()))
							DrawEntityNode(entity);
					});

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
					DisplayCreateEntityMenu();

					Gui::EndPopup();
				}
			}

			Gui::End();
		}

		if (s_ShowInspectorPanel)
		{
			Gui::Begin("Inspector", &s_ShowInspectorPanel);

			if (m_SelectedEntity)
				DrawComponents(m_SelectedEntity);
			else
			{
				const char* name = "None";

				if (hoveredEntity && m_ContextScene)
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
	}

	void SceneHierarchyPanel::DisplayCreateEntityMenu()
	{
		if (Gui::MenuItem("Create Empty"))
			m_SelectedEntity = m_ContextScene->CreateEntity("Empty Entity");
		Gui::Separator();

		if (Gui::BeginMenu("Create 3D"))
		{
			if (Gui::MenuItem("Cube"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Cube");
				m_SelectedEntity.AddComponent<MeshRendererComponent>();
			}
			Gui::Separator();

			if (Gui::MenuItem("Sphere"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Sphere");
				m_SelectedEntity.AddComponent<MeshRendererComponent>().Type = MeshRendererComponent::MeshType::Sphere;
			}
			Gui::Separator();

			if (Gui::MenuItem("Capsule"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Capsule");
				m_SelectedEntity.AddComponent<MeshRendererComponent>().Type = MeshRendererComponent::MeshType::Capsule;
			}
			Gui::Separator();

			if (Gui::MenuItem("Cone"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Cone");
				m_SelectedEntity.AddComponent<MeshRendererComponent>().Type = MeshRendererComponent::MeshType::Cone;
			}
			Gui::Separator();

			if (Gui::MenuItem("Cylinder"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Cylinder");
				m_SelectedEntity.AddComponent<MeshRendererComponent>().Type = MeshRendererComponent::MeshType::Cylinder;
			}
			Gui::Separator();

			if (Gui::MenuItem("Plane"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Plane");
				m_SelectedEntity.AddComponent<MeshRendererComponent>().Type = MeshRendererComponent::MeshType::Plane;
			}
			Gui::Separator();

			if (Gui::MenuItem("Torus"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Torus");
				m_SelectedEntity.AddComponent<MeshRendererComponent>().Type = MeshRendererComponent::MeshType::Torus;
			}

			Gui::EndMenu();
		}
		Gui::Separator();

		if (Gui::BeginMenu("Create 2D"))
		{
			if (Gui::MenuItem("Quad"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Quad");
				m_SelectedEntity.AddComponent<SpriteRendererComponent>();
			}

			Gui::Separator();

			if (Gui::MenuItem("Circle"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Circle");
				m_SelectedEntity.AddComponent<CircleRendererComponent>();
			}

			Gui::EndMenu();
		}
		Gui::Separator();

		if (Gui::BeginMenu("Camera"))
		{
			if (Gui::MenuItem("Perspective"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Camera");
				auto& cameraComponent = m_SelectedEntity.AddComponent<CameraComponent>();
				cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
			}
			Gui::Separator();

			if (Gui::MenuItem("Orthographic"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Camera");
				auto& cameraComponent = m_SelectedEntity.AddComponent<CameraComponent>();
				cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
			}
			Gui::EndMenu();
		}
		Gui::Separator();

		if (Gui::BeginMenu("Light"))
		{
			if (Gui::MenuItem("Directional"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Directional Light");
				m_SelectedEntity.AddComponent<LightSourceComponent>().Type = LightSourceComponent::LightType::Directional;
			}
			Gui::Separator();

			if (Gui::MenuItem("Point"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Point Light");
				m_SelectedEntity.AddComponent<LightSourceComponent>().Type = LightSourceComponent::LightType::Point;
			}
			Gui::Separator();
			
			if (Gui::MenuItem("Spot"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Spot Light");
				m_SelectedEntity.AddComponent<LightSourceComponent>().Type = LightSourceComponent::LightType::Spot;
			}

			Gui::EndMenu();
		}
		Gui::Separator();

		if (Gui::BeginMenu("Physics"))
		{
			if (Gui::MenuItem("Static Box Collider 2D"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Box Collider2D");
				m_SelectedEntity.AddComponent<SpriteRendererComponent>();
				m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				m_SelectedEntity.AddComponent<BoxCollider2DComponent>();
			}

			Gui::Separator();

			if (Gui::MenuItem("Static Circle Collider 2D"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Circle Collider2D");
				m_SelectedEntity.AddComponent<CircleRendererComponent>();
				m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				m_SelectedEntity.AddComponent<CircleCollider2DComponent>();
			}

			Gui::Separator();

			if (Gui::MenuItem("Dynamic Box Collider 2D"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Box Collider2D");
				m_SelectedEntity.AddComponent<SpriteRendererComponent>();
				auto& rb2d = m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				rb2d.Type = RigidBody2DComponent::BodyType::Dynamic;
				m_SelectedEntity.AddComponent<BoxCollider2DComponent>();
			}

			Gui::Separator();

			if (Gui::MenuItem("Dynamic Circle Collider 2D"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Circle Collider2D");
				m_SelectedEntity.AddComponent<CircleRendererComponent>();
				auto& cc2d = m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				cc2d.Type = RigidBody2DComponent::BodyType::Dynamic;
				m_SelectedEntity.AddComponent<CircleCollider2DComponent>();
			}

			Gui::EndMenu();
		}
		Gui::Separator();

		if (Gui::BeginMenu("Audio"))
		{
			if (Gui::MenuItem("Source Entity"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Audio Source");
				m_SelectedEntity.AddComponent<AudioSourceComponent>();
			}

			Gui::Separator();

			if (Gui::MenuItem("Listener Entity"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Audio Listener");
				m_SelectedEntity.AddComponent<AudioListenerComponent>();
			}

			Gui::EndMenu();
		}
		Gui::Separator();

		if (Gui::BeginMenu("Effects"))
		{
			if (Gui::MenuItem("Particles"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Particle Emitter");
				m_SelectedEntity.AddComponent<ParticleEmitterComponent>();
			}

			Gui::EndMenu();
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = Gui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (Gui::BeginDragDropSource())
		{
			UUID uuid = entity.GetUUID();
			Gui::SetDragDropPayload("SCENE_HIERARCHY_ITEM", (const void*)&uuid, sizeof(UUID), ImGuiCond_Once);
			Gui::EndDragDropSource();
		}

		if (Gui::IsItemClicked())
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
			Gui::Separator();

			if (Gui::MenuItem("Duplicate Entity", "Ctrl+D"))
			{
				m_ContextScene->DuplicateEntity(entity);
				Gui::CloseCurrentPopup();
			}
			Gui::Separator();

			if (Gui::MenuItem("Delete Entity", "Del"))
				m_EntityShouldBeDestroyed = true;

			Gui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = Gui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
				Gui::TreePop();
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

	static void DrawVec3Controls(const std::string& label, Math::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f, std::function<void()> uiCallback = nullptr)
	{
		ImGuiIO& io = Gui::GetIO();
		const auto& boldFont = io.Fonts->Fonts[0];

		Gui::PushID(label.c_str());

		Gui::Columns(2);
		Gui::SetColumnWidth(0, columnWidth);
		Gui::Text(label.c_str());
		Gui::NextColumn();

		Gui::PushMultiItemsWidths(3, Gui::CalcItemWidth());
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("X", buttonSize))
		{
			values.x = resetValue;

			if (uiCallback != nullptr)
				uiCallback();
		}
		Gui::PopFont();
		Gui::PopStyleColor(3);

		Gui::SameLine();
		if (Gui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f") && uiCallback != nullptr)
			uiCallback();
		Gui::PopItemWidth();
		Gui::SameLine();

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("Y", buttonSize))
		{
			values.y = resetValue;

			if (uiCallback != nullptr)
				uiCallback();
		}
		Gui::PopFont();
		Gui::PopStyleColor(3);

		Gui::SameLine();
		if (Gui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f") && uiCallback != nullptr)
			uiCallback();
		Gui::PopItemWidth();
		Gui::SameLine();

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("Z", buttonSize))
		{
			values.z = resetValue;

			if (uiCallback != nullptr)
				uiCallback();
		}
		Gui::PopFont();
		Gui::PopStyleColor(3);

		Gui::SameLine();
		if (Gui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f") && uiCallback != nullptr)
			uiCallback();
		Gui::PopItemWidth();

		Gui::PopStyleVar();
		Gui::Columns(1);
		Gui::PopID();
	}

	template <typename TComponent, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiCallback, bool removeable = true)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		static SharedRef<Texture2D> settingsIcon = Texture2D::Create("Resources/Icons/Inspector/SettingsIcon.png");

		if (entity.HasComponent<TComponent>())
		{
			auto& component = entity.GetComponent<TComponent>();
			ImVec2 contentRegionAvailable = Gui::GetContentRegionAvail();

			Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			Gui::Separator();
			bool open = Gui::TreeNodeEx((void*)typeid(TComponent).hash_code(), treeNodeFlags, name.c_str());
			Gui::PopStyleVar();
			Gui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (Gui::ImageButton((void*)settingsIcon->GetRendererID(), ImVec2{ lineHeight - 5.5f, lineHeight - 5.5f }))
				Gui::OpenPopup("ComponentSettings");

			bool componentShouldBeRemoved = false;
			if (Gui::BeginPopup("ComponentSettings"))
			{
				if (Gui::MenuItem("Copy Component"))
				{
					// TODO: Copy Component
					Gui::CloseCurrentPopup();
				}
				if (removeable)
					Gui::Separator();

				if (removeable && Gui::MenuItem("Remove Component"))
					componentShouldBeRemoved = true;

				Gui::EndPopup();
			}

			if (open)
			{
				uiCallback(component);
				Gui::TreePop();
			}

			if (componentShouldBeRemoved)
				entity.RemoveComponent<TComponent>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

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
		}

		Gui::SameLine();
		Gui::PushItemWidth(-1);

		bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		if (Gui::Button("Add Component") || (Input::IsKeyPressed(Key::A) && controlPressed && shiftPressed && Gui::IsWindowHovered()))
		{
			Gui::OpenPopup("AddComponent");

			// We should reset the search bar here
			memset(m_ComponentSearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
			m_ComponentSearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed
		}

		if (Gui::BeginPopup("AddComponent"))
		{
			// Search Bar + Filtering
			bool isSearching = Gui::InputTextWithHint("##ComponentSearch", "Search", m_ComponentSearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
			if (isSearching)
				m_ComponentSearchInputTextFilter.Build();

			Gui::Spacing();
			Gui::Separator();

			static const char* componentNames[] = {
				"Camera",
				"Skybox",
				"Light Source",
				"Mesh Renderer",
				"Sprite Renderer",
				"Circle Renderer",
				"Particle Emitter",
				"Audio Source",
				"Audio Listener",
				"RigidBody 2D",
				"Box Collider 2D",
				"Circle Collider 2D",
				"C# Script",
				"C++ Script"
			};

			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[0]))
				DisplayAddComponentPopup<CameraComponent>(componentNames[0]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[1]))
				DisplayAddComponentPopup<SkyboxComponent>(componentNames[1]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[2]))
				DisplayAddComponentPopup<LightSourceComponent>(componentNames[2]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[3]))
				DisplayAddComponentPopup<MeshRendererComponent>(componentNames[3]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[4]))
				DisplayAddComponentPopup<SpriteRendererComponent>(componentNames[4]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[5]))
				DisplayAddComponentPopup<CircleRendererComponent>(componentNames[5]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[6]))
				DisplayAddComponentPopup<ParticleEmitterComponent>(componentNames[6]);

			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[7]))
				DisplayAddComponentPopup<AudioSourceComponent>(componentNames[7]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[8]))
				DisplayAddComponentPopup<AudioListenerComponent>(componentNames[8]);

			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[9]))
				DisplayAddComponentPopup<RigidBody2DComponent>(componentNames[9]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[10]))
			DisplayAddComponentPopup<BoxCollider2DComponent>(componentNames[10]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[11]))
				DisplayAddComponentPopup<CircleCollider2DComponent>(componentNames[11]);

			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[12]))
				DisplayAddComponentPopup<ScriptComponent>(componentNames[12]);
			if (m_ComponentSearchInputTextFilter.PassFilter(componentNames[13]))
				DisplayAddComponentPopup<NativeScriptComponent>(componentNames[13], true);

			Gui::EndPopup();
		}

		Gui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
		{
			DrawVec3Controls("Translation", component.Translation);
			Math::vec3 rotation = Math::Rad2Deg(component.Rotation);
			DrawVec3Controls("Rotation", rotation, 0.0f, 100.0f, [&]()
			{
				component.Rotation = Math::Deg2Rad(rotation);
			});
			DrawVec3Controls("Scale", component.Scale, 1.0f);
		}, false);

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
		{
			auto& camera = component.Camera;

			Gui::Checkbox("Primary", &component.Primary);

			const char* projectionTypes[] = { "Perspective", "Othrographic" };
			const char* currentProjectionType = projectionTypes[(uint32_t)camera.GetProjectionType()];

			if (Gui::BeginCombo("Projection", currentProjectionType))
			{
				uint32_t arraySize = SP_ARRAYCOUNT(projectionTypes);

				for (uint32_t i = 0; i < arraySize; i++)
				{
					bool isSelected = strcmp(currentProjectionType, projectionTypes[i]) == 0;
					if (Gui::Selectable(projectionTypes[i], isSelected))
					{
						currentProjectionType = projectionTypes[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();

					if (i != arraySize - 1)
						Gui::Separator();
				}

				Gui::EndCombo();
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float verticalFov = Math::Rad2Deg(camera.GetPerspectiveVerticalFOV());
				if (Gui::DragFloat("Vertical FOV", &verticalFov))
					camera.SetPerspectiveVerticalFOV(Math::Deg2Rad(verticalFov));

				float nearClip = camera.GetPerspectiveNearClip();
				if (Gui::DragFloat("Near", &nearClip))
					camera.SetPerspectiveNearClip(nearClip);

				float farClip = camera.GetPerspectiveFarClip();
				if (Gui::DragFloat("Far", &farClip))
					camera.SetPerspectiveFarClip(farClip);
			}
			else
			{
				float orthoSize = camera.GetOrthographicSize();
				if (Gui::DragFloat("Size", &orthoSize))
					camera.SetOrthographicSize(orthoSize);

				float nearClip = camera.GetOrthographicNearClip();
				if (Gui::DragFloat("Near", &nearClip))
					camera.SetOrthographicNearClip(nearClip);

				float farClip = camera.GetOrthographicFarClip();
				if (Gui::DragFloat("Far", &farClip))
					camera.SetOrthographicFarClip(farClip);

				Gui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
			}
		});

		DrawComponent<SkyboxComponent>("Skybox", entity, [](auto& component)
		{
			SharedRef<Skybox> skybox = component.Source;
			
			char buffer[256];

			std::string skyboxPath = skybox->GetDirectoryPath();

			if (!skyboxPath.empty())
				memcpy(buffer, skyboxPath.c_str(), sizeof(buffer));
			else
				memset(buffer, 0, sizeof(buffer));

			Gui::InputText("##Skybox Source", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

			// Accept a Skybox Directory from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path skyboxDirectoryPath = std::filesystem::path(g_AssetPath) / path;

					// Make sure we are recieving an actual directory otherwise we will have trouble loading it
					if (std::filesystem::is_directory(skyboxDirectoryPath))
						skybox->SetDirectoryPath(skyboxDirectoryPath.string());
					else
						SP_CORE_WARN("Could not load skybox, must be a directory - {}", skyboxDirectoryPath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			Gui::SameLine();
			Gui::Text("Source");
		});

		DrawComponent<LightSourceComponent>("Light Source", entity, [](auto& component)
		{
			static const char* lightTypes[] = { "Directional", "Point", "Spot" };
			const char* currentLightType = lightTypes[(uint32_t)component.Type];

			if (Gui::BeginCombo("Light Type", currentLightType))
			{
				uint32_t arraySize = SP_ARRAYCOUNT(lightTypes);

				for (uint32_t i = 0; i < arraySize; i++)
				{
					bool isSelected = strcmp(currentLightType, lightTypes[i]) == 0;
					if (Gui::Selectable(lightTypes[i], isSelected))
					{
						currentLightType = lightTypes[i];
						component.Type = static_cast<LightSourceComponent::LightType>(i);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();

					if (i != arraySize - 1)
						Gui::Separator();
				}

				Gui::EndCombo();
			}

			SharedRef<LightSource> lightSource = component.Source;

			switch (component.Type)
			{
				case LightSourceComponent::LightType::Directional:
				{
					Math::vec3 direction = lightSource->GetDirection();
					if (Gui::DragFloat3("Direction", Math::ValuePtr(direction), 0.01f, 0.0f, 0.0f, "%.2f"))
						lightSource->SetDirection(direction);
					break;
				}
				case LightSourceComponent::LightType::Point:
				{
					Math::vec2 attenuation = lightSource->GetAttenuation();

					float range = attenuation.x * -4.0f;
					if (Gui::DragFloat("Range", &range, 0.01f, -0.75f, 0.0f, "%.2f"))
						lightSource->SetAttenuation({ range / -4.0f, attenuation.y });

					float intensity = attenuation.y * -4.0f;
					if (Gui::DragFloat("Intensity", &intensity, 0.001f, -0.75f, 0.0f, "%.3f"))
						lightSource->SetAttenuation({ attenuation.x, intensity / -4.0f });

					break;
				}
				case LightSourceComponent::LightType::Spot:
				{
					Math::vec3 direction = lightSource->GetDirection();
					if (Gui::DragFloat3("Direction", Math::ValuePtr(direction), 0.01f, 0.0f, 0.0f, "%.2f"))
						lightSource->SetDirection(direction);

					Math::vec2 attenuation = lightSource->GetAttenuation();

					float range = attenuation.x * -4.0f;
					if (Gui::DragFloat("Range", &range, 0.01f, -0.75f, 0.0f, "%.2f"))
						lightSource->SetAttenuation({ range / -4.0f, attenuation.y });

					float intensity = attenuation.y * -4.0f;
					if (Gui::DragFloat("Intensity", &intensity, 0.001f, -0.75f, 0.0f, "%.3f"))
						lightSource->SetAttenuation({ attenuation.x, intensity / -4.0f });

					float cutoff = lightSource->GetCutOff();
					if (Gui::DragFloat("CutOff", &cutoff))
						lightSource->SetCutOff(cutoff);

					float outerCutoff = lightSource->GetOuterCutOff();
					if (Gui::DragFloat("Outer CutOff", &outerCutoff))
						lightSource->SetOuterCutOff(outerCutoff);

					break;
				}
			}

			Math::vec3 ambient = lightSource->GetAmbient();
			if (Gui::ColorEdit3("Ambient", Math::ValuePtr(ambient)))
				lightSource->SetAmbient(ambient);

			Math::vec3 diffuse = lightSource->GetDiffuse();
			if (Gui::DragFloat3("Diffuse", Math::ValuePtr(diffuse), 0.01f, 0.01f, 1.0f, "%.2f"))
				lightSource->SetDiffuse(diffuse);

			Math::vec3 specular = lightSource->GetSpecular();
			if (Gui::DragFloat3("Specular", Math::ValuePtr(specular), 0.01f, 0.01f, 1.0f, "%.2f"))
				lightSource->SetSpecular(specular);

			Math::vec3 color = lightSource->GetColor();
			if (Gui::ColorEdit3("Color", Math::ValuePtr(color)))
				lightSource->SetColor(color);
		});

		static SharedRef<Texture2D> checkerboardIcon = Texture2D::Create("Resources/Icons/Inspector/Checkerboard.png");

		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [&](auto& component)
		{
			static const char* meshTypes[] = { "Cube", "Sphere", "Capsule", "Cone", "Cylinder", "Plane", "Torus", "Custom" };
			const char* currentMeshType = meshTypes[(uint32_t)component.Type];

			static const char* defaultMeshSourcePaths[] = {
				"Resources/Meshes/Default/Cube.obj",
				"Resources/Meshes/Default/Sphere.obj",
				"Resources/Meshes/Default/Capsule.obj",
				"Resources/Meshes/Default/Cone.obj",
				"Resources/Meshes/Default/Cylinder.obj",
				"Resources/Meshes/Default/Plane.obj",
				"Resources/Meshes/Default/Torus.obj",
			};

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));

			if (Gui::BeginCombo("Mesh Type", currentMeshType))
			{
				uint32_t arraySize = SP_ARRAYCOUNT(meshTypes);

				for (uint32_t i = 0; i < arraySize; i++)
				{
					bool isSelected = strcmp(currentMeshType, meshTypes[i]) == 0;
					if (Gui::Selectable(meshTypes[i], isSelected))
					{
						currentMeshType = meshTypes[i];
						component.Type = static_cast<MeshRendererComponent::MeshType>(i);

						if (component.Type != MeshRendererComponent::MeshType::Custom)
							component.Mesh = Model::Create(std::string(defaultMeshSourcePaths[i]), entity);
						else
							component.Mesh = Model::Create(std::string(buffer), entity);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();

					if (i != arraySize - 1)
						Gui::Separator();
				}

				Gui::EndCombo();
			}
			
			if (component.Mesh)
				memcpy(buffer, component.Mesh->GetPath().c_str(), sizeof(buffer));

			Gui::InputText("##Mesh Source", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

			// Accept a Model File from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path modelFilepath = std::filesystem::path(g_AssetPath) / path;

					// Make sure we are recieving an actual obj file otherwise we will have trouble opening it
					if (modelFilepath.filename().extension() == ".obj")
					{
						component.Mesh = Model::Create(modelFilepath.string(), entity);
						component.Type = MeshRendererComponent::MeshType::Custom;
					}
					else
						SP_CORE_WARN("Could not load model file, not a '.obj' - {}", modelFilepath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			Gui::SameLine();
			Gui::Text("Mesh Source");

			auto textureSize = ImVec2{ 64, 64 };

			Gui::Text("Texture");
			Gui::SameLine();
			Gui::SetCursorPosX(Gui::GetContentRegionAvail().x);

			SharedRef<Material> material = component.Mesh->GetMaterial();
			Math::vec3 ambient = material->GetAmbient();

			if (component.Texture)
			{
				ImVec4 tintColor = { ambient.r, ambient.g, ambient.b, 1.0f };

				if (Gui::ImageButton((void*)component.Texture->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }, -1, { 0, 0, 0, 0 }, tintColor))
					component.Texture = nullptr;
				else if (Gui::IsItemHovered())
				{
					Gui::BeginTooltip();
					Gui::Text(component.Texture->GetPath().c_str());
					Gui::EndTooltip();
				}
			}
			else
			{
				// Show the default checkerboard texture
				Gui::ImageButton((void*)checkerboardIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
			}

			// Accept a Texture from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;

					// Make sure we are recieving an actual texture otherwise we will have trouble opening it
					if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
					{
						SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

						if (texture->IsLoaded())
							component.Texture = texture;
						else
							SP_WARN("Could not load texture {}", texturePath.filename().string());
					}
					else
						SP_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			if (Gui::TreeNodeEx("Material", treeNodeFlags))
			{
				Gui::Unindent();

				if (Gui::ColorEdit3("Ambient", Math::ValuePtr(ambient)))
					material->SetAmbient(ambient);

				Gui::Text("Diffuse");
				Gui::SameLine();
				Gui::SetCursorPosX(Gui::GetContentRegionAvail().x);

				SharedRef<Texture2D> diffuseMap = material->GetDiffuseMap();

				if (diffuseMap)
				{
					ImVec4 tintColor = { ambient.r, ambient.g, ambient.b, 1.0f };

					if (Gui::ImageButton((void*)diffuseMap->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }, -1, { 0, 0, 0, 0 }, tintColor))
						material->SetDiffuseMap(nullptr);
					else if (Gui::IsItemHovered())
					{
						Gui::BeginTooltip();
						Gui::Text(diffuseMap->GetPath().c_str());
						Gui::EndTooltip();
					}
				}
				else
				{
					// Show the default checkerboard texture
					Gui::ImageButton((void*)checkerboardIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
				}

				// Accept a Diffuse map from the content browser
				if (Gui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;

						// Make sure we are recieving an actual texture otherwise we will have trouble opening it
						if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
						{
							SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

							if (texture->IsLoaded())
								material->SetDiffuseMap(texture);
							else
								SP_WARN("Could not load texture {}", texturePath.filename().string());
						}
						else
							SP_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
					}
					Gui::EndDragDropTarget();
				}

				Gui::Spacing();
				
				Gui::Text("Specular");
				Gui::SameLine();
				Gui::SetCursorPosX(Gui::GetContentRegionAvail().x);

				SharedRef<Texture2D> specularMap = material->GetSpecularMap();

				if (specularMap)
				{
					ImVec4 tintColor = { ambient.r, ambient.g, ambient.b, 1.0f };

					if (Gui::ImageButton((void*)specularMap->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }, -1, { 0, 0, 0, 0 }, tintColor))
						material->SetSpecularMap(nullptr);
					else if (Gui::IsItemHovered())
					{
						Gui::BeginTooltip();
						Gui::Text(specularMap->GetPath().c_str());
						Gui::EndTooltip();
					}
				}
				else
				{
					// Show the default checkerboard texture
					Gui::ImageButton((void*)checkerboardIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
				}

				// Accept a Specular map from the content browser
				if (Gui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;

						// Make sure we are recieving an actual texture otherwise we will have trouble opening it
						if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
						{
							SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

							if (texture->IsLoaded())
								material->SetSpecularMap(texture);
							else
								SP_WARN("Could not load texture {}", texturePath.filename().string());
						}
						else
							SP_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
					}
					Gui::EndDragDropTarget();
				}

				float shininess = material->GetShininess();
				if (Gui::DragFloat("Shininess", &shininess, 2.0f, 2.0f, 256.0f, "%.2f"))
					material->SetShininess(shininess);

				Gui::DragFloat2("UV", Math::ValuePtr(component.Scale), 0.05f, 0.0f, 0.0f, "%.2f");

				Gui::Checkbox("Reflective", &component.Reflective);
				Gui::Checkbox("Refractive", &component.Refractive);

				Gui::Indent();
				Gui::TreePop();
			}
		});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [&](auto& component)
		{
			Gui::ColorEdit4("Color", Math::ValuePtr(component.SpriteColor));

			auto textureSize = ImVec2{ 64, 64 };

			Gui::Text("Texture");
			Gui::SameLine();
			Gui::SetCursorPosX(Gui::GetContentRegionAvail().x);

			if (component.Texture)
			{
				ImVec4 tintColor = { component.SpriteColor.r, component.SpriteColor.g, component.SpriteColor.b, component.SpriteColor.a };

				if (Gui::ImageButton((void*)component.Texture->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }, -1, { 0, 0, 0, 0 }, tintColor))
					component.Texture = nullptr;
				else if (Gui::IsItemHovered())
				{
					Gui::BeginTooltip();
					Gui::Text(component.Texture->GetPath().c_str());
					Gui::EndTooltip();
				}
			}
			else
			{
				// Show the default checkerboard texture
				if (Gui::ImageButton((void*)checkerboardIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }))
					component.Texture = nullptr;
			}

			// Accept a Texture from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;

					// Make sure we are recieving an actual texture otherwise we will have trouble opening it
					if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
					{
						SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

						if (texture->IsLoaded())
							component.Texture = texture;
						else
							SP_CORE_WARN("Could not load texture {}", texturePath.filename().string());
					}
					else
						SP_CORE_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			Gui::DragFloat2("UV", Math::ValuePtr(component.Scale), 0.05f);
		});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
		{
			Gui::ColorEdit4("Color", Math::ValuePtr(component.Color));
			Gui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
			Gui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
		});

		DrawComponent<ParticleEmitterComponent>("Particle Emitter", entity, [](auto& component)
		{
			SharedRef<ParticleEmitter> particleEmitter = component.Emitter;

			ParticleEmitterProperties& emitterProperties = particleEmitter->GetProperties();

			Gui::DragFloat3("Velocity", Math::ValuePtr(emitterProperties.Velocity), 0.25f, 0.1f, 0.0f, "%.2f");
			Gui::DragFloat3("Velocity Variation", Math::ValuePtr(emitterProperties.VelocityVariation), 0.25f, 0.1f, 0.0f, "%.2f");
			Gui::DragFloat2("Size Start", Math::ValuePtr(emitterProperties.SizeBegin), 0.25f, 0.1f, 0.0f, "%.2f");
			Gui::DragFloat2("Size End", Math::ValuePtr(emitterProperties.SizeEnd), 0.25f, 0.1f, 0.0f, "%.2f");
			Gui::DragFloat2("Size Variation", Math::ValuePtr(emitterProperties.SizeVariation), 0.25f, 0.1f, 0.0f, "%.2f");
			Gui::ColorEdit4("Color Start", Math::ValuePtr(emitterProperties.ColorBegin));
			Gui::ColorEdit4("Color End", Math::ValuePtr(emitterProperties.ColorEnd));
			Gui::DragFloat("Rotation", &emitterProperties.Rotation, 0.1f, 0.0f, 0.0f, "%.2f");
			Gui::DragFloat("Lifetime", &emitterProperties.LifeTime, 0.25f, 0.1f, 0.0f, "%.2f");

			if (Gui::Button("Start"))
				particleEmitter->Start();
			Gui::SameLine();

			if (Gui::Button("Stop"))
				particleEmitter->Stop();
		});

		DrawComponent<AudioSourceComponent>("Audio Source", entity, [](auto& component)
		{
			if (component.Source)
			{
				Gui::BeginDisabled(!component.Source->IsPlaying());
				Gui::ProgressBar(component.Source->GetAmountComplete());
				Gui::EndDisabled();
			}

			char buffer[256];
			
			if (component.Source)
				memcpy(buffer, component.Source->GetPath().c_str(), sizeof(buffer));
			else
				memset(buffer, 0, sizeof(buffer));

			Gui::InputText("Source", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

			// Accept a Audio File from the content browser
			if (Gui::BeginDragDropTarget())
			{	
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path audioSourcePath = std::filesystem::path(g_AssetPath) / path;

					// Make sure we are recieving an actual audio file otherwise we will have trouble opening it
					if (audioSourcePath.filename().extension() == ".wav" || audioSourcePath.filename().extension() == ".mp3")
					{
						// If there is another file playing we need to stop it
						if (component.Source)
							component.Source->Stop();

						component.Source = CreateShared<AudioSource>(audioSourcePath.string());
					}
					else
						SP_CORE_WARN("Could not load audio file, not a '.wav' or '.mp3' - {}", audioSourcePath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			Gui::Spacing();

			Gui::BeginDisabled(component.Source == nullptr);

			if (Gui::Button("Play"))
				component.Source->Play();

			Gui::SameLine();

			Gui::BeginDisabled(component.Source != nullptr && !component.Source->IsPlaying());

			if (Gui::Button("Pause"))
				component.Source->Pause();

			Gui::SameLine();
			
			if (Gui::Button("Restart"))
				component.Source->Restart();

			Gui::EndDisabled();

			Gui::SameLine();

			if (Gui::Button("Stop"))
				component.Source->Stop();

			Gui::Spacing();

			if (component.Source != nullptr)
			{
				AudioSource::SoundProperties& props = component.Source->GetProperties();

				if (Gui::DragFloat("Pitch", &props.Pitch, 0.01f, 0.2f, 2.0f))
					component.Source->SetPitch(props.Pitch);

				if (Gui::DragFloat("Volume", &props.Volume, 0.1f))
					component.Source->SetVolume(props.Volume);

				if (Gui::Checkbox("Loop", &props.Loop))
					component.Source->SetLoop(props.Loop);

				if (Gui::Checkbox("Spacialized", &props.Spacialized))
					component.Source->SetSpacialized(props.Spacialized);

				if (props.Spacialized && Gui::TreeNodeEx("Spatialization", treeNodeFlags))
				{
					Gui::Spacing();
					Gui::Unindent();

					DrawVec3Controls("Position", props.Position, 0.0f, 100.0f, [&]()
					{
						component.Source->SetPosition(props.Position);
					});

					DrawVec3Controls("Direction", props.Direction, 0.0f, 100.0f, [&]()
					{
						component.Source->SetDirection(props.Direction);
					});

					DrawVec3Controls("Veloctiy", props.Veloctiy, 0.0f, 100.0f, [&]()
					{
						component.Source->SetVelocity(props.Veloctiy);
					});

					Gui::Spacing();

					if (Gui::TreeNodeEx("Cone", treeNodeFlags))
					{
						Gui::Unindent();
						float innerAngle = Math::Rad2Deg(props.Cone.InnerAngle);
						if (Gui::DragFloat("Inner Angle", &innerAngle, 0.5f))
						{
							props.Cone.InnerAngle = Math::Deg2Rad(innerAngle);
							component.Source->SetCone(props.Cone);
						}
						float outerAngle = Math::Rad2Deg(props.Cone.OuterAngle);
						if (Gui::DragFloat("Outer Angle", &outerAngle, 0.5f))
						{
							props.Cone.OuterAngle = Math::Deg2Rad(outerAngle);
							component.Source->SetCone(props.Cone);
						}
						float outerGain = Math::Rad2Deg(props.Cone.OuterGain);
						if (Gui::DragFloat("Outer Gain", &outerGain, 0.5f))
						{
							props.Cone.OuterGain = Math::Deg2Rad(outerGain);
							component.Source->SetCone(props.Cone);
						}

						Gui::Spacing();

						if (Gui::DragFloat("Min Distance", &props.MinDistance, 0.1f))
							component.Source->SetMinDistance(props.MinDistance);

						if (Gui::DragFloat("Max Distance", &props.MaxDistance, 0.1f))
							component.Source->SetMaxDistance(props.MaxDistance);

						if (Gui::DragFloat("Doppler Factor", &props.DopplerFactor, 0.1f))
							component.Source->SetDopplerFactor(props.DopplerFactor);

						Gui::Indent();
						Gui::TreePop();
					}

					Gui::Indent();
					Gui::TreePop();
				}
			}

			Gui::EndDisabled();
		});

		DrawComponent<AudioListenerComponent>("Audio Listener", entity, [](auto& component)
		{
			AudioListener::ListenerProperties& props = component.Listener->GetProperties();

			DrawVec3Controls("Position", props.Position, 0.0f, 100.0f, [&]()
			{
				component.Listener->SetPosition(props.Position);
			});

			DrawVec3Controls("Direction", props.Direction, 0.0f, 100.0f, [&]()
			{
				component.Listener->SetDirection(props.Direction);
			});

			DrawVec3Controls("Veloctiy", props.Veloctiy, 0.0f, 100.0f, [&]()
			{
				component.Listener->SetVelocity(props.Veloctiy);
			});

			Gui::Spacing();

			if (Gui::TreeNodeEx("Cone", treeNodeFlags))
			{
				Gui::Unindent();

				float innerAngle = Math::Rad2Deg(props.Cone.InnerAngle);
				if (Gui::DragFloat("Inner Angle", &innerAngle, 0.5f))
				{
					props.Cone.InnerAngle = Math::Deg2Rad(innerAngle);
					component.Listener->SetCone(props.Cone);
				}
				float outerAngle = Math::Rad2Deg(props.Cone.OuterAngle);
				if (Gui::DragFloat("Outer Angle", &outerAngle, 0.5f))
				{
					props.Cone.OuterAngle = Math::Deg2Rad(outerAngle);
					component.Listener->SetCone(props.Cone);
				}
				float outerGain = Math::Rad2Deg(props.Cone.OuterGain);
				if (Gui::DragFloat("Outer Gain", &outerGain, 0.5f))
				{
					props.Cone.OuterGain = Math::Deg2Rad(outerGain);
					component.Listener->SetCone(props.Cone);
				}

				Gui::Indent();
				Gui::TreePop();
			}
		});

		DrawComponent<RigidBody2DComponent>("RigidBody 2D", entity, [](auto& component)
		{
			const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
			const char* currentBodyType = bodyTypes[(uint32_t)component.Type];

			if (Gui::BeginCombo("Body Type", currentBodyType))
			{
				uint32_t arraySize = SP_ARRAYCOUNT(bodyTypes);

				for (uint32_t i = 0; i < arraySize; i++)
				{
					bool isSelected = strcmp(currentBodyType, bodyTypes[i]) == 0;
					if (Gui::Selectable(bodyTypes[i], isSelected))
					{
						currentBodyType = bodyTypes[i];
						component.Type = static_cast<RigidBody2DComponent::BodyType>(i);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();

					if (i != arraySize - 1)
						Gui::Separator();
				}

				Gui::EndCombo();
			}

			Gui::Checkbox("Fixed Rotation", &component.FixedRotation);
		});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
		{
			Gui::DragFloat2("Offset", Math::ValuePtr(component.Offset), 0.01f);
			Gui::DragFloat2("Size", Math::ValuePtr(component.Size), 0.01f);
			Gui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Threshold", &component.RestitutionThreshold, 0.1f, 0.0f);
		});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
		{
			Gui::DragFloat2("Offset", Math::ValuePtr(component.Offset), 0.01f);
			Gui::DragFloat("Radius", &component.Radius, 0.01, 0.01f);
			Gui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Threshold", &component.RestitutionThreshold, 0.1f, 0.0f);
		});

		DrawComponent<ScriptComponent>("C# Script", entity, [&](auto& component)
		{
			std::vector<std::string> entityClassNameStrings;
			bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);
			bool allEntityClassNamesCollected = false;

			// Retrieve all entity class names to display them in combo box
			if (!allEntityClassNamesCollected)
			{
				auto entityClasses = ScriptEngine::GetClasses();

				for (auto& [className, entityScriptClass] : entityClasses)
					entityClassNameStrings.push_back(className);

				allEntityClassNamesCollected = true;
			}

			const char* currentClassName = component.ClassName.c_str();

			// Display available entity classes to choose from
			if (allEntityClassNamesCollected && Gui::BeginCombo("Class", currentClassName))
			{
				bool isSearching = Gui::InputTextWithHint("##ClassNameSearch", "Search", m_EntityClassNameInputTextFilter.InputBuf, IM_ARRAYSIZE(m_EntityClassNameInputTextFilter.InputBuf));
				if (isSearching)
					m_EntityClassNameInputTextFilter.Build();

				Gui::Spacing();
				Gui::Separator();

				for (uint32_t i = 0; i < entityClassNameStrings.size(); i++)
				{
					const char* currentEntityClassNameString = entityClassNameStrings[i].c_str();
					bool isSelected = strcmp(currentClassName, currentEntityClassNameString) == 0;

					if (!m_EntityClassNameInputTextFilter.PassFilter(currentEntityClassNameString))
						continue;

					if (Gui::Selectable(currentEntityClassNameString, isSelected))
					{
						// If we select a class we need to set the components class name here
						currentClassName = currentEntityClassNameString;
						component.ClassName = std::string(currentClassName);

						// Reset the search bar once a class is chosen
						memset(m_EntityClassNameInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_EntityClassNameInputTextFilter.InputBuf));
						m_EntityClassNameInputTextFilter.Build();
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();

					if (i != entityClassNameStrings.size() - 1)
						Gui::Separator();
				}

				Gui::EndCombo();
			}

			// Fields
			bool sceneRunning = m_ContextScene->IsRunning();

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
							if (Gui::DragFloat(name.c_str(), &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Double)
						{
							double data = scriptInstance->GetFieldValue<double>(name);
							if (Gui::DragScalar(name.c_str(), ImGuiDataType_Double, &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Vector2)
						{
							Math::vec2 data = scriptInstance->GetFieldValue<Math::vec2>(name);
							if (Gui::DragFloat2(name.c_str(), Math::ValuePtr(data)))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Vector3)
						{
							Math::vec3 data = scriptInstance->GetFieldValue<Math::vec3>(name);
							if (Gui::DragFloat3(name.c_str(), Math::ValuePtr(data)))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Vector4)
						{
							Math::vec4 data = scriptInstance->GetFieldValue<Math::vec4>(name);
							if (Gui::DragFloat4(name.c_str(), Math::ValuePtr(data)))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Bool)
						{
							bool data = scriptInstance->GetFieldValue<bool>(name);
							if (Gui::Checkbox(name.c_str(), &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Char)
						{
							char data = scriptInstance->GetFieldValue<char>(name);
							if (Gui::InputScalar(name.c_str(), ImGuiDataType_S8, &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Short)
						{
							short data = scriptInstance->GetFieldValue<short>(name);
							if (Gui::DragScalar(name.c_str(), ImGuiDataType_S16, &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Int)
						{
							int data = scriptInstance->GetFieldValue<int>(name);
							if (Gui::DragInt(name.c_str(), &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Long)
						{
							long long data = scriptInstance->GetFieldValue<long long>(name);
							if (Gui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Byte)
						{
							unsigned char data = scriptInstance->GetFieldValue<unsigned char>(name);
							if (Gui::DragScalar(name.c_str(), ImGuiDataType_U8, &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::UShort)
						{
							unsigned short data = scriptInstance->GetFieldValue<unsigned short>(name);
							if (Gui::DragScalar(name.c_str(), ImGuiDataType_U16, &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::UInt)
						{
							unsigned int data = scriptInstance->GetFieldValue<unsigned int>(name);
							if (Gui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::ULong)
						{
							unsigned long long data = scriptInstance->GetFieldValue<unsigned long long>(name);
							if (Gui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
								scriptInstance->SetFieldValue(name, data);
						}
						if (field.Type == ScriptFieldType::Entity)
						{
							uint64_t data = scriptInstance->GetFieldValue<uint64_t>(name);
							if (Gui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
								scriptInstance->SetFieldValue(name, data);
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

					auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
					for (const auto& [name, field] : fields)
					{
						auto it = entityFields.find(name);

						// Field has been set in editor
						if (it != entityFields.end())
						{
							ScriptFieldInstance& scriptField = it->second;

							// Display controls to set it
							if (field.Type == ScriptFieldType::Float)
							{
								float data = scriptField.GetValue<float>();
								if (Gui::DragFloat(name.c_str(), &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Double)
							{
								double data = scriptField.GetValue<double>();
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_Double, &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Vector2)
							{
								Math::vec2 data = scriptField.GetValue<Math::vec2>();
								if (Gui::DragFloat2(name.c_str(), Math::ValuePtr(data)))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Vector3)
							{
								Math::vec3 data = scriptField.GetValue<Math::vec3>();
								if (Gui::DragFloat3(name.c_str(), Math::ValuePtr(data)))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Vector4)
							{
								Math::vec4 data = scriptField.GetValue<Math::vec4>();
								if (Gui::DragFloat4(name.c_str(), Math::ValuePtr(data)))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Bool)
							{
								bool data = scriptField.GetValue<bool>();
								if (Gui::Checkbox(name.c_str(), &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Char)
							{
								char data = scriptField.GetValue<char>();
								if (Gui::InputScalar(name.c_str(), ImGuiDataType_S8, &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Short)
							{
								short data = scriptField.GetValue<short>();
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_S16, &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Int)
							{
								int data = scriptField.GetValue<int>();
								if (Gui::DragInt(name.c_str(), &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Long)
							{
								long long data = scriptField.GetValue<long long>();
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Byte)
							{
								unsigned char data = scriptField.GetValue<unsigned char>();
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U8, &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::UShort)
							{
								unsigned short data = scriptField.GetValue<unsigned short>();
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U16, &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::UInt)
							{
								unsigned int data = scriptField.GetValue<unsigned int>();
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::ULong)
							{
								unsigned long long data = scriptField.GetValue<unsigned long long>();
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
									scriptField.SetValue(data);
							}
							if (field.Type == ScriptFieldType::Entity)
							{
								uint64_t data = scriptField.GetValue<uint64_t>();
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
									scriptField.SetValue(data);
							}
						}
						else
						{
							// Display controls to set it
							if (field.Type == ScriptFieldType::Float)
							{
								float data = 0.0f;
								if (Gui::DragFloat(name.c_str(), &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Double)
							{
								double data = 0.0f;
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_Double, &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Vector2)
							{
								Math::vec2 data = Math::vec2(0.0f);
								if (Gui::DragFloat2(name.c_str(), Math::ValuePtr(data)))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Vector3)
							{
								Math::vec3 data = Math::vec3(0.0f);
								if (Gui::DragFloat3(name.c_str(), Math::ValuePtr(data)))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Vector4)
							{
								Math::vec4 data = Math::vec4(0.0f);
								if (Gui::DragFloat4(name.c_str(), Math::ValuePtr(data)))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Bool)
							{
								bool data = false;
								if (Gui::Checkbox(name.c_str(), &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Char)
							{
								char data = 0;
								if (Gui::InputScalar(name.c_str(), ImGuiDataType_S8, &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Short)
							{
								short data = 0;
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_S16, &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Int)
							{
								int data = 0;
								if (Gui::DragInt(name.c_str(), &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Long)
							{
								long long data = 0;
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Byte)
							{
								unsigned char data = 0;
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U8, &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::UShort)
							{
								unsigned short data = 0;
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U16, &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::UInt)
							{
								unsigned int data = 0;
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::ULong)
							{
								unsigned long long data = 0;
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
							if (field.Type == ScriptFieldType::Entity)
							{
								uint64_t data = 0;
								if (Gui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
								{
									ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}

								// Accept an Entity from the scene hierarchy
								if (Gui::BeginDragDropTarget())
								{
									if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
									{
										UUID* entityUUID = (UUID*)payload->Data;
										
										if (Entity entity = m_ContextScene->GetEntityWithUUID(*entityUUID))
										{
											ScriptFieldInstance& fieldInstance = entityFields[name];
											fieldInstance.Field = field;
											fieldInstance.SetValue(*entityUUID);
										}
										else
											SP_WARN("Entity UUID {} was not found in the current scene.", *entityUUID);
									}
									Gui::EndDragDropTarget();
								}
							}
						}
					}
				}
			}
		});

		DrawComponent<NativeScriptComponent>("Native Script", entity, [](auto& component) {});
	}

}
