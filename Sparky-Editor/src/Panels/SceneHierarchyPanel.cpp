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
			bool entityIsValid = m_ContextScene->m_Registry.valid(entityID);

			if (entityIsValid)
			{
				Entity entity{ entityID, m_ContextScene.get() };
				DrawEntityNode(entity);
			}
		});

		if (Gui::IsMouseDown(0) && Gui::IsWindowHovered())
			m_SelectedEntity = {};

		Gui::End();

		Gui::Begin("Inspector");

		if (m_SelectedEntity)
			DrawComponents(m_SelectedEntity);

		Gui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = Gui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (Gui::IsItemClicked())
			m_SelectedEntity = entity;

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			bool opened = Gui::TreeNodeEx((void*)8373456, flags, tag.c_str());

			if (opened)
				Gui::TreePop();

			Gui::TreePop();
		}
	}

	static void DrawVec3Controls(const std::string& label, Math::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
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
		if (Gui::Button("X", buttonSize))
			values.x = resetValue;
		Gui::SameLine();
		Gui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		Gui::PopItemWidth();
		Gui::SameLine();
		Gui::PopStyleColor(3);

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		if (Gui::Button("Y", buttonSize))
			values.y = resetValue;
		Gui::SameLine();
		Gui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		Gui::PopItemWidth();
		Gui::SameLine();
		Gui::PopStyleColor(3);

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		if (Gui::Button("Z", buttonSize))
			values.z = resetValue;
		Gui::SameLine();
		Gui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		Gui::PopItemWidth();
		Gui::PopStyleColor(3);

		Gui::PopStyleVar();
		Gui::Columns(1);
		Gui::PopID();
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (m_SelectedEntity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			Gui::Text("Tag"); Gui::SameLine();
			if (Gui::InputText(" ", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}

			Gui::Separator();
		}

		if (m_SelectedEntity.HasComponent<TransformComponent>())
		{
			DrawComponent<TransformComponent>("Transform", [](Entity e)
			{
				auto& transformComponent = e.GetComponent<TransformComponent>();
				DrawVec3Controls("Translation", transformComponent.Translation);
				Math::vec3 rotation = Math::Rad2Deg(transformComponent.Rotation);
				DrawVec3Controls("Rotation", rotation);
				transformComponent.Rotation = Math::Deg2Rad(rotation);
				DrawVec3Controls("Scale", transformComponent.Scale, 1.0f);
			});
		}

		if (m_SelectedEntity.HasComponent<SpriteComponent>())
		{
			DrawComponent<SpriteComponent>("Sprite", [](Entity e)
			{
				auto& spriteComponent = e.GetComponent<SpriteComponent>();
				Gui::ColorEdit4("Color", Math::ValuePtr(spriteComponent.SpriteColor));
			});
		}

		if (m_SelectedEntity.HasComponent<CameraComponent>())
		{
			DrawComponent<CameraComponent>("Camera", [](Entity e)
			{
				auto& cameraComponent = e.GetComponent<CameraComponent>();
				auto& camera = cameraComponent.Camera;

				Gui::Checkbox("Primary", &cameraComponent.Primary);

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

					Gui::Checkbox("Fixed Aspect Ratio", &cameraComponent.FixedAspectRatio);
				}
			});
		}

		if (m_SelectedEntity.HasComponent<NativeScriptComponent>())
		{
			DrawComponent<NativeScriptComponent>("Native Script", [](Entity e)
			{

			});
		}
	}

}