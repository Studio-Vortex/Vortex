#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

#include "ContentBrowser/ProjectAssetDirectory.h"
#include "ContentBrowser/Thumbnail/ThumbnailGenerator.h"

namespace Vortex {

	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel(const Fs::Path& assetDir);
		~ContentBrowserPanel() override = default;
		
		void OnGuiRender() override;
		
		VX_FORCE_INLINE SharedReference<ProjectAssetDirectory> GetAssetDirectory() const { return m_AssetDirectory; }
		VX_FORCE_INLINE SharedReference<ThumbnailGenerator> GetThumbnailGenerator() { return m_ThumbnailGenerator; }

		EDITOR_PANEL_TYPE(ContentBrowser)

	private:
		void ProcessAssetDirectory(const Fs::Path& assetDir);

		void RenderCreateItemPopup();
		void RenderFileExplorer();
		void RenderMenuBar();
		void RenderCurrentWorkingDirectory();
		void RenderRightClickItemPopup(const Fs::Path& currentPath);
		void RenameItem(const Fs::Path& currentPath);
		void OnCSharpFileRenamed(const Fs::Path& newFilepath, const std::string& oldFilepath);
		void RenderThumbnailSlider();

		SharedReference<Texture2D> FindSuitableItemIcon(const std::filesystem::directory_entry& directoryEntry, const Fs::Path& currentItemPath);
		SharedReference<Texture2D> FindTextureFromAssetManager(const Fs::Path& currentItemPath);
		SharedReference<Texture2D> FindEnvironmentMapFromAssetManager(const Fs::Path& currentItemPath);
		SharedReference<Texture2D> FindMeshIcon(const Fs::Path& extension);

		void UIOnPopupRender(const std::string& currentFilename, const Fs::Path& currentPath);
		void OnConfirmDeletionPopupRender(const std::string& currentFilename, const Fs::Path& currentPath);

		void RecursiveProcessDirectory(Directory* current) const;
		void ProcessFilesInDirectory(Directory* current) const;

	private:
		Fs::Path m_BaseDirectory;
		Fs::Path m_CurrentDirectory;
		Fs::Path m_ItemPathToRename;

		SharedReference<ProjectAssetDirectory> m_AssetDirectory = nullptr;
		SharedReference<ThumbnailGenerator> m_ThumbnailGenerator = nullptr;

		float m_ThumbnailSize = 96.0f;
		float m_ThumbnailPadding = 16.0f;

		bool m_ConfirmDeletionPopupOpen = false;
		bool m_ShowCreateItemMenu = false;

		ImGuiTextFilter m_SearchInputTextFilter;
	};

}
