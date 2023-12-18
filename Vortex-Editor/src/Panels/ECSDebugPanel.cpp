#include "ECSDebugPanel.h"

#include <Vortex/Editor/SelectionManager.h>

namespace Vortex {

	void ECSDebugPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		if (!m_ContextScene)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		RenderSceneActorView();

		if (Actor selected = SelectionManager::GetSelectedActor())
		{
			RenderSelectedActorView(selected);
		}

		Gui::End();
	}

	void ECSDebugPanel::RenderSelectedActorView(Actor selectedActor)
	{
		if (!selectedActor)
		{
			return;
		}

		Gui::Text("Selected Actor - %s (%llu)", selectedActor.GetName(), selectedActor.GetUUID());

		Gui::SameLine();
		Gui::BeginDisabled(m_ClickedActors.empty());
		if (Gui::Button((const char*)VX_ICON_CHEVRON_LEFT))
		{
			// pop uuid from the stack
			UUID lastClickedUUID = m_ClickedActors.top();
			m_ClickedActors.pop();

			Actor lastClickedActor = m_ContextScene->TryGetActorWithUUID(lastClickedUUID);
			SelectionManager::SetSelectedActor(lastClickedActor);
		}
		Gui::EndDisabled();
		UI::SetTooltip("previous actor");

		UI::Draw::Underline();
		
		static const char* columns[] = { "Property", "Value" };

		const ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		const ImVec2 tableSize = { contentRegionAvail.x, 125.0f };
		UI::Table("Actor", columns, VX_ARRAYSIZE(columns), tableSize, [&]()
		{
			Gui::TableNextColumn();
			Gui::Text("Actor");
			Gui::TableNextColumn();
			Gui::Text(selectedActor.GetName().c_str());

			Gui::TableNextColumn();
			Gui::Text("Marker");
			Gui::TableNextColumn();
			Gui::Text(selectedActor.GetMarker().c_str());

			Gui::TableNextColumn();
			Gui::Text("UUID");
			Gui::TableNextColumn();
			Gui::Text("%llu", selectedActor.GetUUID());

			Gui::TableNextColumn();
			Gui::Text("Parent");
			Gui::TableNextColumn();
			std::string parentName = selectedActor.HasParent() ? selectedActor.GetParent().GetName() : "None";
			Gui::Text("%s (%llu)", parentName.c_str(), selectedActor.GetParentUUID());

			if (selectedActor.HasParent())
			{
				UI::DrawItemActivityOutline();

				if (Gui::IsItemClicked())
				{
					Actor parent = selectedActor.GetParent();
					SelectionManager::SetSelectedActor(parent);
					m_ClickedActors.push(parent);
				}
			}

			Gui::TableNextColumn();
			Gui::Text("Scene Name");
			Gui::TableNextColumn();
			Gui::Text("%s", selectedActor.GetContextScene()->GetDebugName().c_str());
		});
		
		const auto& children = selectedActor.Children();

		UI::ShiftCursorY(10.0f);
		Gui::Text("Children");
		UI::Draw::Underline();

		Gui::Text("Count: %u", children.size());

		for (const auto& child : children)
		{
			Actor childActor = m_ContextScene->TryGetActorWithUUID(child);
			if (!childActor)
				continue;

			Gui::Text("  Handle: %s (%llu)", childActor.GetName().c_str(), child);
			UI::DrawItemActivityOutline();

			if (Gui::IsItemClicked())
			{
				SelectionManager::SetSelectedActor(childActor);
				m_ClickedActors.push(childActor);
			}
		}

		UI::ShiftCursorY(10.0f);
		Gui::Text("Scene");
		UI::Draw::Underline();

		UI::BeginPropertyGrid();
		UI::Property("Show Component Signature", m_ShowActorComponentSignature);
		UI::EndPropertyGrid();

		if (m_ShowActorComponentSignature)
		{
			RenderActorComponentSignature(AllComponents{}, selectedActor);
		}
	}

	void ECSDebugPanel::RenderSceneActorView()
	{
		static const char* columns[] = { "Actor", "UUID" };

		const ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		const ImVec2 tableSize = { contentRegionAvail.x, contentRegionAvail.y / 2.0f };
		UI::Table("Actor List", columns, VX_ARRAYSIZE(columns), tableSize, [&]()
		{
			m_ContextScene->m_Registry.each([&](auto& actorID)
			{
				Actor actor{ actorID, m_ContextScene.Raw() };

				Gui::TableNextColumn();
				Gui::Text(actor.GetName().c_str());
				UI::DrawItemActivityOutline();

				if (Gui::IsItemClicked())
				{
					SelectionManager::SetSelectedActor(actor);
					m_ClickedActors.push(actor);
				}
				
				Gui::TableNextColumn();
				Gui::Text("%llu", actor.GetUUID());
			});
		});
	}

}
