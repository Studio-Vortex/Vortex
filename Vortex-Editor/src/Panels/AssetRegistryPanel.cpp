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
		SharedRef<EditorAssetManager> assetManager = Project::GetEditorAssetManager();
		const AssetRegistry& assetRegistry = assetManager->GetAssetRegistry();

		for (auto& [handle, metadata] : assetRegistry)
		{
			Gui::Text("Handle: %u", metadata.Handle);
			Gui::Text("Filepath: %s", metadata.Filepath);
			Gui::Text("Asset Type: %s", Utils::AssetTypeToString(metadata.Type).c_str());
			Gui::Separator();
		}
	}

}
