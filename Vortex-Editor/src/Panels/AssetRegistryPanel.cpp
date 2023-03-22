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

		if (Gui::BeginChild("Asset Search"))
		{
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
				std::string typeAsString = Utils::StringFromAssetType(metadata.Type);
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
				std::string buttonName = "Reload##" + handleAsString + typeAsString;
				if (Gui::Button(buttonName.c_str()))
				{
					Project::GetEditorAssetManager()->ReloadData(metadata.Handle);
					VX_CONSOLE_LOG_INFO("Asset Reloaded: Handle: '{}', Path: '{}'", handleAsString, filepath);
				}

				UI::Draw::Underline();
				for (uint32_t i = 0; i < 2; i++)
					Gui::Spacing();
			}

			Gui::EndChild();
		}
		
		if (Gui::BeginChild("Debug Tools"))
		{
			if (Gui::Button("Reload All Assets"))
			{
				Project::GetEditorAssetManager()->ReloadAssets();
			}

			Gui::EndChild();
		}
	}

}
