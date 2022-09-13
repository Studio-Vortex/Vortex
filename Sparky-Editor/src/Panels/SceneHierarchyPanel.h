#pragma once

#include <Sparky.h>

namespace Sparky {
	
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const SharedRef<Scene>& context);

		void SetContext(const SharedRef<Scene>& context);

		void OnGuiRender();

	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

		template <typename TComponent>
		void DrawComponent(const char* name, bool removeable, void (*callback)(Entity&))
		{
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
			bool open = Gui::TreeNodeEx((void*)typeid(TComponent).hash_code(), treeNodeFlags, name);

			bool componentShouldBeRemoved = false;

			if (removeable)
			{
				Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				Gui::SameLine(Gui::GetWindowWidth() - 25.0f);
				if (Gui::Button("+", ImVec2{ 20.0f, 20.0f }))
					Gui::OpenPopup("ComponentSettings");
				Gui::PopStyleVar();

				if (Gui::BeginPopup("ComponentSettings"))
				{
					if (Gui::MenuItem("Remove Component"))
						componentShouldBeRemoved = true;

					Gui::EndPopup();
				}
			}

			if (open)
			{
				callback(m_SelectedEntity);
				Gui::TreePop();
			}

			if (removeable)
			{
				if (componentShouldBeRemoved)
					m_SelectedEntity.RemoveComponent<TComponent>();
			}
		}

	private:
		SharedRef<Scene> m_ContextScene;
		Entity m_SelectedEntity;
	};

}
