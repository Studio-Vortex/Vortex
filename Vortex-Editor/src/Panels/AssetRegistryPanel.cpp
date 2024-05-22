#include "AssetRegistryPanel.h"

#include <imgui_internal.h>

namespace Vortex {

	void AssetRegistryPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

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
			Gui::SameLine();
			if (Gui::Button("Reload All Assets"))
			{
				Project::GetEditorAssetManager()->ReloadAssets();
				VX_CONSOLE_LOG_INFO("All assets reloaded sucessfully");
			}

			RenderAssetTypeTable();

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
				
				Gui::SameLine();
				
				std::string buttonName = "Reload##" + handleAsString + typeAsString;
				if (Gui::Button(buttonName.c_str()))
				{
					Project::GetEditorAssetManager()->ReloadData(metadata.Handle);
					VX_CONSOLE_LOG_INFO("Asset Reloaded: Handle: '{}', Path: '{}'", handleAsString, filepath);
				}
				
				Gui::SameLine();

				buttonName = "Remove##" + handleAsString + typeAsString;
				if (Gui::Button(buttonName.c_str()))
				{
					if (Project::GetEditorAssetManager()->RemoveAsset(metadata.Handle))
						VX_CONSOLE_LOG_INFO("Asset Removed: Handle: '{}', Path: '{}'", handleAsString, filepath);
					else
						VX_CONSOLE_LOG_ERROR("Failed to Remove Asset: Handle: '{}', Path: '{}'", handleAsString, filepath);
				}

				Gui::Text("Filepath: %s", filepath.c_str());
				Gui::Text("Type: %s", typeAsString.c_str());
				Gui::Text("IsDataLoaded: %s", metadata.IsDataLoaded ? "true" : "false");
				Gui::Text("IsMemoryOnly: %s", metadata.IsMemoryOnly ? "true" : "false");

				UI::Draw::Underline();
				for (uint32_t i = 0; i < 2; i++)
					Gui::Spacing();
			}

			Gui::EndChild();
		}
	}

	void AssetRegistryPanel::RenderAssetTypeTable()
	{
		if (Gui::BeginTable("Registry", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, {}, 100))
		{
			Gui::TableNextColumn();
			for (uint32_t i = 1; i < 5; i++)
			{
				std::string assetTypeStr = Utils::StringFromAssetType((AssetType)i);
				ImVec2 buttonSize = ImVec2(Gui::GetColumnWidth(), 0);
				if (Gui::Button(assetTypeStr.c_str(), buttonSize))
				{
					memset(m_AssetSearchTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_AssetSearchTextFilter.InputBuf));
					VX_CORE_ASSERT(IM_ARRAYSIZE(m_AssetSearchTextFilter.InputBuf) > assetTypeStr.size(), "Asset type string is too long!");
					memcpy(m_AssetSearchTextFilter.InputBuf, assetTypeStr.data(), assetTypeStr.size());
					m_AssetSearchTextFilter.Build();
				}
			}

			Gui::TableNextColumn();
			for (uint32_t i = 5; i < 9; i++)
			{
				std::string assetTypeStr = Utils::StringFromAssetType((AssetType)i);
				ImVec2 buttonSize = ImVec2(Gui::GetColumnWidth(), 0);
				if (Gui::Button(assetTypeStr.c_str(), buttonSize))
				{
					memset(m_AssetSearchTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_AssetSearchTextFilter.InputBuf));
					VX_CORE_ASSERT(IM_ARRAYSIZE(m_AssetSearchTextFilter.InputBuf) > assetTypeStr.size(), "Asset type string is too long!");
					memcpy(m_AssetSearchTextFilter.InputBuf, assetTypeStr.data(), assetTypeStr.size());
					m_AssetSearchTextFilter.Build();
				}
			}

			Gui::TableNextColumn();
			for (uint32_t i = 9; i < 13; i++)
			{
				std::string assetTypeStr = Utils::StringFromAssetType((AssetType)i);
				ImVec2 buttonSize = ImVec2(Gui::GetColumnWidth(), 0);
				if (Gui::Button(assetTypeStr.c_str(), buttonSize))
				{
					memset(m_AssetSearchTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_AssetSearchTextFilter.InputBuf));
					VX_CORE_ASSERT(IM_ARRAYSIZE(m_AssetSearchTextFilter.InputBuf) > assetTypeStr.size(), "Asset type string is too long!");
					memcpy(m_AssetSearchTextFilter.InputBuf, assetTypeStr.data(), assetTypeStr.size());
					m_AssetSearchTextFilter.Build();
				}
			}

			Gui::TableNextColumn();
			for (uint32_t i = 13; i < 16; i++)
			{
				std::string assetTypeStr = Utils::StringFromAssetType((AssetType)i);
				ImVec2 buttonSize = ImVec2(Gui::GetColumnWidth(), 0);
				if (Gui::Button(assetTypeStr.c_str(), buttonSize))
				{
					memset(m_AssetSearchTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_AssetSearchTextFilter.InputBuf));
					VX_CORE_ASSERT(IM_ARRAYSIZE(m_AssetSearchTextFilter.InputBuf) > assetTypeStr.size(), "Asset type string is too long!");
					memcpy(m_AssetSearchTextFilter.InputBuf, assetTypeStr.data(), assetTypeStr.size());
					m_AssetSearchTextFilter.Build();
				}
			}

			Gui::EndTable();
		}
	}

}
