#pragma once

#include <Sparky.h>

namespace Sparky {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		
		void OnGuiRender();

	private:
		void RenderRightClickPopupMenu();
		void RenderFileExplorer();

	private:
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_PathToBeRenamed;

		SharedRef<Texture2D> m_DirectoryIcon;
		SharedRef<Texture2D> m_AudioFileIcon;
		SharedRef<Texture2D> m_CodeFileIcon;
		SharedRef<Texture2D> m_SceneIcon;
		SharedRef<Texture2D> m_FileIcon;
		std::unordered_map<std::string, SharedRef<Texture2D>> m_TextureMap;
		ImGuiTextFilter m_SearchInputTextFilter;
	};

}
