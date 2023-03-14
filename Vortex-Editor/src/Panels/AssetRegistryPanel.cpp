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

		for (const auto& [handle, metadata] : editorAssetManager->GetAssetRegistry())
		{
			Gui::Text("AssetHandle: %llu", handle);
			Gui::Text("Filepath: %s", metadata.Filepath);
			Gui::Text("Type: %s", Utils::AssetTypeToString(metadata.Type));
			Gui::Text("IsDataLoaded: %b", metadata.IsDataLoaded);
			Gui::Text("IsMemoryOnly: %b", metadata.IsMemoryOnly);

			UI::Draw::Underline();
		}
	}

}
