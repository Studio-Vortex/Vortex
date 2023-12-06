#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel(const Fs::Path& assetDir);
		~ContentBrowserPanel() override = default;
		
		void OnGuiRender() override;
		
		EDITOR_PANEL_TYPE(ContentBrowser)

	private:
		void RenderCreateItemPopup();
		void RenderFileExplorer();
		void RenderMenuBar();
		void RenderRightClickItemPopup(const Fs::Path& currentPath);
		void RenameItem(const Fs::Path& currentPath);
		void OnMaterialFileRenamed(const Fs::Path& newFilepath, const std::string& oldFilepath);
		void OnCSharpFileRenamed(const Fs::Path& newFilepath, const std::string& oldFilepath);
		void RenderThumbnailSlider();

		SharedReference<Texture2D> FindSuitableItemIcon(const std::filesystem::directory_entry& directoryEntry, const Fs::Path& currentItemPath);
		SharedReference<Texture2D> FindTextureFromAssetManager(const Fs::Path& currentItemPath);
		SharedReference<Texture2D> FindEnvironmentMapFromAssetManager(const Fs::Path& currentItemPath);
		SharedReference<Texture2D> FindMeshIcon(const Fs::Path& extension);

		void UIOnPopupRender(const std::string& currentFilename, const Fs::Path& currentPath);
		void OnConfirmDeletionPopupRender(const std::string& currentFilename, const Fs::Path& currentPath);

	private:
		Fs::Path m_BaseDirectory;
		Fs::Path m_CurrentDirectory;
		Fs::Path m_ItemPathToRename;

		float m_ThumbnailSize = 96.0f;
		float m_ThumbnailPadding = 16.0f;

		bool m_ConfirmDeletionPopupOpen = false;

		ImGuiTextFilter m_SearchInputTextFilter;
	};

}
