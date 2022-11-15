#include "ContentBrowserPanel.h"

#include "Sparky/Utils/PlatformUtils.h"

#include <imgui_internal.h>

namespace Sparky {

	extern const std::filesystem::path g_AssetPath = "Assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(g_AssetPath)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_AudioFileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/AudioFileIcon.png");
		m_ModelFileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/ModelFileIcon.png");
		m_CodeFileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/CodeFileIcon.png");
		m_SceneIcon = Texture2D::Create("Resources/Icons/ContentBrowser/SceneIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnGuiRender()
	{
		if (s_ShowPanel)
		{
			Gui::Begin("Content Browser", &s_ShowPanel);

			// Left
			float directoryButtonsContainierWidth = std::max(Gui::GetWindowContentRegionWidth() * 0.15f, 145.0f);
			Gui::BeginChild("Left Pane", ImVec2(directoryButtonsContainierWidth, 0), false);

			Gui::TextCentered(g_AssetPath.string().c_str(), 5.0f);
			Gui::SetCursorPosY(28.5f);
			Gui::Separator();

			for (auto& assetDirectoryEntry : std::filesystem::directory_iterator(g_AssetPath))
			{
				if (!assetDirectoryEntry.is_directory())
					continue;

				if (Gui::Button(assetDirectoryEntry.path().filename().string().c_str(), ImVec2{ Gui::GetContentRegionAvail().x, 0.0f }))
				{
					// Clear the search input text so it does not interfere with the child directory
					memset(m_SearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
					m_SearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed

					m_CurrentDirectory = assetDirectoryEntry.path();
				}
			}
			Gui::EndChild();

			Gui::SameLine();

			// Right
			Gui::BeginGroup();
			Gui::BeginChild("Right Pane", ImVec2(0, Gui::GetContentRegionAvail().y));

			RenderFileExplorer();

			Gui::EndChild();
			Gui::EndGroup();

			Gui::End();
		}
	}

	void ContentBrowserPanel::RenderRightClickPopupMenu()
	{
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
	}

	void ContentBrowserPanel::RenderFileExplorer()
	{
		ImGuiIO& io = Gui::GetIO();
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
			memset(m_SearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
			m_SearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed

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
		bool isSearching = Gui::InputTextWithHint("##AssetSearch", "Search", m_SearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
		if (isSearching)
			m_SearchInputTextFilter.Build();

		Gui::Spacing();
		Gui::Separator();

		static float padding = 16.0f;
		static float thumbnailSize = 96.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = Gui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;
		
		//                                       leave some space for the icon size tools
		Gui::BeginChild("Directories", ImVec2(0, Gui::GetContentRegionAvail().y - 68.0f), false);

		Gui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& currentPath = directoryEntry.path();
			auto relativePath = std::filesystem::relative(currentPath, g_AssetPath);
			std::string filenameString = relativePath.filename().string();
			bool skipDirectoryEntry = false;

			if (!m_SearchInputTextFilter.PassFilter(relativePath.string().c_str())) // If the search box text doesn't match up we can skip the directory entry
				skipDirectoryEntry = true;

			if (skipDirectoryEntry)
				continue;

			Gui::PushID(filenameString.c_str());

			SharedRef<Texture2D> icon = m_FileIcon;

			// File icons

			if (directoryEntry.is_directory())
				icon = m_DirectoryIcon;
			else if (currentPath.extension().string() == ".sparky")
				icon = m_SceneIcon;
			else if (currentPath.extension().string() == ".cs")
				icon = m_CodeFileIcon;
			else if (currentPath.extension().string() == ".obj")
				icon = m_ModelFileIcon;
			else if (currentPath.extension().string() == ".wav" || currentPath.extension().string() == ".mp3")
				icon = m_AudioFileIcon;
			else if (currentPath.extension().string() == ".png" || currentPath.extension().string() == ".jpg" || currentPath.extension().string() == ".tga")
			{
				if (m_TextureMap.find(currentPath.string()) == m_TextureMap.end())
					m_TextureMap[currentPath.string()] = Texture2D::Create(currentPath.string());

				icon = m_TextureMap[currentPath.string()];
			}

			Gui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
			Gui::ImageButton((void*)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
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
					FileSystem::OpenFileExplorer(m_CurrentDirectory.string().c_str());
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

			if (Gui::IsPopupOpen("Confirm"))
			{
				Math::vec2 applicationWindowSize = Application::Get().GetWindow().GetSize();
				ImVec2 confirmWindowSize = { 500, 200 };
				Gui::SetNextWindowSize(confirmWindowSize, ImGuiCond_Always);
				Gui::SetNextWindowPos({ (applicationWindowSize.x / 2.0f) - (confirmWindowSize.x / 2.0f), (applicationWindowSize.y / 2.0f) - (confirmWindowSize.y / 2.0f) }, ImGuiCond_Always);
			}
			
			if (Gui::BeginPopupModal("Confirm", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				Gui::Separator();
				Gui::Spacing();

				ImVec2 button_size(Gui::GetFontSize() * 8.65f, 0.0f);

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
				Gui::SetNextItemWidth(thumbnailSize);
				if (Gui::InputText("##RenameInputText", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					bool inputTextWasNotEmpty = strlen(buffer) != 0;

					if (inputTextWasNotEmpty && (m_CurrentDirectory / std::filesystem::path(buffer)) != currentPath)
					{
						// Get the new path from the input text buffer relative to the current directory
						std::filesystem::path newFilePath = m_CurrentDirectory / std::filesystem::path(buffer);
						
						// Rename the current path to the new path
						std::filesystem::rename(currentPath, newFilePath);

						// Rename C# Class name in file
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
										// Modify the class name line
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

						// Rename AudioSourceComponent Source path
						if (newFilePath.filename().extension() == ".wav" || newFilePath.filename().extension() == ".mp3")
						{
							// TODO: Once we have an asset system, ask the asset system to rename an asset here otherwise the engine could crash by loading a non-existant file
							// we have the old filename
							//oldFilenameWithExtension
							// we also have the newFilePath
						}

						// Rename SpriteRendererComponent Texture path
						if (newFilePath.filename().extension() == ".png" || newFilePath.filename().extension() == ".jpg" || newFilePath.filename().extension() == ".tga")
						{
							// TODO: Once we have an asset system, ask the asset system to rename an asset here otherwise the engine could crash by loading a non-existant file
							// we have the old filename
							//oldFilenameWithExtension
							// we also have the newFilePath
						}
					}

					m_PathToBeRenamed = "";
				}
			}

			// Drag items from the content browser to else-where in the editor
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
					memset(m_SearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
					m_SearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed
				}
			}

			// If we are not renaming the current entry we can show the path
			if (currentPath != m_PathToBeRenamed)
			{
				size_t dotPos = filenameString.find('.');
				std::string filenameWithoutExtension = filenameString.substr(0, dotPos);
				Gui::TextWrapped(filenameWithoutExtension.c_str());
			}

			Gui::NextColumn();
			Gui::PopID();
		}

		Gui::Columns(1);

		RenderRightClickPopupMenu();

		Gui::EndChild();

		// Accept a Prefab from the scene hierarchy
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
			{
				UUID uuid = *(UUID*)payload->Data;
				SP_CORE_INFO("UUID: {}", uuid);
				// Todo Create a new prefab here from the entity's uuid
			}
			Gui::EndDragDropTarget();
		}

		Gui::Spacing();
		Gui::Separator();
		Gui::Spacing();

		Gui::SliderFloat("Thumbnail Size", &thumbnailSize, 64.0f, 512.0f, "%.0f");
		static float newPadding = padding / 4.0f;
		if (Gui::SliderFloat("Padding", &newPadding, 1.0f, 16.0f, "%.0f"))
			padding = newPadding * 4.0f;
	}

}
