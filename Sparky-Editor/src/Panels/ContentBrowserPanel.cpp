#include "ContentBrowserPanel.h"

namespace Sparky {

	static const std::filesystem::path s_AssetsPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(s_AssetsPath)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

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

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = Gui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;
		Gui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetsPath);
			std::string filenameString = relativePath.filename().string();

			SharedRef<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
			if (Gui::IsItemHovered() && Gui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
				{
					m_CurrentDirectory /= path.filename();
				}

			}
			Gui::TextWrapped(filenameString.c_str());

			Gui::NextColumn();
		}

		Gui::Columns(1);

		Gui::SliderFloat("Thumbnail Size", &thumbnailSize, 32.0f, 512.0f);
		Gui::SliderFloat("Padding", &padding, 0.0f, 32.0f);

		Gui::End();
	}

}