#include "ContentBrowserPanel.h"

#include "Sparky/Utils/PlatformUtils.h"

#include <imgui_internal.h>

namespace Sparky {

	extern const std::filesystem::path g_AssetPath = "Assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(g_AssetPath)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnGuiRender()
	{
		Gui::Begin("Content Browser");
		// Left
		static uint32_t selectedSetting = 0;
		Gui::BeginChild("Left Pane", ImVec2(150, 0), true);
		Gui::TextCentered(g_AssetPath.string().c_str(), 5.0f);
		Gui::Separator();
		for (auto& assetDirectoryFolder : std::filesystem::directory_iterator(g_AssetPath))
		{
			if (Gui::Button(assetDirectoryFolder.path().filename().string().c_str(), ImVec2{ Gui::GetContentRegionAvail().x, 0.0f }))
			{
				// Clear the search input text so it does not interfere with the child directory
				memset(m_InputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_InputTextFilter.InputBuf));
				m_InputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed

				m_CurrentDirectory = assetDirectoryFolder.path();
			}
		}
		Gui::EndChild();

		Gui::SameLine();

		// Right
		Gui::BeginGroup();
		Gui::BeginChild("Right Pane", ImVec2(0, Gui::GetContentRegionAvail().y));

		// Right-click on blank space in content browser panel
		if (Gui::BeginPopupContextWindow(0, 1, false))
		{
			if (Gui::MenuItem("New Folder"))
			{
				m_PathToBeRenamed = m_CurrentDirectory / std::filesystem::path("New Folder");
				std::filesystem::create_directory(m_PathToBeRenamed);
				Gui::CloseCurrentPopup();
			}
			Gui::Separator();

			if (Gui::MenuItem("New Scene"))
			{
				m_PathToBeRenamed = m_CurrentDirectory / std::filesystem::path("Untitled.sparky");
				std::ofstream newSceneFile(m_PathToBeRenamed);
				newSceneFile << "Scene: Untitled\nEntities:";
				newSceneFile.close();

				Gui::CloseCurrentPopup();
			}

			Gui::EndPopup();
		}

		RenderFileExplorer();
		Gui::EndChild();
		Gui::EndGroup();

