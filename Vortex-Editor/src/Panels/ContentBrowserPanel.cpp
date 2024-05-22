#include "ContentBrowserPanel.h"

#include <Vortex/Serialization/SceneSerializer.h>
#include <Vortex/Editor/EditorResources.h>

#include <imgui_internal.h>

namespace Vortex {

	ContentBrowserPanel::ContentBrowserPanel(const Fs::Path& assetDir)
		: m_BaseDirectory(assetDir), m_CurrentDirectory(m_BaseDirectory)
	{
		if (!FileSystem::Exists(assetDir))
		{
			VX_CONSOLE_LOG_ERROR("[Editor] Failed to setup content browser, asset directory was invalid!");
			return;
		}

		m_ThumbnailGenerator = SharedReference<ThumbnailGenerator>::Create();
		m_ThumbnailGenerator->Init();

		ProcessAssetDirectory(assetDir);
	}

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

		Gui::Spacing();
		UI::Draw::Underline();
		Gui::Spacing();

		RenderThumbnailSlider();

		Gui::EndChild();
		Gui::EndGroup();

		Gui::End();
	}

	void ContentBrowserPanel::ProcessAssetDirectory(const Fs::Path& assetDir)
	{
		// setup base asset directory
		Directory baseDirectory;
		baseDirectory.Path = assetDir;
		baseDirectory.Stem = assetDir.stem().string();
		baseDirectory.Parent = nullptr; // this is the root dir

		RecursiveProcessDirectory(&baseDirectory);
		ProcessFilesInDirectory(&baseDirectory);

		m_AssetDirectory = SharedReference<ProjectAssetDirectory>::Create(baseDirectory);
	}

	void ContentBrowserPanel::RenderCreateItemPopup()
	{
		auto separator = []() {
			UI::Draw::Underline();
			Gui::Spacing();
		};

		if (Gui::BeginMenu("Create"))
		{
			const Fs::Path currentDirectory = m_CurrentDirectory.filename();

			if (Gui::MenuItem("Folder"))
			{
				const std::string filename = "New Folder";
				m_ItemPathToRename = m_CurrentDirectory / filename;
				FileSystem::CreateDirectoryV(m_ItemPathToRename);

				Gui::CloseCurrentPopup();
			}
			separator();

			if (Gui::MenuItem("Scene"))
			{
				const std::string filename = "Untitled.vortex";
				SharedReference<Scene> scene = Project::GetEditorAssetManager()->CreateNewAsset<Scene>(currentDirectory.string(), filename);
				SceneSerializer serializer(scene);
				serializer.Serialize((m_CurrentDirectory / filename).string());
				VX_CORE_ASSERT(scene, "Failed to create scene!");

				Gui::CloseCurrentPopup();
			}
			separator();

			if (Gui::MenuItem("Material"))
			{
				const std::string filename = "NewMaterial.vmaterial";
				SharedReference<Shader> shader = Renderer::GetShaderLibrary().Get("PBR_Static");
				MaterialProperties properties = MaterialProperties();
				SharedReference<Material> material = Project::GetEditorAssetManager()->CreateNewAsset<Material>(currentDirectory.string(), filename, shader, properties);
				VX_CORE_ASSERT(material, "Failed to create material!");

				Gui::CloseCurrentPopup();
			}
			separator();

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
	}

	void ContentBrowserPanel::RenderFileExplorer()
	{
		RenderMenuBar();
		Gui::Spacing();
		UI::Draw::Underline();

		const ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		const float cellSize = m_ThumbnailSize + m_ThumbnailPadding;
		const float panelWidth = contentRegionAvail.x;

		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
		{
			columnCount = 1;
		}

		//                                                leave some space for the thumbnail size slider
		Gui::BeginChild("##FileExplorer", ImVec2(0, contentRegionAvail.y - 45.0f), false);

		Gui::Columns(columnCount, 0, false);

		for (const auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const Fs::Path& currentPath = directoryEntry.path();
			const std::string extension = FileSystem::GetFileExtension(currentPath);
			const std::string filenameString = currentPath.filename().string();
			const bool matchingSearch = !m_SearchInputTextFilter.PassFilter(currentPath.string().c_str());
			const bool isDirectory = directoryEntry.is_directory();
			const bool validAssetExtension = Project::GetEditorAssetManager()->IsValidAssetExtension(extension);
			const bool isHiddenFile = !isDirectory && !validAssetExtension;

			if (matchingSearch)
				continue;

			if (isHiddenFile)
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
				const wchar_t* itemPath = currentPath.c_str();
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

		// Right-click on blank space in content browser panel to show create menu
		if (Gui::BeginPopupContextWindow(0, 1, false))
		{
			RenderCreateItemPopup();

			Gui::EndPopup();
		}

		Gui::EndChild();

		// Accept a Prefab from the scene hierarchy
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
			{
				const Actor& droppedActor = *(Actor*)payload->Data;
				if (droppedActor)
				{
					VX_CONSOLE_LOG_INFO("Dropped Actor Name: {}", droppedActor.Name());

					const std::string filename = droppedActor.Name() + ".vprefab";

					// todo should we use prefabs directory or the current directory
					const Fs::Path prefabDirectory = Project::GetAssetDirectory() / "Prefabs";
					if (!FileSystem::Exists(prefabDirectory)) {
						FileSystem::CreateDirectoryV(prefabDirectory);
					}

					SharedReference<Prefab> prefab = Project::GetEditorAssetManager()->CreateNewAsset<Prefab>("Prefabs", filename);
					if (prefab)
						prefab->Create(droppedActor);
					else
						VX_CONSOLE_LOG_ERROR("Failed to create prefab!");
				}
			}
			Gui::EndDragDropTarget();
		}
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

		const std::string assetDirectory = m_BaseDirectory.string();
		const size_t lastSlashPos = assetDirectory.find_last_of("/\\") + 1;
		const std::string relativeAssetDirectory = assetDirectory.substr(lastSlashPos, assetDirectory.size());

		if (FileSystem::Equivalent(m_CurrentDirectory, Project::GetAssetDirectory()))
		{
			Gui::Text(relativeAssetDirectory.c_str());
		}
		else
		{
			if (Gui::Button(relativeAssetDirectory.c_str()))
			{
				m_CurrentDirectory = m_BaseDirectory;
			}
		}

		Gui::SameLine();

		RenderCurrentWorkingDirectory();

		// Search Bar + Filtering
		const ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		const float inputTextSize = (Gui::GetWindowWidth() / 2.0f) * 0.85f;

		UI::ShiftCursor(contentRegionAvail.x - inputTextSize - 1.0f, 1.0f);
		Gui::SetNextItemWidth(inputTextSize);
		const bool isSearching = Gui::InputTextWithHint("##ItemAssetSearch", "Search...", m_SearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
		if (isSearching) {
			m_SearchInputTextFilter.Build();
		}

		UI::DrawItemActivityOutline();
	}

	void ContentBrowserPanel::RenderCurrentWorkingDirectory()
	{
		const Fs::Path relativePath = FileSystem::Relative(m_CurrentDirectory, m_BaseDirectory);
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
				const Fs::Path currentDirectory = m_CurrentDirectory.filename();
				if (currentDirectory.string() != entry)
				{
					// find the directory
					Fs::Path directory = m_CurrentDirectory;
					for (uint32_t j = 0; j < numPaths; j++)
					{
						directory = directory.parent_path();

						const Fs::Path directoryFilename = directory.filename();

						if (directoryFilename != entry)
							continue;

						// we found the path
						m_CurrentDirectory = directory;
						break;
					}
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
		memcpy(buffer, oldFilenameWithExtension.c_str(), oldFilenameWithExtension.size());

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
		UI::BeginPropertyGrid();

		if (UI::PropertySlider("Thumbnail Size", m_ThumbnailSize, 64.0f, 512.0f, " % .0f"))
		{
			m_ThumbnailPadding = m_ThumbnailSize / 6.0f;
		}

		UI::EndPropertyGrid();
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
			case AssetType::PrefabAsset:          itemIcon = EditorResources::PrefabIcon;                         break;
			case AssetType::ScriptAsset:          itemIcon = EditorResources::CodeFileIcon;                       break;
			case AssetType::TextureAsset:         itemIcon = FindTextureFromAssetManager(currentItemPath);        break;
			case AssetType::MaterialAsset:        itemIcon = EditorResources::MaterialIcon;                       break;
			case AssetType::AnimatorAsset:                                                                        break;
			case AssetType::AnimationAsset:                                                                       break;
			case AssetType::StaticMeshAsset:      itemIcon = FindMeshIcon(extension);                             break;
			case AssetType::EnvironmentAsset:     itemIcon = FindEnvironmentMapFromAssetManager(currentItemPath); break;
			case AssetType::PhysicsMaterialAsset:                                                                 break;
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
		std::string ext = extension.string();
		String::ToLower(ext);

		if (ext == ".fbx")
		{
			return EditorResources::FBXIcon;
		}
		else if (ext == ".gltf")
		{
			return EditorResources::FileIcon;
		}
		else if (ext == ".dae")
		{
			return EditorResources::FileIcon;
		}
		else if (ext == ".glb")
		{
			return EditorResources::FileIcon;
		}
		else if (ext == ".obj")
		{
			return EditorResources::OBJIcon;
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

			Gui::TextCentered(fmt::format("Are you sure you want to permanently delete '{}'?", currentFilename).c_str(), 40.0f);

			Gui::Spacing();
			UI::Draw::Underline();
			UI::ShiftCursorY(60.0f);

			if (Gui::Button("Yes", button_size))
			{
				Fs::Path path = Project::GetEditorAssetManager()->GetRelativePath(currentPath);
				VX_CONSOLE_LOG_INFO("{} deleted from asset directory.", path.string());
				SharedReference<Asset> asset = Project::GetEditorAssetManager()->GetAssetFromFilepath(currentPath);
				if (asset) {
					Project::GetEditorAssetManager()->RemoveAsset(asset->Handle);
				}
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

	void ContentBrowserPanel::RecursiveProcessDirectory(Directory* current) const
	{
		for (const auto& entry : std::filesystem::recursive_directory_iterator(current->Path))
		{
			if (!entry.is_directory())
				continue;

			const Fs::Path path = entry.path();
			const std::string stem = path.stem().string();
			if (stem.starts_with('.'))
				continue;

			Directory directoryEntry;
			directoryEntry.Path = path;
			directoryEntry.Stem = stem;
			directoryEntry.Parent = current;

			std::vector<Directory>& children = current->Children;

			children.push_back(directoryEntry);
			Directory* last = &children.back();

			RecursiveProcessDirectory(last);
			ProcessFilesInDirectory(last);
		}
	}

	void ContentBrowserPanel::ProcessFilesInDirectory(Directory* current) const
	{
		for (const auto& entry : std::filesystem::directory_iterator(current->Path))
		{
			if (entry.is_directory())
				continue;

			const Fs::Path path = entry.path();
			if (!path.has_extension())
				continue;

			const Fs::Path ext = path.extension();
			const bool valid = Project::GetEditorAssetManager()->IsValidAssetExtension(ext);
			if (!valid)
				continue;

			AssetEntry assetEntry;
			assetEntry.Metadata = Project::GetEditorAssetManager()->GetMetadata(path);
			assetEntry.Location = current;

			current->Files.push_back(assetEntry);
		}
	}

}
