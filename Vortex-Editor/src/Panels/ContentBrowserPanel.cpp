#include "ContentBrowserPanel.h"

#include <Vortex/Editor/EditorResources.h>

#include <imgui_internal.h>

namespace Vortex {

	ContentBrowserPanel::ContentBrowserPanel(const Fs::Path& assetDir)
		: m_BaseDirectory(assetDir), m_CurrentDirectory(m_BaseDirectory) { }

	void ContentBrowserPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		// Left
		float directoryContainerWidth = std::max(Gui::GetWindowContentRegionWidth() * 0.15f, 165.0f);
		Gui::BeginChild("Left Pane", ImVec2(directoryContainerWidth, 0));

		if (UI::TreeNode(Project::GetActive()->GetName().c_str()))
		{
			Gui::Unindent();

			for (const auto& assetDirectoryEntry : std::filesystem::directory_iterator(m_BaseDirectory))
			{
				if (!assetDirectoryEntry.is_directory())
					continue;

				const ImGuiTreeNodeFlags directoryTreeNodeFlags = ((m_CurrentDirectory == assetDirectoryEntry.path()) ? ImGuiTreeNodeFlags_Selected : 0) |
					ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

				if (Gui::TreeNodeEx(assetDirectoryEntry.path().filename().string().c_str(), directoryTreeNodeFlags))
				{
					for (const auto& directoryEntry : std::filesystem::directory_iterator(assetDirectoryEntry))
					{
						const std::string& path = directoryEntry.path().filename().string();

						Gui::Selectable(path.c_str());
					}

					Gui::TreePop();
				}
				else if (Gui::IsItemClicked(ImGuiMouseButton_Left))
				{
					// Clear the search input text so it does not interfere with the child directory
					memset(m_SearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
					m_SearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed

					m_CurrentDirectory = assetDirectoryEntry.path();
				}
			}

			Gui::TreePop();
			Gui::Indent();
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

	void ContentBrowserPanel::RenderCreateItemPopup()
	{
		// Right-click on blank space in content browser panel
		if (Gui::BeginPopupContextWindow(0, 1, false))
		{
			if (Gui::BeginMenu("Create"))
			{
				if (Gui::MenuItem("Folder"))
				{
					m_ItemPathToRename = m_CurrentDirectory / Fs::Path("New Folder");
					FileSystem::CreateDirectoryV(m_ItemPathToRename);

					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Scene"))
				{
					std::ofstream fout(m_ItemPathToRename);
					VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");
					fout << "Scene: Untitled\nEntities:";
					fout.close();

					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Material"))
				{
					const std::string filename = "NewMaterial.vmaterial";
					SharedReference<Shader> shader = Renderer::GetShaderLibrary().Get("PBR_Static");
					MaterialProperties properties = MaterialProperties();
					SharedReference<Material> material = Project::GetEditorAssetManager()->CreateNewAsset<Material>("Materials", filename, shader, properties);
					VX_CORE_ASSERT(material, "Failed to create material!");

					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("C# Script"))
				{
					std::ofstream fout(m_ItemPathToRename);
					fout << R"(using Vortex;

public class Untitled : Actor
{
	// Called once before the first frame or when the Actor is Instantiated into the Scene
	void OnCreate()
	{
		
	}

	// Called once every frame
	void OnUpdate()
	{
		
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

		RenderMenuBar();

		Gui::Spacing();
		UI::Draw::Underline();

		float cellSize = m_ThumbnailSize + m_ThumbnailPadding;

		float panelWidth = Gui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;
		
		//                                                leave some space for the thumbnail size slider
		Gui::BeginChild("##FileExplorer", ImVec2(0, Gui::GetContentRegionAvail().y - 45.0f), false);

		Gui::Columns(columnCount, 0, false);

		for (const auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& currentPath = directoryEntry.path();
			const Fs::Path relativePath = currentPath;
			const std::string filenameString = relativePath.filename().string();
			const bool skipDirectoryEntry = !m_SearchInputTextFilter.PassFilter(relativePath.string().c_str());
			const bool isHiddenFile = !directoryEntry.is_directory()
				&& !Project::GetEditorAssetManager()->IsValidAssetExtension(FileSystem::GetFileExtension(currentPath));

			if (skipDirectoryEntry || isHiddenFile)
				continue;

			Gui::PushID(filenameString.c_str());

			const SharedReference<Texture2D> itemIcon = FindSuitableItemIcon(directoryEntry, currentPath);

			Gui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
			UI::ImageButtonEx(itemIcon, { m_ThumbnailSize, m_ThumbnailSize }, { 0, 0, 0, 0 }, { 1, 1, 1, 1 });
			Gui::PopStyleColor();

			RenderRightClickItemPopup(currentPath);

			// Drag items from the content browser to else-where in the editor
			if (Gui::BeginDragDropSource())
			{
				const wchar_t* itemPath = relativePath.c_str();
				Gui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				Gui::BeginTooltip();

				const SharedReference<Texture2D> icon = FindSuitableItemIcon(directoryEntry, currentPath);

				UI::ImageEx(icon, { 12.0f, 12.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
				Gui::SameLine();

				const std::string fullPath = directoryEntry.path().relative_path().string();
				const size_t lastSlashPos = fullPath.find_last_of("/\\") + 1;
				const std::string filenameWithExtension = fullPath.substr(lastSlashPos);
				const std::string filename = FileSystem::RemoveFileExtension(filenameWithExtension);
				Gui::Text("%s", filename.c_str());

				Gui::EndTooltip();
				Gui::EndDragDropSource();
			}

			// Double click to enter into a directory or open the file
			if (Gui::IsItemHovered() && Gui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
				{
					m_CurrentDirectory /= currentPath.filename();

					memset(m_SearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
					m_SearchInputTextFilter.Build();
				}
				else
				{
					FileDialogue::OpenInFileExplorer(currentPath.string().c_str());
				}
			}

			// If we are not renaming the current entry we can show the path
			if (currentPath != m_ItemPathToRename)
			{
				const size_t dotPos = filenameString.find('.');
				const std::string filenameWithoutExtension = filenameString.substr(0, dotPos);
				Gui::TextWrapped(filenameWithoutExtension.c_str());
			}

			Gui::NextColumn();
			Gui::PopID();
		}

		Gui::Columns(1);

		RenderCreateItemPopup();

		Gui::EndChild();

		// Accept a Prefab from the scene hierarchy
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
			{
				const Actor& droppedActor = *(Actor*)payload->Data;
				VX_CONSOLE_LOG_INFO("Dropped Actor Name: {}", droppedActor.GetName());
				// Todo Create a new prefab here from the Actor's uuid
				//SharedRef<Prefab> prefab = Prefab::Create((Project::GetProjectDirectory() / droppedActor.GetName() / ".vprefab"));
				SharedRef<Prefab> prefab = Prefab::Create(droppedActor);
			}
			Gui::EndDragDropTarget();
		}

		Gui::Spacing();
		UI::Draw::Underline();
		Gui::Spacing();

		RenderThumbnailSlider();
	}

	void ContentBrowserPanel::RenderMenuBar()
	{
		Gui::BeginDisabled(m_CurrentDirectory == Project::GetAssetDirectory());
		if (Gui::Button((const char*)VX_ICON_CHEVRON_LEFT, { 45, 0 }))
		{
			// Clear the search input text so it does not interfere with the parent directory
			memset(m_SearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
			m_SearchInputTextFilter.Build(); // We also need to rebuild to search results because the buffer has changed

			m_CurrentDirectory = FileSystem::GetParentDirectory(m_CurrentDirectory);
		}
		UI::SetTooltip("Back");
		Gui::EndDisabled();

		Gui::SameLine();

		const Fs::Path fullyQualifiedAssetDirectory = m_BaseDirectory;
		const size_t lastSlashPos = fullyQualifiedAssetDirectory.string().find_last_of("/\\") + 1;
		const std::string relativeProjectAssetDirectory = fullyQualifiedAssetDirectory.string().substr(lastSlashPos, fullyQualifiedAssetDirectory.string().size());

		if (FileSystem::Equivalent(m_CurrentDirectory, Project::GetAssetDirectory()))
		{
			Gui::Text(relativeProjectAssetDirectory.c_str());
		}
		else
		{
			if (Gui::Button(relativeProjectAssetDirectory.c_str()))
			{
				m_CurrentDirectory = m_BaseDirectory;
			}
		}

		Gui::SameLine();

		Fs::Path relativePath = FileSystem::Relative(m_CurrentDirectory, m_BaseDirectory);
		std::vector<std::string> splitPath = String::SplitString(relativePath.string(), "/\\");

		if (splitPath[0] != ".")
		{
			UI::ShiftCursorY(1.0f);
			Gui::Text((const char*)VX_ICON_CHEVRON_RIGHT);
			Gui::SameLine();
		}

		const size_t numPaths = splitPath.size();
		uint32_t i = 0;

		for (const auto& entry : splitPath)
		{
			if (entry == ".")
				continue;

			const std::string label = entry + "##" + std::to_string(i);
			if (Gui::Button(label.c_str()))
			{
				// TODO fix this
				// currently crashes when you click any of these buttons
				if (!FileSystem::Equivalent(m_CurrentDirectory, entry))
				{
					m_CurrentDirectory = FileSystem::Relative(entry, m_BaseDirectory);
				}

				memset(m_SearchInputTextFilter.InputBuf, 0, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
				m_SearchInputTextFilter.Build();
			}

			Gui::SameLine();

			if (numPaths - 1 == i++)
			{
				break;
			}

			UI::ShiftCursorY(1.0f);
			Gui::Text((const char*)VX_ICON_CHEVRON_RIGHT);

			Gui::SameLine();
		}

		// Search Bar + Filtering
		const float inputTextSize = Gui::GetWindowWidth() / 3.0f;
		UI::ShiftCursorX(Gui::GetContentRegionAvail().x - inputTextSize);
		Gui::SetNextItemWidth(inputTextSize);
		const bool isSearching = Gui::InputTextWithHint("##ItemAssetSearch", "Search...", m_SearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
		if (isSearching)
		{
			m_SearchInputTextFilter.Build();
		}
	}

	void ContentBrowserPanel::RenderRightClickItemPopup(const Fs::Path& currentPath)
	{
		// Right-click on directory or file for utilities popup
		if (Gui::BeginPopupContextItem())
		{
			if (Gui::MenuItem("Rename"))
			{
				m_ItemPathToRename = currentPath;
				Gui::CloseCurrentPopup();
			}
			UI::Draw::Underline();

			const bool isCSharpExtension = currentPath.filename().extension() == ".cs";

			if (isCSharpExtension && Gui::MenuItem("Open with VsCode"))
			{
				Platform::LaunchProcess("code", std::format("{} {}", currentPath.parent_path().string(), currentPath.string()).c_str());
				Gui::CloseCurrentPopup();
			}
			if (isCSharpExtension)
			{
				UI::Draw::Underline();
			}

			if (Gui::MenuItem("Open in Explorer"))
			{
				if (std::filesystem::is_directory(currentPath))
				{
					FileDialogue::OpenInFileExplorer(currentPath.string().c_str());
				}
				else
				{
					const Fs::Path parentDirectory = FileSystem::GetParentDirectory(currentPath);
					FileDialogue::OpenInFileExplorer(parentDirectory.string().c_str());
				}

				Gui::CloseCurrentPopup();
			}
			UI::Draw::Underline();

			if (Gui::MenuItem("Delete"))
			{
				m_ConfirmDeletionPopupOpen = true;
				Gui::CloseCurrentPopup();
			}

			Gui::EndPopup();
		}

		const Fs::Path filename = currentPath.filename();
		const std::string currentFilename = filename.string();

		UIOnPopupRender(currentFilename, currentPath);

		if (String::FastCompare(currentPath.string(), m_ItemPathToRename.string()))
		{
			RenameItem(currentPath);
		}
	}

	void ContentBrowserPanel::RenameItem(const Fs::Path& currentPath)
	{
		// Find the last backslash in the path and copy the filename to the buffer
		char buffer[256];
		const size_t lastSlashPos = currentPath.string().find_last_of('/\\');
		const std::string oldFilenameWithExtension = currentPath.string().substr(lastSlashPos + 1, currentPath.string().length());
		memcpy(buffer, oldFilenameWithExtension.c_str(), sizeof(buffer));

		Gui::SetKeyboardFocusHere();
		Gui::SetNextItemWidth(m_ThumbnailSize);

		const ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;

		if (Gui::InputText("##RenameInputText", buffer, sizeof(buffer), flags))
		{
			m_ItemPathToRename.clear();

			// Get the new path from the input text buffer relative to the current directory
			const bool inputEmpty = strlen(buffer) == 0;

			if (inputEmpty)
			{
				return;
			}

			const Fs::Path newFilepath = m_CurrentDirectory / Fs::Path(buffer);
			const bool equivalent = currentPath == newFilepath;
			const bool validRename = !inputEmpty && !equivalent;

			if (!validRename)
			{
				return;
  			}

			// Now we need to rename the asset
			SharedReference<Asset> asset = Project::GetEditorAssetManager()->GetAssetFromFilepath(currentPath);
			if (asset == nullptr)
			{
				FileSystem::Rename(currentPath, newFilepath);
				return;
			}

			if (!Project::GetEditorAssetManager()->RenameAsset(asset, newFilepath))
			{
				return;
			}

			FileSystem::Rename(currentPath, newFilepath);
			if (!Project::GetEditorAssetManager()->ReloadData(asset->Handle))
			{
				return;
			}

			Project::GetEditorAssetManager()->WriteToRegistryFile();
		}
	}

	void ContentBrowserPanel::OnCSharpFileRenamed(const Fs::Path& newFilepath, const std::string& oldFilepath)
	{
		std::ifstream file(newFilepath);

		if (file.is_open())
		{
			// Make sure we start at the beginning of the file
			file.seekg(0);

			std::vector<std::string> lineBuffer;
			std::string currentLine;

			// Get the old class name from the old filename - i.e. "Player.cs" -> "Player"
			const size_t lastDotPos = oldFilepath.find_last_of('.');
			const std::string oldClassName = oldFilepath.substr(0, lastDotPos);
			bool classNameFound = false;

			while (std::getline(file, currentLine))
			{
				lineBuffer.push_back(currentLine);

				if (currentLine.find(oldClassName) == std::string::npos)
					continue;

				classNameFound = true;
				// NOTE: we can't just break here beacuse we need all the lines in the file
			}

			file.close();

			VX_CORE_ASSERT(classNameFound, "C# Class Name was not the same as filename!");

			// Find the line with the class name and replace it
			for (auto& line : lineBuffer)
			{
				// We didn't find the class name, move on to next line
				if (line.find(oldClassName) == std::string::npos)
					continue;
				
				// Get the new name of the file to rename the C# class
				const std::string newPath = newFilepath.filename().string();
				const size_t lastDotPos = newPath.find_first_of('.');
				const std::string newClassName = newPath.substr(0, lastDotPos);

				String::ReplaceToken(line, oldClassName.c_str(), newClassName);

				// Replace the contents of the file
				std::ofstream fout(newFilepath, std::ios::trunc);
				for (auto& editedLine : lineBuffer)
				{
					fout << editedLine << '\n';
				}

				// don't forget to close it
				fout.close();

				// renaming class was successful
				break;
			}
		}
	}

	void ContentBrowserPanel::RenderThumbnailSlider()
	{
		Gui::PushItemWidth(-1);

		if (Gui::SliderFloat("##Thumbnail Size", &m_ThumbnailSize, 64.0f, 512.0f, "%.0f"))
		{
			m_ThumbnailPadding = m_ThumbnailSize / 6.0f;
		}

		Gui::PopItemWidth();
	}

	SharedReference<Texture2D> ContentBrowserPanel::FindSuitableItemIcon(const std::filesystem::directory_entry& directoryEntry, const Fs::Path& currentItemPath)
	{
		const std::string extension = FileSystem::GetFileExtension(currentItemPath);
		SharedReference<Texture2D> itemIcon = EditorResources::FileIcon;

		if (directoryEntry.is_directory())
		{
			itemIcon = EditorResources::DirectoryIcon;
			return itemIcon;
		}

		const AssetType assetType = Project::GetEditorAssetManager()->GetAssetTypeFromExtension(extension);
		if (assetType == AssetType::None)
		{
			return itemIcon;
		}

		switch (assetType)
		{
			case AssetType::MeshAsset:            itemIcon = FindMeshIcon(extension);                             break;
			case AssetType::FontAsset:            itemIcon = EditorResources::FontIcon;                           break;
			case AssetType::AudioAsset:           itemIcon = EditorResources::AudioFileIcon;                      break;
			case AssetType::SceneAsset:           itemIcon = EditorResources::SceneIcon;                          break;
			case AssetType::PrefabAsset:          break;
			case AssetType::ScriptAsset:          itemIcon = EditorResources::CodeFileIcon;                       break;
			case AssetType::TextureAsset:         itemIcon = FindTextureFromAssetManager(currentItemPath);        break;
			case AssetType::MaterialAsset:        break;
			case AssetType::AnimatorAsset:        break;
			case AssetType::AnimationAsset:       break;
			case AssetType::StaticMeshAsset:      itemIcon = FindMeshIcon(extension);                             break;
			case AssetType::EnvironmentAsset:     itemIcon = FindEnvironmentMapFromAssetManager(currentItemPath); break;
			case AssetType::PhysicsMaterialAsset: break;
		}

		return itemIcon;
	}

	SharedReference<Texture2D> ContentBrowserPanel::FindTextureFromAssetManager(const Fs::Path& currentItemPath)
	{
		if (SharedReference<Asset> asset = Project::GetEditorAssetManager()->GetAssetFromFilepath(currentItemPath))
		{
			VX_CORE_ASSERT(asset.Is<Texture2D>(), "Invalid Texture!");
			return asset.As<Texture2D>();
		}

		VX_CORE_ASSERT(false, "Unknown texture!");
		return nullptr;
	}

	SharedReference<Texture2D> ContentBrowserPanel::FindEnvironmentMapFromAssetManager(const Fs::Path& currentItemPath)
	{
		if (SharedReference<Asset> asset = Project::GetEditorAssetManager()->GetAssetFromFilepath(currentItemPath))
		{
			VX_CORE_ASSERT(asset.Is<Skybox>(), "Invalid Environment!");
			return asset.As<Skybox>()->GetEnvironmentMap();
		}

		VX_CORE_ASSERT(false, "Unknown environment map!");
		return nullptr;
	}

	SharedReference<Texture2D> ContentBrowserPanel::FindMeshIcon(const Fs::Path& extension)
	{
		if (extension == ".obj")
		{
			return EditorResources::OBJIcon;
		}
		else if (extension == ".fbx")
		{
			return EditorResources::FBXIcon;
		}
		else if (extension == ".gltf")
		{
			return EditorResources::FileIcon;
		}

		VX_CORE_ASSERT(false, "Unknown extension!");
		return nullptr;
	}

	void ContentBrowserPanel::UIOnPopupRender(const std::string& currentFilename, const Fs::Path& currentPath)
	{
		OnConfirmDeletionPopupRender(currentFilename, currentPath);
	}

	void ContentBrowserPanel::OnConfirmDeletionPopupRender(const std::string& currentFilename, const Fs::Path& currentPath)
	{
		const std::string popupName = "Confirm##ContentBrowser";
		if (m_ConfirmDeletionPopupOpen)
		{
			Gui::OpenPopup(popupName.c_str());
			m_ConfirmDeletionPopupOpen = false;
		}

		if (UI::ShowMessageBox(popupName.c_str(), nullptr, { 500, 165 }))
		{
			UI::Draw::Underline();
			Gui::Spacing();

			const ImVec2 button_size(Gui::GetFontSize() * 8.65f, 0.0f);

			Gui::TextCentered(fmt::format("Are you sure you want to permanently delete '{}' ?", currentFilename).c_str(), 40.0f);

			Gui::Spacing();
			UI::Draw::Underline();
			UI::ShiftCursorY(60.0f);

			if (Gui::Button("Yes", button_size))
			{
				FileSystem::Remove(currentPath);
				Gui::CloseCurrentPopup();
			}
			Gui::SameLine();

			if (Gui::Button("No", button_size))
			{
				Gui::CloseCurrentPopup();
			}
			Gui::SameLine();

			if (Gui::Button("Cancel", button_size))
			{
				Gui::CloseCurrentPopup();
			}

			Gui::EndPopup();
		}
	}

}
