#include "SceneHierarchyPanel.h"

#include <imgui_internal.h>

namespace Sparky {

	SceneHierarchyPanel::SceneHierarchyPanel(const SharedRef<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const SharedRef<Scene>& context)
	{
		m_ContextScene = context;
	}

	void SceneHierarchyPanel::OnGuiRender()
	{
		Gui::Begin("Scene Hierarchy");
		m_ContextScene->m_Registry.each([&](auto entityID)
		{
			Entity entity{ entityID, m_ContextScene.get() };
			DrawEntityNode(entity);
		});

		if (Gui::IsMouseDown(0) && Gui::IsWindowHovered())
			m_SelectedEntity = {};

		// Right-click on blank space
		if (Gui::BeginPopupContextWindow(0, 1, false))
		{
			if (Gui::MenuItem("Add Empty Entity"))
				m_ContextScene->CreateEntity("Empty Entity");

			Gui::EndPopup();
		}

		Gui::End();

		Gui::Begin("Inspector");
		if (m_SelectedEntity)
		{
			DrawComponents(m_SelectedEntity);
		}

		Gui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = Gui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (Gui::IsItemClicked())
			m_SelectedEntity = entity;

		bool entityShouldBeDeleted = false;

		if (Gui::BeginPopupContextItem())
		{
			if (Gui::MenuItem("Delete Entity"))
				entityShouldBeDeleted = true;

			Gui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = Gui::TreeNodeEx((void*)8373456, flags, tag.c_str());
			if (opened)
				Gui::TreePop();
			Gui::TreePop();
		}

		if (entityShouldBeDeleted)
		{
			m_ContextScene->DestroyEntity(entity);

			if (m_SelectedEntity == entity)
				m_SelectedEntity = {};
		}
	}

	static void DrawVec3Controls(const std::string& label, Math::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = Gui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		Gui::PushID(label.c_str());
		Gui::Columns(2);
		Gui::SetColumnWidth(0, columnWidth);
		Gui::Text(label.c_str());
		Gui::NextColumn();

		Gui::PushMultiItemsWidths(3, Gui::CalcItemWidth());
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3, lineHeight };

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("X", buttonSize))
			values.x = resetValue;
		Gui::PopFont();
		Gui::SameLine();
		Gui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		Gui::PopItemWidth();
		Gui::SameLine();
		Gui::PopStyleColor(3);

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("Y", buttonSize))
			values.y = resetValue;
		Gui::PopFont();
		Gui::SameLine();
		Gui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		Gui::PopItemWidth();
		Gui::SameLine();
		Gui::PopStyleColor(3);

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("Z", buttonSize))
			values.z = resetValue;
		Gui::PopFont();
		Gui::SameLine();
		Gui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		Gui::PopItemWidth();
		Gui::PopStyleColor(3);

		Gui::PopStyleVar();
		Gui::Columns(1);
		Gui::PopID();
	}

	template <typename TComponent, typename UIFunction>
	void DrawComponent(const std::string& name, Entity entity, UIFunction uiCallback)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<TComponent>())
		{
			auto& component = entity.GetComponent<TComponent>();
			ImVec2 contentRegionAvailable = Gui::GetContentRegionAvail();

			Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = Gui::TreeNodeEx((void*)typeid(TComponent).hash_code(), treeNodeFlags, name.c_str());
			Gui::PopStyleVar();

			Gui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (Gui::Button("...", ImVec2{ lineHeight, lineHeight }))
				Gui::OpenPopup("ComponentSettings");

			bool componentShouldBeRemoved = false;
			if (Gui::BeginPopup("ComponentSettings"))
			{
				if (Gui::MenuItem("Remove Component"))
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
			if (Gui::InputText("##Tag", buffer, sizeof(buffer)))
				tag = std::string(buffer);
		}

		Gui::SameLine();
		Gui::PushItemWidth(-1);

		if (Gui::Button("Add Component"))
			Gui::OpenPopup("AddComponent");

		if (Gui::BeginPopup("AddComponent"))
		{
			if (!m_SelectedEntity.HasComponent<CameraComponent>())
			{
				if (Gui::MenuItem("Camera"))
				{
					m_SelectedEntity.AddComponent<CameraComponent>();
					Gui::CloseCurrentPopup();
				}
			}

			if (!m_SelectedEntity.HasComponent<SpriteComponent>())
			{
				if (Gui::MenuItem("Sprite"))
				{
					m_SelectedEntity.AddComponent<SpriteComponent>();
					Gui::CloseCurrentPopup();
				}
			}

			if (Gui::MenuItem("Native Script"))
			{
				m_SelectedEntity.AddComponent<NativeScriptComponent>();
				Gui::CloseCurrentPopup();
			}

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
		});

		DrawComponent<SpriteComponent>("Sprite", entity, [](auto& component)
		{
			Gui::ColorEdit4("Color", Math::ValuePtr(component.SpriteColor));
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

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
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

		DrawComponent<NativeScriptComponent>("Native Script", entity, [](auto& component)
		{

		});
	}

}