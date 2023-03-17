#pragma once

#include <Vortex.h>

namespace Vortex {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;
		
		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(const SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderCreateItemPopup();
		void RenderFileExplorer();
		void RenderMenuBar();
		void RenderRightClickItemPopup(const std::filesystem::path& currentPath);
		void RenderThumbnailSlider();
		SharedReference<Texture2D> FindSuitableItemIcon(const std::filesystem::directory_entry& directoryEntry, const std::filesystem::path& currentItemPath, const std::filesystem::path& extension);
		void FindTextureFromAssetManager(const std::filesystem::path& currentItemPath, SharedReference<Texture2D>& itemIcon);
		void FindMeshIcon(const std::filesystem::path& extension, SharedReference<Texture2D>& itemIcon);

	private:
		inline static bool s_ShowPanel = true;
		float m_ThumbnailSize = 96.0f;
		float m_ThumbnailPadding = 16.0f;

	private:
		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_ItemPathToRename;

		ImGuiTextFilter m_SearchInputTextFilter;
	};

}
