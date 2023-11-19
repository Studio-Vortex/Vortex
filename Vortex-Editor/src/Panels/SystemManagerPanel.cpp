#include "SystemManagerPanel.h"

namespace Vortex {

	void SystemManagerPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		auto& assetSystems = SystemManager::s_AssetSystems;

		for (auto& it : assetSystems)
		{
			SharedReference<IAssetSystem> assetSystem = it.second;
			const std::string& assetSystemName = assetSystem->GetDebugName();
			Gui::Text(assetSystemName.c_str());
			UI::Draw::Underline();
			assetSystem->OnGuiRender();
			UI::Draw::Underline();
		}

		Gui::End();
	}

}
