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
		for (auto& assetDirectoryEntry : std::filesystem::directory_iterator(g_AssetPath))
		{
			if (!assetDirectoryEntry.is_directory())
				continue;

			if (Gui::Button(assetDirectoryEntry.path().filename().string().c_str(), ImVec2{ Gui::GetContentRegionAvail().x, 0.0f }))
			{
				// Clear the search input text so it does not interfere with the child directory
				memset(m_InputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_InputTextFilter.InputBuf));
				m_InputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed

				m_CurrentDirectory = assetDirectoryEntry.path();
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
			if (Gui::BeginMenu("Create"))
			{
				if (Gui::MenuItem("Folder"))
				{
					m_PathToBeRenamed = m_CurrentDirectory / std::filesystem::path("New Folder");
					std::filesystem::create_directory(m_PathToBeRenamed);
					Gui::CloseCurrentPopup();
				}
				Gui::Separator();

				if (Gui::MenuItem("Scene"))
				{
					m_PathToBeRenamed = m_CurrentDirectory / std::filesystem::path("Untitled.sparky");
					std::ofstream fout(m_PathToBeRenamed);
					fout << "Scene: Untitled\nEntities:";
					fout.close();

					Gui::CloseCurrentPopup();
				}
				Gui::Separator();

				if (Gui::MenuItem("C# Script"))
				{
					m_PathToBeRenamed = m_CurrentDirectory / std::filesystem::path("Untitled.cs");
					std::ofstream fout(m_PathToBeRenamed);
					fout << R"(using System;
using Sparky;

public class Untitled : Entity
{
	public override void OnCreate()
	{
		// Called once before the first frame
	}

	public override void OnUpdate(float delta)
	{
		// Called once every frame
	}
}
)";

					fout.close();

					Gui::CloseCurrentPopup();
				}

				Gui::EndMenu();
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
			const auto& currentPath = directoryEntry.path();
			auto relativePath = std::filesystem::relative(currentPath, g_AssetPath);
			std::string filenameString = relativePath.filename().string();
			bool skipDirectoryEntry = false;

			if (!m_InputTextFilter.PassFilter(relativePath.string().c_str())) // If the search box text doesn't match up we can skip the directory entry
				skipDirectoryEntry = true;

			if (skipDirectoryEntry)
				continue;

			Gui::PushID(filenameString.c_str());

			SharedRef<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			if (currentPath.extension().string() == ".png" || currentPath.extension().string() == ".jpg")
			{
				if (m_TextureMap.find(currentPath.string()) == m_TextureMap.end())
					m_TextureMap[currentPath.string()] = Texture2D::Create(currentPath.string());

				icon = m_TextureMap[currentPath.string()];
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
					m_PathToBeRenamed = currentPath;
					Gui::CloseCurrentPopup();
				}
				Gui::Separator();

				bool isCSharpFile = currentPath.filename().extension() == ".cs";

				if (isCSharpFile && Gui::MenuItem("Open with VsCode"))
				{
					FileSystem::LaunchApplication("code", std::format("{} {}", currentPath.parent_path().string(), currentPath.string()).c_str());
					Gui::CloseCurrentPopup();
				}
				if (isCSharpFile)
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

				Gui::Text("Are you sure you want to permanently delete '%s' ?", currentPath.filename().string().c_str());
				Gui::Text("This cannot be undone.");

				Gui::Spacing();
				Gui::Separator();

				for (uint32_t i = 0; i < 18; i++)
					Gui::Spacing();

				if (Gui::Button("Yes", button_size))
				{
					std::filesystem::remove(currentPath);
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

			if (currentPath == m_PathToBeRenamed)
			{
				// Find the last backslash in the path and copy the filename to the buffer
				char buffer[256];
				size_t pos = currentPath.string().find_last_of('\\');
				std::string oldFilenameWithExtension = currentPath.string().substr(pos + 1, currentPath.string().length());
				memcpy(buffer, oldFilenameWithExtension.c_str(), sizeof(buffer));

				Gui::SetKeyboardFocusHere();
				if (Gui::InputText("##RenameInputText", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (strlen(buffer) != 0 && (m_CurrentDirectory / std::filesystem::path(buffer)) != currentPath)
					{
						std::filesystem::path newFilePath = m_CurrentDirectory / std::filesystem::path(buffer);
						std::filesystem::rename(currentPath, newFilePath);

						// Rename C# file
						if (newFilePath.filename().extension() == ".cs")
						{
							std::ifstream cSharpScriptFile(newFilePath);
							 
							if (cSharpScriptFile.is_open())
							{
								// Make sure we start at the beginning of the file
								cSharpScriptFile.seekg(0);

								std::vector<std::string> fileLineBuffer;
								std::string currentLine;

								// Get the old class name from the old filename - i.e. "Player.cs" -> "Player"
								std::string oldClassName = oldFilenameWithExtension.substr(0, oldFilenameWithExtension.find_last_of('.'));
								bool classNameFound = false;

								while (std::getline(cSharpScriptFile, currentLine))
								{
									fileLineBuffer.push_back(currentLine);

									if (currentLine.find(oldClassName) != std::string::npos)
										classNameFound = true;
								}

								cSharpScriptFile.close();

								SP_CORE_ASSERT(classNameFound, "C# Class Name was not the same as filename!");

								// Find the line with the class name and replace it
								for (auto& line : fileLineBuffer)
								{
									if (line.find(oldClassName) != std::string::npos)
									{
										// Get the new name of the file to rename the C# class
										std::string newClassName = newFilePath.filename().string().substr(0, newFilePath.filename().string().find_first_of('.'));
										std::string formattedLine = std::format("public class {} : Entity", newClassName);
										line = formattedLine;

										// Replace the contents of the file
										std::ofstream fout(newFilePath, std::ios::trunc);

										for (auto& editedLine : fileLineBuffer)
											fout << editedLine << '\n';

										// Close the edited file
										fout.close();

										// Since we edited the file already we can skip the rest of the lines;
										break;
									}
								}
							}
						}
					}

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
					m_CurrentDirectory /= currentPath.filename();

					// We also need to reset the search input text here
					memset(m_InputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_InputTextFilter.InputBuf));
					m_InputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed
				}
			}

			// If we are not renaming the current entry we can show the path
			if (currentPath != m_PathToBeRenamed)
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
