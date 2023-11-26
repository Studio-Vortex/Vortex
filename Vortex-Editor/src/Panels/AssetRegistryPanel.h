#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class AssetRegistryPanel : public EditorPanel
	{
	public:
		~AssetRegistryPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(AssetRegistry)

	private:
		void RenderLoadedAssets();
		void RenderAssetTypeTable();

	private:
		ImGuiTextFilter m_AssetSearchTextFilter;
	};

}
