#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel(const std::filesystem::path& assetDir);
		~ContentBrowserPanel() override = default;
		
		void OnGuiRender() override;
		
		EDITOR_PANEL_TYPE(ContentBrowser)

	private:
		void RenderCreateItemPopup();
		void RenderFileExplorer();
		void RenderMenuBar();
		void RenderRightClickItemPopup(const std::filesystem::path& currentPath);
		void RenderThumbnailSlider();
		SharedReference<Texture2D> FindSuitableItemIcon(const std::filesystem::directory_entry& directoryEntry, const std::filesystem::path& currentItemPath);
		void FindTextureFromAssetManager(const std::filesystem::path& currentItemPath, SharedReference<Texture2D>& itemIcon);
		void FindEnvironmentMapFromAssetManager(const std::filesystem::path& currentItemPath, SharedReference<Texture2D>& itemIcon);
		void FindMeshIcon(const std::filesystem::path& extension, SharedReference<Texture2D>& itemIcon);

	private:
		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_ItemPathToRename;

		float m_ThumbnailSize = 96.0f;
		float m_ThumbnailPadding = 16.0f;

		ImGuiTextFilter m_SearchInputTextFilter;
	};

}
