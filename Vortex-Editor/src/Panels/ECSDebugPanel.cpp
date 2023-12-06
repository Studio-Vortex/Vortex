#include "ECSDebugPanel.h"

#include <Vortex/Editor/SelectionManager.h>

namespace Vortex {

	void ECSDebugPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!IsOpen)
			return;

		if (!m_ContextScene)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		RenderSceneEntityView();

		if (Entity selected = SelectionManager::GetSelectedEntity())
		{
			RenderSelectedEntityView(selected);
		}

		Gui::End();
	}

	void ECSDebugPanel::RenderSelectedEntityView(Entity selectedEntity)
	{
		if (!selectedEntity)
		{
			return;
		}

		Gui::Text("Selected Entity - %s (%llu)", selectedEntity.GetName(), selectedEntity.GetUUID());

		Gui::SameLine();
		Gui::BeginDisabled(m_ClickedEntities.empty());
		if (Gui::Button((const char*)VX_ICON_CHEVRON_LEFT))
		{
			// pop uuid from the stack
			UUID lastClickedUUID = m_ClickedEntities.top();
			m_ClickedEntities.pop();

			Entity lastClickedEntity = m_ContextScene->TryGetEntityWithUUID(lastClickedUUID);
			SelectionManager::SetSelectedEntity(lastClickedEntity);
		}
		Gui::EndDisabled();
		UI::SetTooltip("previous entity");

		UI::Draw::Underline();
		
		static const char* columns[] = { "Property", "Value" };

		const ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		const ImVec2 tableSize = { contentRegionAvail.x, 125.0f };
		UI::Table("Entity", columns, VX_ARRAYCOUNT(columns), tableSize, [&]()
		{
			Gui::TableNextColumn();
			Gui::Text("Entity");
			Gui::TableNextColumn();
			Gui::Text(selectedEntity.GetName().c_str());

			Gui::TableNextColumn();
			Gui::Text("Marker");
			Gui::TableNextColumn();
			Gui::Text(selectedEntity.GetMarker().c_str());

			Gui::TableNextColumn();
			Gui::Text("UUID");
			Gui::TableNextColumn();
			Gui::Text("%llu", selectedEntity.GetUUID());

			Gui::TableNextColumn();
			Gui::Text("Parent");
			Gui::TableNextColumn();
			std::string parentName = selectedEntity.HasParent() ? selectedEntity.GetParent().GetName() : "None";
			Gui::Text("%s (%llu)", parentName.c_str(), selectedEntity.GetParentUUID());

			if (selectedEntity.HasParent())
			{
				UI::DrawItemActivityOutline();

				if (Gui::IsItemClicked())
				{
					Entity parent = selectedEntity.GetParent();
					SelectionManager::SetSelectedEntity(parent);
					m_ClickedEntities.push(parent);
				}
			}

			Gui::TableNextColumn();
			Gui::Text("Scene Name");
			Gui::TableNextColumn();
			Gui::Text("%s", selectedEntity.GetContextScene()->GetDebugName().c_str());
		});
		
		const auto& children = selectedEntity.Children();

		UI::ShiftCursorY(10.0f);
		Gui::Text("Children");
		UI::Draw::Underline();

		Gui::Text("Count: %u", children.size());

		for (const auto& child : children)
		{
			Entity childEntity = m_ContextScene->TryGetEntityWithUUID(child);
			if (!childEntity)
				continue;

			Gui::Text("  Handle: %s (%llu)", childEntity.GetName().c_str(), child);
			UI::DrawItemActivityOutline();

			if (Gui::IsItemClicked())
			{
				SelectionManager::SetSelectedEntity(childEntity);
				m_ClickedEntities.push(childEntity);
			}
		}

		UI::ShiftCursorY(10.0f);
		Gui::Text("Scene");
		UI::Draw::Underline();

		UI::BeginPropertyGrid();
		UI::Property("Show Component Signature", m_ShowEntityComponentSignature);
		UI::EndPropertyGrid();

		if (m_ShowEntityComponentSignature)
		{
			RenderEntityComponentSignature(AllComponents{}, selectedEntity);
		}
	}

	void ECSDebugPanel::RenderSceneEntityView()
	{
		static const char* columns[] = { "Entity", "UUID" };

		const ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		const ImVec2 tableSize = { contentRegionAvail.x, contentRegionAvail.y / 2.0f };
		UI::Table("Entity List", columns, VX_ARRAYCOUNT(columns), tableSize, [&]()
		{
			m_ContextScene->m_Registry.each([&](auto& entityID)
			{
				Entity entity{ entityID, m_ContextScene.Raw() };

				Gui::TableNextColumn();
				Gui::Text(entity.GetName().c_str());
				UI::DrawItemActivityOutline();

				if (Gui::IsItemClicked())
				{
					SelectionManager::SetSelectedEntity(entity);
					m_ClickedEntities.push(entity);
				}
				
				Gui::TableNextColumn();
				Gui::Text("%llu", entity.GetUUID());
			});
		});
	}

}
