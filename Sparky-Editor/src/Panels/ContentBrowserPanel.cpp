#include "ContentBrowserPanel.h"

#include "Sparky/Utils/PlatformUtils.h"

#include <imgui_internal.h>

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
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		Gui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(g_AssetPath))
		{
			if (Gui::Button(" <-- "))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();

			Gui::SameLine();
		}

		if (Gui::Button(" + "))
			Gui::OpenPopup("CreateList");

		if (Gui::BeginPopup("CreateList"))
		{
			if (Gui::MenuItem("New Folder"))
			{
				std::filesystem::create_directory(m_CurrentDirectory / std::filesystem::path("New Folder"));
				Gui::CloseCurrentPopup();
			}
			Gui::Separator();

			if (Gui::MenuItem("New Scene"))
			{
				std::ofstream newSceneFile(m_CurrentDirectory / std::filesystem::path("Untitled.sparky"));
				newSceneFile << "Scene: Untitled\nEntities:";
				newSceneFile.close();

				Gui::CloseCurrentPopup();
			}

			Gui::EndPopup();
		}

		// Make sure cached texture icons exist, if they dont remove them from cache
		for (auto it = m_TextureMap.cbegin(), next_it = it; it != m_TextureMap.cend(); it = next_it)
		{
			++next_it;

			if (!std::filesystem::exists(it->first))
				m_TextureMap.erase(it);
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
				if (m_TextureMap.find(path.string()) == m_TextureMap.end())
					m_TextureMap[path.string()] = Texture2D::Create(path.string());

				icon = m_TextureMap[path.string()];
			}

			static bool renameDirectoryEntry = false;
			static bool openPopup = false;

			Gui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
			Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
			Gui::PopStyleColor();

			// Right-click on item for file utilities popup
			if (Gui::IsItemHovered() && Gui::IsItemClicked(ImGuiMouseButton_Right))
				Gui::OpenPopup("FileUtilities");

			if (Gui::BeginPopup("FileUtilities"))
			{
				if (Gui::MenuItem("Rename"))
				{
					renameDirectoryEntry = true;
					Gui::CloseCurrentPopup();
				}
				Gui::Separator();

				if (Gui::MenuItem("Open in File Explorer"))
				{
					FileSystem::OpenDirectory(m_CurrentDirectory.string().c_str());
					Gui::CloseCurrentPopup();
				}
				Gui::Separator();

				static bool deleteEntry = false;

				if (Gui::MenuItem("Delete"))
				{
					if (directoryEntry.is_directory())
					{
						SP_CORE_WARN("Directory to be deleted: {}/", path);
						// TODO: Delete directory with some form of confirmation
						Gui::CloseCurrentPopup();
					}
					else
					{
						std::filesystem::remove(path); // ALSO CONFIRM HERE
						Gui::CloseCurrentPopup();
					}
				}

				Gui::EndPopup();
			}

			if (renameDirectoryEntry)
			{
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));

				if (Gui::InputText("##RenameInputText", buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					renameDirectoryEntry = false;

					if (strlen(buffer) != 0)
						std::filesystem::rename(path, m_CurrentDirectory / std::filesystem::path(buffer));
				}
			}

			if (Gui::BeginDragDropSource())
			{
				const wchar_t* itemPath = relativePath.c_str();
				Gui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				Gui::EndDragDropSource();
			}

			// Double click to enter into a directory
			if (Gui::IsItemHovered() && Gui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();
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

		Gui::ShowDemoWindow();

		Gui::End();
	}

}
