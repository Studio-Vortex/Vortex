#include "SceneHierarchyPanel.h"

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
				auto& transform = e.GetComponent<TransformComponent>().Transform;
				Gui::DragFloat3("Position", Math::ValuePtr(transform[3]), 0.1f);
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