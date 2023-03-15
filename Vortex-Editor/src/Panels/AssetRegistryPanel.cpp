#include "AssetRegistryPanel.h"

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

		UI::BeginPropertyGrid();

		for (const auto& [handle, metadata] : editorAssetManager->GetAssetRegistry())
		{
			Gui::Text("AssetHandle: %llu", handle);
			Gui::Text("Filepath: %s", metadata.Filepath);
			Gui::Text("Type: %s", Asset::GetAssetNameFromType(metadata.Type));
			Gui::Text("IsDataLoaded: %s", metadata.IsDataLoaded ? "true" : "false");
			Gui::Text("IsMemoryOnly: %s", metadata.IsMemoryOnly ? "true" : "false");

			UI::Draw::Underline();
		}

		UI::EndPropertyGrid();
	}

}