		Gui::End();
	}

	void ContentBrowserPanel::RenderFileExplorer()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		// Make sure cached texture icons exist, if they dont remove them from cache
		for (auto it = m_TextureMap.cbegin(), next_it = it; it != m_TextureMap.cend(); it = next_it)
		{
			++next_it;

			if (!std::filesystem::exists(it->first))
				m_TextureMap.erase(it);
		}

		Gui::BeginDisabled(m_CurrentDirectory == std::filesystem::path(g_AssetPath));
		float originalFrameRounding = Gui::GetStyle().FrameRounding;
		Gui::GetStyle().FrameRounding = 5.0f;
		if (Gui::Button("  <--  "))
		{
			// Clear the search input text so it does not interfere with the parent directory
			memset(m_InputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_InputTextFilter.InputBuf));
			m_InputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed

			m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}
		Gui::GetStyle().FrameRounding = originalFrameRounding;
		Gui::EndDisabled();

		Gui::SameLine();
		Gui::Text(m_CurrentDirectory.string().c_str());

		// Search Bar + Filtering
		float inputTextSize = Gui::GetWindowWidth() / 2.0f - Gui::CalcTextSize(m_CurrentDirectory.string().c_str()).x;
		Gui::SetCursorPos({ Gui::GetContentRegionAvail().x - inputTextSize, -3.0f });
		Gui::SetNextItemWidth(inputTextSize);
		bool isSearching = Gui::InputTextWithHint("##Search", "Search", m_InputTextFilter.InputBuf, IM_ARRAYSIZE(m_InputTextFilter.InputBuf));
		if (isSearching)
			m_InputTextFilter.Build();

		Gui::Spacing();
		Gui::Separator();

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
			bool skipDirectoryEntry = false;

			if (!m_InputTextFilter.PassFilter(relativePath.string().c_str())) // If the search box text doesn't match up we can skip the directory entry
				skipDirectoryEntry = true;

			if (skipDirectoryEntry)
				continue;

			Gui::PushID(filenameString.c_str());

			SharedRef<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			if (path.extension().string() == ".png" || path.extension().string() == ".jpg")
			{
				if (m_TextureMap.find(path.string()) == m_TextureMap.end())
					m_TextureMap[path.string()] = Texture2D::Create(path.string());

				icon = m_TextureMap[path.string()];
			}

			Gui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
			Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
			Gui::PopStyleColor();

			static bool confirmDeletionPopupOpen = false;

			// Right-click on directory or file for utilities popup
			if (Gui::BeginPopupContextItem())
			{
				if (Gui::MenuItem("Rename"))
				{
					m_PathToBeRenamed = path;
					Gui::CloseCurrentPopup();
				}
				Gui::Separator();

				if (Gui::MenuItem("Open in File Explorer"))
				{
					FileSystem::OpenDirectory(m_CurrentDirectory.string().c_str());
					Gui::CloseCurrentPopup();
				}
				Gui::Separator();

				if (Gui::MenuItem("Delete"))
				{
					confirmDeletionPopupOpen = true;
					Gui::CloseCurrentPopup();
				}

				Gui::EndPopup();
			}

			if (confirmDeletionPopupOpen)
			{
				Gui::OpenPopup("Confirm");
				confirmDeletionPopupOpen = false;
			}

			ImVec2 windowSize = { 500, 200 };
			Gui::SetNextWindowSize(windowSize);
			Gui::SetNextWindowPos({ (io.DisplaySize.x / 2.0f) - (windowSize.x / 2.0f), (io.DisplaySize.y / 2.0f) - (windowSize.y / 2.0f) });
			ImVec2 button_size(Gui::GetFontSize() * 8.65f, 0.0f);
			
			if (Gui::BeginPopupModal("Confirm", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				Gui::Separator();
				Gui::Spacing();

				Gui::Text("Are you sure you want to permanently delete '%s' ?", path.filename().string().c_str());
				Gui::Text("This cannot be undone.");

				Gui::Spacing();
				Gui::Separator();

				for (uint32_t i = 0; i < 18; i++)
					Gui::Spacing();

				if (Gui::Button("Yes", button_size))
				{
					std::filesystem::remove(path);
					Gui::CloseCurrentPopup();
				}

				Gui::SameLine();

				if (Gui::Button("No", button_size))
					Gui::CloseCurrentPopup();

				Gui::SameLine();

				if (Gui::Button("Cancel", button_size))
					Gui::CloseCurrentPopup();

				Gui::Spacing();
				Gui::EndPopup();
			}

			if (path == m_PathToBeRenamed)
			{
				char buffer[256];
				size_t pos = path.string().find_last_of('\\');
				std::string pathToCopy = path.string().substr(pos + 1, path.string().length());
				memcpy(buffer, pathToCopy.c_str(), sizeof(buffer));

				Gui::SetKeyboardFocusHere();
				if (Gui::InputText("##RenameInputText", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (strlen(buffer) != 0)
						std::filesystem::rename(path, m_CurrentDirectory / std::filesystem::path(buffer));

					m_PathToBeRenamed = "";
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
				{
					m_CurrentDirectory /= path.filename();

					// We also need to reset the search input text here
					memset(m_InputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_InputTextFilter.InputBuf));
					m_InputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed
				}
			}

			// If we are not renaming the current entry we can show the path
			if (path != m_PathToBeRenamed)
				Gui::TextWrapped(filenameString.c_str());

			Gui::NextColumn();
			Gui::PopID();
		}

		Gui::Columns(1);

		Gui::Spacing();
		Gui::Separator();
		Gui::Spacing();

		Gui::SliderFloat("Thumbnail Size", &thumbnailSize, 64.0f, 512.0f, "%.0f");
		static float newPadding = padding / 4.0f;
		if (Gui::SliderFloat("Padding", &newPadding, 1.0f, 16.0f, "%.0f"))
			padding = newPadding * 4.0f;
	}

}
