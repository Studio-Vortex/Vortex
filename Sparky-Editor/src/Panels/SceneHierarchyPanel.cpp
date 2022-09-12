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
			if (Gui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				auto& transform = entity.GetComponent<TransformComponent>().Transform;
				Gui::DragFloat3(" ", Math::ValuePtr(transform[3]), 0.1f);
				Gui::TreePop();
			}
		}

		if (m_SelectedEntity.HasComponent<SpriteComponent>())
		{

		}

		if (m_SelectedEntity.HasComponent<TransformComponent>())
		{

		}

		if (m_SelectedEntity.HasComponent<TransformComponent>())
		{

		}
	}

}