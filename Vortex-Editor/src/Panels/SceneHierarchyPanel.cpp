#include "SceneHierarchyPanel.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Core/Buffer.h"

#include <imgui_internal.h>

#include <codecvt>

namespace Vortex {

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

	void SceneHierarchyPanel::OnGuiRender(Entity hoveredEntity, const EditorCamera& editorCamera)
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

					if (entity)
					{
						// If the name lines up with the search box we can show it
						if (m_EntitySearchInputTextFilter.PassFilter(entity.GetName().c_str()) && entity.GetParentUUID() == 0)
							DrawEntityNode(entity, editorCamera);
					}
				});

				if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

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

	inline static Math::vec3 GetEditorCameraForwardPosition(const EditorCamera& editorCamera)
	{
		return editorCamera.GetPosition() + (editorCamera.GetForwardDirection() * 3.0f);
	}

	inline static void CreateModel(const std::string& name, Model::Default defaultMesh, Entity& entity, SharedRef<Scene> contextScene, const EditorCamera& editorCamera)
	{
		entity = contextScene->CreateEntity(name);
		MeshRendererComponent& meshRenderer = entity.AddComponent<MeshRendererComponent>();
		meshRenderer.Type = static_cast<MeshType>(defaultMesh);
		meshRenderer.Mesh = Model::Create(defaultMesh, entity.GetTransform(), (int)(entt::entity)entity);
		entity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
	}

	void SceneHierarchyPanel::DisplayCreateEntityMenu(const EditorCamera& editorCamera)
	{
		if (Gui::MenuItem("Create Empty"))
		{
			m_SelectedEntity = m_ContextScene->CreateEntity("Empty Entity");
			m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
		}
		Gui::Separator();

		if (Gui::BeginMenu("Create 3D"))
		{
			if (Gui::MenuItem("Cube"))
				CreateModel("Cube", Model::Default::Cube, m_SelectedEntity, m_ContextScene, editorCamera);
			Gui::Separator();

			if (Gui::MenuItem("Sphere"))
				CreateModel("Sphere", Model::Default::Sphere, m_SelectedEntity, m_ContextScene, editorCamera);
			Gui::Separator();

			if (Gui::MenuItem("Capsule"))
				CreateModel("Capsule", Model::Default::Capsule, m_SelectedEntity, m_ContextScene, editorCamera);
			Gui::Separator();

			if (Gui::MenuItem("Cone"))
				CreateModel("Cone", Model::Default::Cone, m_SelectedEntity, m_ContextScene, editorCamera);
			Gui::Separator();

			if (Gui::MenuItem("Cylinder"))
				CreateModel("Cylinder", Model::Default::Cylinder, m_SelectedEntity, m_ContextScene, editorCamera);
			Gui::Separator();

			if (Gui::MenuItem("Plane"))
				CreateModel("Plane", Model::Default::Plane, m_SelectedEntity, m_ContextScene, editorCamera);
			Gui::Separator();

			if (Gui::MenuItem("Torus"))
				CreateModel("Torus", Model::Default::Torus, m_SelectedEntity, m_ContextScene, editorCamera);

			Gui::EndMenu();
		}
		Gui::Separator();

		if (Gui::BeginMenu("Create 2D"))
		{
			if (Gui::MenuItem("Quad"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Quad");
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
				m_SelectedEntity.AddComponent<SpriteRendererComponent>();
			}

			Gui::Separator();

			if (Gui::MenuItem("Circle"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Circle");
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
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
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}
			Gui::Separator();

			if (Gui::MenuItem("Orthographic"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Camera");
				auto& cameraComponent = m_SelectedEntity.AddComponent<CameraComponent>();
				cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
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
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}
			Gui::Separator();

			if (Gui::MenuItem("Point"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Point Light");
				m_SelectedEntity.AddComponent<LightSourceComponent>().Type = LightSourceComponent::LightType::Point;
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}
			Gui::Separator();

			if (Gui::MenuItem("Spot"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Spot Light");
				m_SelectedEntity.AddComponent<LightSourceComponent>().Type = LightSourceComponent::LightType::Spot;
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}

			Gui::EndMenu();
		}
		Gui::Separator();

		if (Gui::BeginMenu("Physics"))
		{
			if (Gui::MenuItem("Box Collider"))
			{
				CreateModel("Box Collider", Model::Default::Cube, m_SelectedEntity, m_ContextScene, editorCamera);
				m_SelectedEntity.AddComponent<RigidBodyComponent>();
				m_SelectedEntity.AddComponent<BoxColliderComponent>();
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}

			Gui::Separator();

			if (Gui::MenuItem("Sphere Collider"))
			{
				CreateModel("Sphere Collider", Model::Default::Sphere, m_SelectedEntity, m_ContextScene, editorCamera);
				m_SelectedEntity.AddComponent<RigidBodyComponent>();
				m_SelectedEntity.AddComponent<SphereColliderComponent>();
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}

			Gui::Separator();

			if (Gui::MenuItem("Capsule Collider"))
			{
				CreateModel("Capsule Collider", Model::Default::Capsule, m_SelectedEntity, m_ContextScene, editorCamera);
				m_SelectedEntity.AddComponent<RigidBodyComponent>();
				m_SelectedEntity.AddComponent<CapsuleColliderComponent>();
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}

			Gui::Separator();

			if (Gui::MenuItem("Static Mesh Collider"))
			{
				CreateModel("Static Mesh Collider", Model::Default::Cube, m_SelectedEntity, m_ContextScene, editorCamera);
				m_SelectedEntity.AddComponent<RigidBodyComponent>();
				m_SelectedEntity.AddComponent<StaticMeshColliderComponent>();
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}

			Gui::Separator();

			if (Gui::MenuItem("Box Collider 2D"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Box Collider2D");
				m_SelectedEntity.AddComponent<SpriteRendererComponent>();
				m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				m_SelectedEntity.AddComponent<BoxCollider2DComponent>();
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}

			Gui::Separator();

			if (Gui::MenuItem("Circle Collider 2D"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Circle Collider2D");
				m_SelectedEntity.AddComponent<CircleRendererComponent>();
				m_SelectedEntity.AddComponent<RigidBody2DComponent>();
				m_SelectedEntity.AddComponent<CircleCollider2DComponent>();
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
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
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}

			Gui::Separator();

			if (Gui::MenuItem("Listener Entity"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Audio Listener");
				m_SelectedEntity.AddComponent<AudioListenerComponent>();
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
			}

			Gui::EndMenu();
		}
		Gui::Separator();

		if (Gui::BeginMenu("UI"))
		{
			if (Gui::MenuItem("Text"))
			{
				m_SelectedEntity = m_ContextScene->CreateEntity("Text");
				m_SelectedEntity.AddComponent<TextMeshComponent>();
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
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
				m_SelectedEntity.GetTransform().Translation = GetEditorCameraForwardPosition(editorCamera);
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

			if (hoveredEntity && m_ContextScene)
			{
				if (hoveredEntity.HasComponent<TagComponent>())
				{
					const auto& tag = hoveredEntity.GetComponent<TagComponent>().Tag;

					if (!tag.empty())
						name = tag.c_str();
				}
			}

			Gui::SetCursorPosX(10.0f);
			Gui::Text("Hovered Entity: %s", name);
		}

		Gui::End();
	}

	void SceneHierarchyPanel::DisplayAddMarkerPopup(TagComponent& tagComponent)
	{
		Gui::Spacing();
		Gui::TextCentered("New Marker", 5.0f);
		Gui::Separator();
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

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, const EditorCamera& editorCamera)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (entity.Children().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool isPrefab = entity.HasComponent<PrefabComponent>();
		if (isPrefab)
			Gui::PushStyleColor(ImGuiCol_Text, ImVec4(0.32f, 0.7f, 0.87f, 1.0f));

		const bool opened = Gui::TreeNodeEx((void*)(uint32_t)entity, flags, tag.c_str());

		if (isPrefab)
			Gui::PopStyleColor();

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

			if (Gui::MenuItem("Add Empty Child"))
			{
				Entity childEntity = m_ContextScene->CreateEntity("Empty Entity");
				m_ContextScene->ParentEntity(childEntity, entity);
				// Set child to origin of parent
				childEntity.GetTransform().Translation = Math::vec3(0.0f);
				SetSelectedEntity(childEntity);
				Gui::CloseCurrentPopup();
			}
			Gui::Separator();

			if (Gui::MenuItem("Unparent Entity"))
			{
				m_ContextScene->UnparentEntity(entity);
				Gui::CloseCurrentPopup();
			}
			Gui::Separator();

			if (Gui::MenuItem("Duplicate Entity", "Ctrl+D"))
			{
				m_ContextScene->DuplicateEntity(entity);
				Gui::CloseCurrentPopup();
			}
			Gui::Separator();

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
			for (auto& child : entity.Children())
			{
				Entity childEntity = m_ContextScene->TryGetEntityWithUUID(child);
				if (childEntity)
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
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiCallback, std::function<void(const TComponent&)> copyCallback = nullptr, std::function<void(TComponent&)> pasteCallback = nullptr, bool removeable = true)
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
					if (copyCallback)
						copyCallback(component);

					Gui::CloseCurrentPopup();
				}
				Gui::Separator();
				if (Gui::MenuItem("Paste Component"))
				{
					if (pasteCallback)
						pasteCallback(component);

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

			bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
			bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

			if (Gui::Button("Add Component") || (Input::IsKeyPressed(Key::A) && controlPressed && shiftPressed && Gui::IsWindowHovered()))
			{
				Gui::OpenPopup("AddComponent");

				// We should reset the search bar here
				memset(m_ComponentSearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
				m_ComponentSearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed
			}

			bool active = tagComponent.IsActive;
			if (Gui::Checkbox("Active", &active))
				tagComponent.IsActive = active;

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
							Gui::Separator();
						else // The last marker in the markers vector
						{
							Gui::Separator();

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
		if (Gui::BeginPopup("AddComponent"))
		{
			// Search Bar + Filtering
			bool isSearching = Gui::InputTextWithHint("##ComponentSearch", "Search", m_ComponentSearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_ComponentSearchInputTextFilter.InputBuf));
			if (isSearching)
				m_ComponentSearchInputTextFilter.Build();

			Gui::Spacing();
			Gui::Separator();

			if (const char* name = "Camera"; m_ComponentSearchInputTextFilter.PassFilter(name))
				DisplayAddComponentPopup<CameraComponent>(name);
			if (const char* componentName = "Skybox"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<SkyboxComponent>(componentName);
			if (const char* componentName = "Light Source"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<LightSourceComponent>(componentName);
			if (const char* componentName = "Mesh Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<MeshRendererComponent>(componentName);
			if (const char* componentName = "Sprite Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<SpriteRendererComponent>(componentName);
			if (const char* componentName = "Circle Renderer"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<CircleRendererComponent>(componentName);
			if (const char* componentName = "Particle Emitter"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<ParticleEmitterComponent>(componentName);
			if (const char* componentName = "Text Mesh"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<TextMeshComponent>(componentName);

			if (const char* componentName = "Audio Source"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<AudioSourceComponent>(componentName);
			if (const char* componentName = "Audio Listener"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<AudioListenerComponent>(componentName);

			if (const char* componentName = "RigidBody"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<RigidBodyComponent>(componentName);
			if (const char* componentName = "Character Controller"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<CharacterControllerComponent>(componentName);
			if (const char* componentName = "Physics Material"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<PhysicsMaterialComponent>(componentName);
			if (const char* componentName = "Box Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<BoxColliderComponent>(componentName);
			if (const char* componentName = "Sphere Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<SphereColliderComponent>(componentName);
			if (const char* componentName = "Capsule Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<CapsuleColliderComponent>(componentName);
			if (const char* componentName = "Static Mesh Collider"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<StaticMeshColliderComponent>(componentName);

			if (const char* componentName = "RigidBody 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<RigidBody2DComponent>(componentName);
			if (const char* componentName = "Box Collider 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<BoxCollider2DComponent>(componentName);
			if (const char* componentName = "Circle Collider 2D"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<CircleCollider2DComponent>(componentName);

			if (const char* componentName = "Nav Mesh Agent"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<NavMeshAgentComponent>(componentName);

			if (const char* componentName = "Script"; m_ComponentSearchInputTextFilter.PassFilter(componentName))
				DisplayAddComponentPopup<ScriptComponent>(componentName, true);

			Gui::EndPopup();
		}

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
		{
			DrawVec3Controls("Translation", component.Translation);
			Math::vec3 rotation = Math::Rad2Deg(component.GetRotationEuler());
			DrawVec3Controls("Rotation", rotation, 0.0f, 100.0f, [&]()
			{
				component.SetRotationEuler(Math::Deg2Rad(rotation));
			});
			DrawVec3Controls("Scale", component.Scale, 1.0f);
		},
		[=](const auto& component)
		{
			m_TransformToCopy = (TransformComponent)component;
		},
		[=](auto& component)
		{
			component = m_TransformToCopy;
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
				float perspectiveVerticalFOV = Math::Rad2Deg(camera.GetPerspectiveVerticalFOV());
				if (Gui::DragFloat("Field of View", &perspectiveVerticalFOV))
					camera.SetPerspectiveVerticalFOV(Math::Deg2Rad(perspectiveVerticalFOV));

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

			Gui::ColorEdit3("Clear Color", Math::ValuePtr(component.ClearColor));
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
					std::filesystem::path skyboxDirectoryPath = std::filesystem::path(path);

					// Make sure we are recieving an actual directory otherwise we will have trouble loading it
					if (std::filesystem::is_directory(skyboxDirectoryPath))
						skybox->SetDirectoryPath(skyboxDirectoryPath.string());
					else
						VX_CORE_WARN("Could not load skybox, must be a directory - {}", skyboxDirectoryPath.filename().string());
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

			if (Project::GetActive()->GetProperties().RendererProps.EnablePBRRenderer)
			{
				Math::vec3 radiance = lightSource->GetRadiance();
				if (Gui::ColorEdit3("Radiance", Math::ValuePtr(radiance)))
					lightSource->SetRadiance(radiance);
			}
			else
			{ // Blinn-Phong settings
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
			}
		});

		static SharedRef<Texture2D> checkerboardIcon = Texture2D::Create("Resources/Icons/Inspector/Checkerboard.png");

		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [&](auto& component)
		{
			static const char* meshTypes[] = { "Cube", "Sphere", "Capsule", "Cone", "Cylinder", "Plane", "Torus", "Custom" };
			const char* currentMeshType = meshTypes[(uint32_t)component.Type];

			char buffer[512];
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
						component.Type = static_cast<MeshType>(i);

						if (component.Type != MeshType::Custom)
							component.Mesh = Model::Create(static_cast<Model::Default>(i), entity.GetTransform(), (int)(entt::entity)entity);
						else
							component.Mesh = Model::Create(std::string(buffer), entity.GetTransform(), (int)(entt::entity)entity);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();

					if (i != arraySize - 1)
						Gui::Separator();
				}

				Gui::EndCombo();
			}
			
			if (component.Mesh)
			{
				memcpy(buffer, component.Mesh->GetPath().c_str(), sizeof(buffer));
				buffer[component.Mesh->GetPath().size()] = '\0';
			}

			Gui::Text("Mesh Source");
			Gui::SameLine();
			Gui::InputText("##Mesh Source", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

			// Accept a Model File from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path modelFilepath = std::filesystem::path(path);

					// Make sure we are recieving an actual obj file otherwise we will have trouble opening it
					if (modelFilepath.filename().extension() == ".obj" || modelFilepath.filename().extension() == ".fbx" || modelFilepath.filename().extension() == ".gltf")
					{
						component.Mesh = Model::Create(modelFilepath.string(), entity.GetTransform(), (int)(entt::entity)entity);
						component.Type = MeshType::Custom;
						component.Mesh->SetMaterial(MaterialInstance::Create());
					}
					else
						VX_CORE_WARN("Could not load model file - {}", modelFilepath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			if (component.Mesh)
			{
				SharedRef<Material> material = component.Mesh->GetMaterial();
				Math::vec3 albedo = material->GetAlbedo();
				auto textureSize = ImVec2{ 64, 64 };

				if (Gui::TreeNodeEx("Material", treeNodeFlags))
				{
					if (Gui::TreeNodeEx("Normal", treeNodeFlags))
					{
						Gui::Text("Map");
						Gui::SameLine();
						Gui::SetCursorPosX(Gui::GetContentRegionAvail().x);

						if (SharedRef<Texture2D> normalMap = material->GetNormalMap())
						{
							ImVec4 tintColor = { albedo.r, albedo.g, albedo.b, 1.0f };

							if (Gui::ImageButton((void*)normalMap->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }, -1, { 0, 0, 0, 0 }, tintColor))
								material->SetNormalMap(nullptr);
							else if (Gui::IsItemHovered())
							{
								Gui::BeginTooltip();
								Gui::Text(normalMap->GetPath().c_str());
								Gui::EndTooltip();
							}
						}
						else
							Gui::ImageButton((void*)checkerboardIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });

						// Accept a Normal map from the content browser
						if (Gui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
							{
								const wchar_t* path = (const wchar_t*)payload->Data;
								std::filesystem::path texturePath = std::filesystem::path(path);

								// Make sure we are recieving an actual texture otherwise we will have trouble opening it
								if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
								{
									SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

									if (texture->IsLoaded())
										material->SetNormalMap(texture);
									else
										VX_WARN("Could not load texture {}", texturePath.filename().string());
								}
								else
									VX_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
							}
							Gui::EndDragDropTarget();
						}

						Gui::TreePop();
					}

					if (Gui::TreeNodeEx("Albedo", treeNodeFlags))
					{
						Gui::Text("Map");
						Gui::SameLine();
						Gui::SetCursorPosX(Gui::GetContentRegionAvail().x);

						if (SharedRef<Texture2D> albedoMap = material->GetAlbedoMap())
						{
							ImVec4 tintColor = { albedo.r, albedo.g, albedo.b, 1.0f };

							if (Gui::ImageButton((void*)albedoMap->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }, -1, { 0, 0, 0, 0 }, tintColor))
								material->SetAlbedoMap(nullptr);
							else if (Gui::IsItemHovered())
							{
								Gui::BeginTooltip();
								Gui::Text(albedoMap->GetPath().c_str());
								Gui::EndTooltip();
							}
						}
						else
							Gui::ImageButton((void*)checkerboardIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });

						// Accept a Albedo map from the content browser
						if (Gui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
							{
								const wchar_t* path = (const wchar_t*)payload->Data;
								std::filesystem::path texturePath = std::filesystem::path(path);

								// Make sure we are recieving an actual texture otherwise we will have trouble opening it
								if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
								{
									SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

									if (texture->IsLoaded())
										material->SetAlbedoMap(texture);
									else
										VX_WARN("Could not load texture {}", texturePath.filename().string());
								}
								else
									VX_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
							}
							Gui::EndDragDropTarget();
						}

						if (!material->GetAlbedoMap())
						{
							Math::vec3 albedo = material->GetAlbedo();
							if (Gui::ColorEdit3("##Albedo", Math::ValuePtr(albedo)))
								material->SetAlbedo(albedo);
						}

						Gui::TreePop();
					}

					if (Gui::TreeNodeEx("Metallic", treeNodeFlags))
					{
						Gui::Text("Map");
						Gui::SameLine();
						Gui::SetCursorPosX(Gui::GetContentRegionAvail().x);

						if (SharedRef<Texture2D> metallicMap = material->GetMetallicMap())
						{
							ImVec4 tintColor = { albedo.r, albedo.g, albedo.b, 1.0f };

							if (Gui::ImageButton((void*)metallicMap->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }, -1, { 0, 0, 0, 0 }, tintColor))
								material->SetMetallicMap(nullptr);
							else if (Gui::IsItemHovered())
							{
								Gui::BeginTooltip();
								Gui::Text(metallicMap->GetPath().c_str());
								Gui::EndTooltip();
							}
						}
						else
							Gui::ImageButton((void*)checkerboardIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });

						// Accept a Metallic map from the content browser
						if (Gui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
							{
								const wchar_t* path = (const wchar_t*)payload->Data;
								std::filesystem::path texturePath = std::filesystem::path(path);

								// Make sure we are recieving an actual texture otherwise we will have trouble opening it
								if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
								{
									SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

									if (texture->IsLoaded())
										material->SetMetallicMap(texture);
									else
										VX_WARN("Could not load texture {}", texturePath.filename().string());
								}
								else
									VX_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
							}
							Gui::EndDragDropTarget();
						}

						if (!material->GetMetallicMap())
						{
							float metallic = material->GetMetallic();
							if (Gui::DragFloat("##Metallic", &metallic, 0.01f, 0.01f, 1.0f, "%.2f"))
								material->SetMetallic(metallic);
						}

						Gui::TreePop();
					}

					if (Gui::TreeNodeEx("Roughness", treeNodeFlags))
					{
						Gui::Text("Map");
						Gui::SameLine();
						Gui::SetCursorPosX(Gui::GetContentRegionAvail().x);

						if (SharedRef<Texture2D> roughnessMap = material->GetRoughnessMap())
						{
							ImVec4 tintColor = { albedo.r, albedo.g, albedo.b, 1.0f };

							if (Gui::ImageButton((void*)roughnessMap->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }, -1, { 0, 0, 0, 0 }, tintColor))
								material->SetRoughnessMap(nullptr);
							else if (Gui::IsItemHovered())
							{
								Gui::BeginTooltip();
								Gui::Text(roughnessMap->GetPath().c_str());
								Gui::EndTooltip();
							}
						}
						else
							Gui::ImageButton((void*)checkerboardIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });

						// Accept a Roughness map from the content browser
						if (Gui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
							{
								const wchar_t* path = (const wchar_t*)payload->Data;
								std::filesystem::path texturePath = std::filesystem::path(path);

								// Make sure we are recieving an actual texture otherwise we will have trouble opening it
								if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
								{
									SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

									if (texture->IsLoaded())
										material->SetRoughnessMap(texture);
									else
										VX_WARN("Could not load texture {}", texturePath.filename().string());
								}
								else
									VX_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
							}
							Gui::EndDragDropTarget();
						}

						if (!material->GetRoughnessMap())
						{
							float roughness = material->GetRoughness();
							if (Gui::DragFloat("##Roughness", &roughness, 0.01f, 0.01f, 1.0f, "%.2f"))
								material->SetRoughness(roughness);
						}

						Gui::TreePop();
					}

					if (Gui::TreeNodeEx("Ambient Occlusion", treeNodeFlags))
					{
						Gui::Text("Map");
						Gui::SameLine();
						Gui::SetCursorPosX(Gui::GetContentRegionAvail().x);

						if (SharedRef<Texture2D> ambientOcclusionMap = material->GetAmbientOcclusionMap())
						{
							ImVec4 tintColor = { albedo.r, albedo.g, albedo.b, 1.0f };

							if (Gui::ImageButton((void*)ambientOcclusionMap->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 }, -1, { 0, 0, 0, 0 }, tintColor))
								material->SetAmbientOcclusionMap(nullptr);
							else if (Gui::IsItemHovered())
							{
								Gui::BeginTooltip();
								Gui::Text(ambientOcclusionMap->GetPath().c_str());
								Gui::EndTooltip();
							}
						}
						else
							Gui::ImageButton((void*)checkerboardIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });

						// Accept a Ambient Occlusion map from the content browser
						if (Gui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
							{
								const wchar_t* path = (const wchar_t*)payload->Data;
								std::filesystem::path texturePath = std::filesystem::path(path);

								// Make sure we are recieving an actual texture otherwise we will have trouble opening it
								if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
								{
									SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

									if (texture->IsLoaded())
										material->SetAmbientOcclusionMap(texture);
									else
										VX_WARN("Could not load texture {}", texturePath.filename().string());
								}
								else
									VX_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
							}
							Gui::EndDragDropTarget();
						}

						Gui::TreePop();
					}

					Gui::DragFloat2("UV", Math::ValuePtr(component.Scale), 0.05f, 0.0f, 0.0f, "%.2f");

					Gui::Checkbox("Reflective", &component.Reflective);
					Gui::Checkbox("Refractive", &component.Refractive);

					Gui::TreePop();
				}
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
					std::filesystem::path texturePath = std::filesystem::path(path);

					// Make sure we are recieving an actual texture otherwise we will have trouble opening it
					if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
					{
						SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());

						if (texture->IsLoaded())
							component.Texture = texture;
						else
							VX_CORE_WARN("Could not load texture {}", texturePath.filename().string());
					}
					else
						VX_CORE_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
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
			Gui::DragFloat3("Offset", Math::ValuePtr(emitterProperties.Offset), 0.25f, 0.0f, 0.0f, "%.2f");
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

		DrawComponent<TextMeshComponent>("Text Mesh", entity, [](auto& component)
		{
			std::string fontFilepath = component.FontAsset->GetFontAtlas()->GetPath();
			Gui::Text(fontFilepath.c_str());
			Gui::InputText("##FontSource", (char*)fontFilepath.c_str(), fontFilepath.size(), ImGuiInputTextFlags_ReadOnly);

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
							VX_CORE_WARN("Could not load font {}", fontPath.filename().string());
					}
					else
						VX_CORE_WARN("Could not load font, not a '.tff' - {}", fontPath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			char buffer[2048] = { 0 };
			memcpy(buffer, component.TextString.c_str(), component.TextString.size());
			if (Gui::InputTextMultiline("Text", buffer, sizeof(buffer)))
			{
				component.TextString = std::string(buffer);
				component.TextHash = std::hash<std::string>()(component.TextString);
			}

			Gui::ColorEdit4("Color", Math::ValuePtr(component.Color));
			Gui::DragFloat("Line Spacing", &component.LineSpacing);
			Gui::DragFloat("Kerning", &component.Kerning);
			Gui::DragFloat("Max Width", &component.MaxWidth);
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
					std::filesystem::path audioSourcePath = std::filesystem::path(path);

					// Make sure we are recieving an actual audio file otherwise we will have trouble opening it
					if (audioSourcePath.filename().extension() == ".wav" || audioSourcePath.filename().extension() == ".mp3")
					{
						// If there is another file playing we need to stop it
						if (component.Source)
							component.Source->Stop();

						component.Source = AudioSource::Create(audioSourcePath.string());
					}
					else
						VX_CORE_WARN("Could not load audio file, not a '.wav' or '.mp3' - {}", audioSourcePath.filename().string());
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

				if (Gui::Checkbox("Play On Start", &props.PlayOnStart))
					component.Source->SetPlayOnStart(props.PlayOnStart);

				if (Gui::Checkbox("Play One Shot", &props.PlayOneShot))
					component.Source->SetPlayOneShot(props.PlayOneShot);

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

		DrawComponent<RigidBodyComponent>("RigidBody", entity, [](auto& component)
		{
			const char* bodyTypes[] = { "Static", "Dynamic" };
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
						component.Type = static_cast<RigidBodyType>(i);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();

					if (i != arraySize - 1)
						Gui::Separator();
				}

				Gui::EndCombo();
			}

			Gui::DragFloat("Mass", &component.Mass, 0.01f, 0.01f, 1.0f, "%.2f");
			Gui::DragFloat3("Linear Velocity", Math::ValuePtr(component.LinearVelocity), 1.0f, 0.0f, 0.0f, "%.2f");
			Gui::DragFloat("Linear Drag", &component.LinearDrag, 0.01f, 0.01f, 1.0f, "%.2f");
			Gui::DragFloat3("Angular Velocity", Math::ValuePtr(component.AngularVelocity), 1.0f, 0.0f, 0.0f, "%.2f");
			Gui::DragFloat("Angular Drag", &component.AngularDrag, 0.01f, 0.01f, 1.0f, "%.2f");

			Gui::Text("Disable Gravity");
			Gui::SameLine();
			Gui::Checkbox("##DisableGravity", &component.DisableGravity);
			Gui::Text("Is Kinematic");
			Gui::SameLine();
			Gui::Checkbox("##IsKinematic", &component.IsKinematic);

			if (Gui::TreeNodeEx("Constraints", treeNodeFlags))
			{
				Gui::Text("Position");
				Gui::SameLine();
				Gui::Text("X");
				Gui::SameLine();
				Gui::Checkbox("##XTranslationConstraint", &component.LockPositionX);
				Gui::SameLine();
				Gui::Text("Y");
				Gui::SameLine();
				Gui::Checkbox("##YTranslationConstraint", &component.LockPositionY);
				Gui::SameLine();
				Gui::Text("Z");
				Gui::SameLine();
				Gui::Checkbox("##ZTranslationConstraint", &component.LockPositionZ);
				Gui::Text("Rotation");
				Gui::SameLine();
				Gui::Text("X");
				Gui::SameLine();
				Gui::Checkbox("##XRotationConstraint", &component.LockRotationX);
				Gui::SameLine();
				Gui::Text("Y");
				Gui::SameLine();
				Gui::Checkbox("##YRotationConstraint", &component.LockRotationY);
				Gui::SameLine();
				Gui::Text("Z");
				Gui::SameLine();
				Gui::Checkbox("##ZRotationConstraint", &component.LockRotationZ);

				Gui::TreePop();
			}
		});

		DrawComponent<CharacterControllerComponent>("Character Controller", entity, [](auto& component)
		{
			Gui::DragFloat("Slope Limit", &component.SlopeLimitDegrees);
			Gui::DragFloat("Step Offset", &component.StepOffset);
			Gui::Checkbox("Disable Gravity", &component.DisableGravity);
		});

		DrawComponent<PhysicsMaterialComponent>("Physics Material", entity, [](auto& component)
		{
			Gui::DragFloat("Static Friction", &component.StaticFriction, 0.01f, 0.01f, 1.0f, "%.2f");
			Gui::DragFloat("Dynamic Friction", &component.DynamicFriction, 0.01f, 0.01f, 1.0f, "%.2f");
			Gui::DragFloat("Bounciness", &component.Bounciness, 0.01f, 0.01f, 1.0f, "%.2f");
		});

		DrawComponent<BoxColliderComponent>("Box Collider", entity, [](auto& component)
		{
			Gui::DragFloat3("Half Size", Math::ValuePtr(component.HalfSize), 0.01f, 0.0f, 0.0f, "%.2f");
			Gui::DragFloat3("Offset", Math::ValuePtr(component.Offset), 0.01f, 0.0f, 0.0f, "%.2f");
			Gui::Checkbox("Is Trigger", &component.IsTrigger);
		});

		DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [](auto& component)
		{
			Gui::DragFloat("Radius", &component.Radius, 0.01f, 0.0f, 0.0f, "%.2f");
			Gui::DragFloat3("Offset", Math::ValuePtr(component.Offset), 0.01f, 0.0f, 0.0f, "%.2f");
			Gui::Checkbox("Is Trigger", &component.IsTrigger);
		});

		DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, [](auto& component)
		{
			Gui::DragFloat("Radius", &component.Radius, 0.01f, 0.0f, 0.0f, "%.2f");
			Gui::DragFloat("Height", &component.Height, 0.01f, 0.0f, 0.0f, "%.2f");
			Gui::DragFloat3("Offset", Math::ValuePtr(component.Offset), 0.01f, 0.0f, 0.0f, "%.2f");
			Gui::Checkbox("Is Trigger", &component.IsTrigger);
		});

		DrawComponent<StaticMeshColliderComponent>("Static Mesh Collider", entity, [](auto& component)
		{
			
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
						component.Type = static_cast<RigidBody2DType>(i);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();

					if (i != arraySize - 1)
						Gui::Separator();
				}

				Gui::EndCombo();
			}

			Gui::DragFloat2("Velocity", Math::ValuePtr(component.Velocity), 0.01f);
			Gui::DragFloat("Drag", &component.Drag, 0.01f, 0.01f, 1.0f);
			Gui::DragFloat("Angular Drag", &component.AngularDrag, 0.01f, 0.01f, 1.0f);
			Gui::DragFloat("Gravity Scale", &component.GravityScale, 0.01f, 0.01f, 1.0f);
			Gui::Checkbox("Freeze Rotation", &component.FixedRotation);
		});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
		{
			Gui::DragFloat2("Offset", Math::ValuePtr(component.Offset), 0.01f);
			Gui::DragFloat2("Size", Math::ValuePtr(component.Size), 0.01f);
			Gui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Threshold", &component.RestitutionThreshold, 0.1f, 0.0f);
			Gui::Checkbox("Is Tigger", &component.IsTrigger);
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

		DrawComponent<NavMeshAgentComponent>("Nav Mesh Agent", entity, [](auto& component)
		{
			
		});

		DrawComponent<ScriptComponent>("Script", entity, [&](auto& component)
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
							if (Gui::DragFloat(name.c_str(), &data, 0.01f))
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
										
										if (Entity entity = m_ContextScene->TryGetEntityWithUUID(*entityUUID))
										{
											ScriptFieldInstance& fieldInstance = entityFields[name];
											fieldInstance.Field = field;
											fieldInstance.SetValue(*entityUUID);
										}
										else
											VX_WARN("Entity UUID {} was not found in the current scene.", *entityUUID);
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
