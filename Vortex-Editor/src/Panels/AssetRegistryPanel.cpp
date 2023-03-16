#include "AssetRegistryPanel.h"

#include <imgui_internal.h>

namespace Vortex {

	void AssetRegistryPanel::OnGuiRender()
	{
		if (!s_ShowPanel)
			return;

		Gui::Begin("Asset Registry", &s_ShowPanel);

		RenderLoadedAssets();

		Gui::End();
	}

	void AssetRegistryPanel::RenderLoadedAssets()
	{
		SharedReference<EditorAssetManager> editorAssetManager = Project::GetEditorAssetManager();

		if (Gui::InputTextWithHint("##AssetRegistrySearch", "Search...", m_AssetSearchTextFilter.InputBuf, IM_ARRAYSIZE(m_AssetSearchTextFilter.InputBuf)))
		{
			m_AssetSearchTextFilter.Build();
		}
		UI::Draw::Underline();
		Gui::Spacing();

		const bool searchedString = strlen(m_AssetSearchTextFilter.InputBuf) != 0;

		for (const auto& [handle, metadata] : editorAssetManager->GetAssetRegistry())
		{
			std::string filepath = metadata.Filepath.string();
			std::string typeAsString = Asset::GetAssetNameFromType(metadata.Type);
			std::string handleAsString = std::to_string(metadata.Handle);

			const bool matchingSearch = m_AssetSearchTextFilter.PassFilter(filepath.c_str())
				|| m_AssetSearchTextFilter.PassFilter(typeAsString.c_str())
				|| m_AssetSearchTextFilter.PassFilter(handleAsString.c_str());

			if (searchedString && !matchingSearch)
				continue;

			Gui::Text("AssetHandle: %s", handleAsString.c_str());
			Gui::Text("Filepath: %s", filepath.c_str());
			Gui::Text("Type: %s", typeAsString.c_str());
			Gui::Text("IsDataLoaded: %s", metadata.IsDataLoaded ? "true" : "false");
			Gui::Text("IsMemoryOnly: %s", metadata.IsMemoryOnly ? "true" : "false");

			UI::Draw::Underline();

			Gui::Spacing();
		}
	}

}
