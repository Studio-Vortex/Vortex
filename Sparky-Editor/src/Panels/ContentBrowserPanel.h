#pragma once

#include <Sparky.h>

#include <filesystem>

namespace Sparky {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		
		void OnGuiRender();

	private:
		std::filesystem::path m_CurrentDirectory;
	};

}
