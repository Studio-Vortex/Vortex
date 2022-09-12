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

}