#pragma once

#include <Sparky.h>

namespace Sparky {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		
		void OnGuiRender();

	private:
		std::filesystem::path m_CurrentDirectory;

		SharedRef<Texture2D> m_DirectoryIcon;
		SharedRef<Texture2D> m_FileIcon;
	};

}
