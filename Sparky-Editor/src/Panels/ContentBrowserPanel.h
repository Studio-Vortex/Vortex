#pragma once

#include <Sparky.h>

namespace Sparky {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		
		void OnGuiRender();

		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderRightClickPopupMenu();
		void RenderFileExplorer();
		void RenderSlider(float& thumbnailSize, float& padding);

	private:
		inline static bool s_ShowPanel = true;

	private:
		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_PathToBeRenamed;

		SharedRef<Texture2D> m_DirectoryIcon = nullptr;
		SharedRef<Texture2D> m_AudioFileIcon = nullptr;
		SharedRef<Texture2D> m_OBJIcon = nullptr;
		SharedRef<Texture2D> m_CodeFileIcon = nullptr;
		SharedRef<Texture2D> m_SceneIcon = nullptr;
		SharedRef<Texture2D> m_FileIcon = nullptr;

		std::unordered_map<std::string, SharedRef<Texture2D>> m_TextureMap;
		ImGuiTextFilter m_SearchInputTextFilter;
	};

}
