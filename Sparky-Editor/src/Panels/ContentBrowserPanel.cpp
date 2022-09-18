#include "ContentBrowserPanel.h"

namespace Sparky {

	extern const std::filesystem::path g_AssetPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(g_AssetPath)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnGuiRender()
	{
		Gui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(g_AssetPath))
		{
			if (Gui::Button("<--"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		// Make sure cached texture icons exist, if they dont remove them from cache
		for (auto it = m_ImageMap.cbegin(), next_it = it; it != m_ImageMap.cend(); it = next_it)
		{
			++next_it;

			if (!std::filesystem::exists((*it).first))
				m_ImageMap.erase(it);
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
			auto relativePath = std::filesystem::relative(path, g_AssetPath);
			std::string filenameString = relativePath.filename().string();

			Gui::PushID(filenameString.c_str());

			SharedRef<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			if (path.extension().string() == ".png" || path.extension().string() == ".jpg")
			{
				if (m_ImageMap.find(path.string()) == m_ImageMap.end())
					m_ImageMap[path.string()] = Texture2D::Create(path.string());

				icon = m_ImageMap[path.string()];
			}

			Gui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
			Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

			if (Gui::BeginDragDropSource())
			{
				const wchar_t* itemPath = relativePath.c_str();
				Gui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				Gui::EndDragDropSource();
			}

			Gui::PopStyleColor();
			if (Gui::IsItemHovered() && Gui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
				{
					m_CurrentDirectory /= path.filename();
				}

			}
			Gui::TextWrapped(filenameString.c_str());

			Gui::NextColumn();

			Gui::PopID();
		}

		Gui::Columns(1);

		Gui::Spacing();
		Gui::Separator();
		Gui::Spacing();

		Gui::SliderFloat("Thumbnail Size", &thumbnailSize, 64.0f, 512.0f);
		Gui::SliderFloat("Padding", &padding, 4.0f, 64.0f);

		Gui::End();
	}

}