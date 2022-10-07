#include "SceneHierarchyPanel.h"

#include "Sparky/Scripting/ScriptEngine.h"
#include "Sparky/Scene/Scene.h"

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
	}

	void SceneHierarchyPanel::OnGuiRender(Entity hoveredEntity)
	{
		Gui::Begin("Scene Hierarchy");

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
				if (Gui::MenuItem("Create Empty Entity"))
					m_SelectedEntity = m_ContextScene->CreateEntity("Empty Entity");
				Gui::Separator();

				if (Gui::BeginMenu("Create 2D Entity"))
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

				if (Gui::BeginMenu("Create Physics 2D Entity"))
				{
					if (Gui::MenuItem("Empty RigidBody"))
					{
						m_SelectedEntity = m_ContextScene->CreateEntity("Empty RigidBody2D");
						m_SelectedEntity.AddComponent<RigidBody2DComponent>();
					}
					Gui::Separator();

					if (Gui::MenuItem("Empty Box Collider"))
					{
						m_SelectedEntity = m_ContextScene->CreateEntity("Empty BoxCollider2D");
						m_SelectedEntity.AddComponent<RigidBody2DComponent>();
						m_SelectedEntity.AddComponent<BoxCollider2DComponent>();
					}
					Gui::Separator();

					if (Gui::MenuItem("Empty Circle Collider"))
					{
						m_SelectedEntity = m_ContextScene->CreateEntity("Empty CircleCollider2D");
						m_SelectedEntity.AddComponent<RigidBody2DComponent>();
						m_SelectedEntity.AddComponent<CircleCollider2DComponent>();
					}

					Gui::EndMenu();
				}
				Gui::Separator();

				if (Gui::BeginMenu("Create Camera Entity"))
				{
					if (Gui::MenuItem("Perspective Camera"))
					{
						m_SelectedEntity = m_ContextScene->CreateEntity("Camera");
						auto& cameraComponent = m_SelectedEntity.AddComponent<CameraComponent>();
						cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
					}
					Gui::Separator();

					if (Gui::MenuItem("Orthographic Camera"))
					{
						m_SelectedEntity = m_ContextScene->CreateEntity("Camera");
						auto& cameraComponent = m_SelectedEntity.AddComponent<CameraComponent>();
						cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
					}
					Gui::EndMenu();
				}

				Gui::EndPopup();
			}
		}

		Gui::End();

		Gui::Begin("Inspector");

		if (m_SelectedEntity)
			DrawComponents(m_SelectedEntity);
		else
		{
			const char* name = "None";
			if (hoveredEntity && m_ContextScene)
				name = hoveredEntity.GetComponent<TagComponent>().Tag.c_str();

			Gui::SetCursorPosX(10.0f);
			Gui::Text("Hovered Entity: %s", name);
		}

		Gui::End();
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

	static void DrawVec3Controls(const std::string& label, Math::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
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
			values.x = resetValue;
		Gui::PopFont();
		Gui::PopStyleColor(3);

		Gui::SameLine();
		Gui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		Gui::PopItemWidth();
		Gui::SameLine();

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("Y", buttonSize))
			values.y = resetValue;
		Gui::PopFont();
		Gui::PopStyleColor(3);

		Gui::SameLine();
		Gui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		Gui::PopItemWidth();
		Gui::SameLine();

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("Z", buttonSize))
			values.z = resetValue;
		Gui::PopFont();
		Gui::PopStyleColor(3);

		Gui::SameLine();
		Gui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		Gui::PopItemWidth();

		Gui::PopStyleVar();
		Gui::Columns(1);
		Gui::PopID();
	}

	template <typename TComponent, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiCallback, bool removeable = true)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

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
			if (Gui::Button("+", ImVec2{ lineHeight, lineHeight }))
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
				m_EntityShouldBeRenamed = false;
			}
		}

		Gui::SameLine();
		Gui::PushItemWidth(-1);

		bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		if (Gui::Button("Add Component") || (Input::IsKeyPressed(Key::A) && controlPressed && shiftPressed && Gui::IsWindowHovered()))
			Gui::OpenPopup("AddComponent");

		if (Gui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentPopup<TransformComponent>("Transform");
			DisplayAddComponentPopup<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentPopup<CircleRendererComponent>("Circle Renderer");
			DisplayAddComponentPopup<CameraComponent>("Camera");
			DisplayAddComponentPopup<RigidBody2DComponent>("RigidBody 2D");
			DisplayAddComponentPopup<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentPopup<CircleCollider2DComponent>("Circle Collider 2D");

			// Allow for multiple Scripts on an entity
			DisplayAddComponentPopup<ScriptComponent>("C# Script");
			DisplayAddComponentPopup<NativeScriptComponent>("C++ Script", false, true);

			Gui::EndPopup();
		}

		Gui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
		{
			DrawVec3Controls("Translation", component.Translation);
			Math::vec3 rotation = Math::Rad2Deg(component.Rotation);
			DrawVec3Controls("Rotation", rotation);
			component.Rotation = Math::Deg2Rad(rotation);
			DrawVec3Controls("Scale", component.Scale, 1.0f);
		}, false);

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
		{
			Gui::ColorEdit4("Color", Math::ValuePtr(component.SpriteColor));

			if (Gui::Button("Texture", ImVec2{ 100.0f, 0.0f }))
				component.Texture = nullptr;

			// Accept a Texture from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
					SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());
					if (texture->IsLoaded())
						component.Texture = texture;
					else
						SP_WARN("Could not load texture {}", texturePath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			Gui::DragFloat("Scale", &component.Scale, 0.1f, 0.0f, 100.0f);
		});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
		{
			Gui::ColorEdit4("Color", Math::ValuePtr(component.Color));
			Gui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
			Gui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
		});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
		{
			auto& camera = component.Camera;

			Gui::Checkbox("Primary", &component.Primary);

			const char* projectionTypes[] = { "Perspective", "Othrographic" };
			const char* currentProjectionType = projectionTypes[(uint32_t)camera.GetProjectionType()];

			if (Gui::BeginCombo("Projection", currentProjectionType))
			{
				for (uint32_t i = 0; i < 2; i++)
				{
					bool isSelected = strcmp(currentProjectionType, projectionTypes[i]) == 0;
					if (Gui::Selectable(projectionTypes[i], isSelected))
					{
						currentProjectionType = projectionTypes[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();
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

		DrawComponent<RigidBody2DComponent>("RigidBody 2D", entity, [](auto& component)
		{
			const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
			const char* currentBodyType = bodyTypes[(uint32_t)component.Type];

			if (Gui::BeginCombo("Body Type", currentBodyType))
			{
				for (uint32_t i = 0; i < 3; i++)
				{
					bool isSelected = strcmp(currentBodyType, bodyTypes[i]) == 0;
					if (Gui::Selectable(bodyTypes[i], isSelected))
					{
						currentBodyType = bodyTypes[i];
						component.Type = static_cast<RigidBody2DComponent::BodyType>(i);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();
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
			Gui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.1f, 0.0f);
		});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
		{
			Gui::DragFloat2("Offset", Math::ValuePtr(component.Offset), 0.01f);
			Gui::DragFloat("Radius", &component.Radius, 0.01, 0.01f);
			Gui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			Gui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.1f, 0.0f);
		});

		DrawComponent<ScriptComponent>("C# Script", entity, [entity, scene = m_ContextScene](auto& component)
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
				for (uint32_t i = 0; i < entityClassNameStrings.size(); i++)
				{
					const char* currentEntityClassNameString = entityClassNameStrings[i].c_str();
					bool isSelected = strcmp(currentClassName, currentEntityClassNameString) == 0;

					if (Gui::Selectable(currentEntityClassNameString, isSelected))
					{
						currentClassName = currentEntityClassNameString;
						component.ClassName = std::string(currentClassName);
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();
				}

				Gui::EndCombo();
			}

			// Fields
			bool sceneRunning = scene->IsRunning();

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
							}
						}
					}
				}
			}
		});

		DrawComponent<NativeScriptComponent>("Native Script", entity, [](auto& component) {});
	}

}
