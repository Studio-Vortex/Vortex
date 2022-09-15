#include "ContentBrowserPanel.h"

namespace Sparky {

	static const std::filesystem::path s_AssetsPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(s_AssetsPath) { }

	void ContentBrowserPanel::OnGuiRender()
	{
		Gui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(s_AssetsPath))
		{
			if (Gui::Button("<--"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetsPath);
			std::string filenameString = relativePath.filename().string();

			if (directoryEntry.is_directory())
			{
				if (Gui::Button(filenameString.c_str()))
				{
					m_CurrentDirectory /= path.filename();
				}
			}
			else
			{
				if (Gui::Button(filenameString.c_str()))
				{

				}
			}
		}

		Gui::End();
	}

}