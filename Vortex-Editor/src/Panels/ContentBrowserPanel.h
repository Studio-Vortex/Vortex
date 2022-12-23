#pragma once

#include <Vortex.h>

namespace Vortex {

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

		std::unordered_map<std::string, SharedRef<Texture2D>> m_TextureMap;
		ImGuiTextFilter m_SearchInputTextFilter;
	};

}
