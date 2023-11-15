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

		if (Entity selected = SelectionManager::GetSelectedEntity())
		{
			RenderSelectedEntityView(selected);
		}

		RenderSceneEntityView();

		Gui::End();
	}

	void ECSDebugPanel::RenderSelectedEntityView(Entity selectedEntity)
	{
		Gui::Text("Selected Entity");
		UI::Draw::Underline();
		
		static const char* columns[] = { "Property", "Value" };
		UI::Table("Entity", columns, VX_ARRAYCOUNT(columns), { Gui::GetContentRegionAvail().x, 125.0f }, [&]()
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
			Gui::Text("  Handle: %llu", child);
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

		UI::Table("Entity List", columns, VX_ARRAYCOUNT(columns), Gui::GetContentRegionAvail(), [&]()
		{
			m_ContextScene->m_Registry.each([&](auto& entityID)
			{
				Entity entity{ entityID, m_ContextScene.Raw() };

				Gui::TableNextColumn();
				Gui::Text(entity.GetName().c_str());
				
				Gui::TableNextColumn();
				Gui::Text("%llu", entity.GetUUID());
			});
		});
	}

}
