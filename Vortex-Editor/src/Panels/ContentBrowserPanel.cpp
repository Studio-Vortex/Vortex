#include "ContentBrowserPanel.h"

#include <Vortex/Editor/EditorResources.h>

#include <imgui_internal.h>

namespace Vortex {

	ContentBrowserPanel::ContentBrowserPanel(const std::filesystem::path& assetDir)
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
					m_ItemPathToRename = m_CurrentDirectory / std::filesystem::path("New Folder");
					std::filesystem::create_directory(m_ItemPathToRename);
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Scene"))
				{
					m_ItemPathToRename = m_CurrentDirectory / std::filesystem::path("Untitled.vortex");
					std::ofstream fout(m_ItemPathToRename);
					fout << "Scene: Untitled\nEntities:";
					fout.close();

					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Material"))
				{
					std::filesystem::path materialsDirectory = Project::GetAssetDirectory() / "Materials";
					if (!FileSystem::Exists(materialsDirectory))
						FileSystem::CreateDirectoryV(materialsDirectory);

					m_ItemPathToRename = m_CurrentDirectory / "NewMaterial.vmaterial";
					SharedReference<Shader> pbrStaticShader = Renderer::GetShaderLibrary().Get("PBR_Static");
					SharedReference<EditorAssetManager> editorAssetManager = Project::GetEditorAssetManager();
					SharedReference<Material> materialAsset = editorAssetManager->CreateNewAsset<Material>("Materials", "NewMaterial.vmaterial", pbrStaticShader, MaterialProperties());
					VX_CORE_ASSERT(AssetManager::IsHandleValid(materialAsset->Handle), "Invalid asset handle!");

					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("C# Script"))
				{
					m_ItemPathToRename = m_CurrentDirectory / std::filesystem::path("Untitled.cs");
					std::ofstream fout(m_ItemPathToRename);
					fout << R"(using Vortex;

public class Untitled : Entity
{
	// Called once before the first frame or when the Entity is Instantiated into the Scene
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
			std::filesystem::path relativePath = currentPath;
			std::string filenameString = relativePath.filename().string();
			const bool skipDirectoryEntry = !m_SearchInputTextFilter.PassFilter(relativePath.string().c_str());
			const bool isHiddenFile = !directoryEntry.is_directory()
				&& !Project::GetEditorAssetManager()->IsValidAssetExtension(FileSystem::GetFileExtension(currentPath));

			if (skipDirectoryEntry || isHiddenFile)
				continue;

			Gui::PushID(filenameString.c_str());

			SharedReference<Texture2D> itemIcon = FindSuitableItemIcon(directoryEntry, currentPath);

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

				SharedReference<Texture2D> icon = FindSuitableItemIcon(directoryEntry, currentPath);

				UI::ImageEx(icon, { 12.0f, 12.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
				Gui::SameLine();

				std::string fullPath = directoryEntry.path().relative_path().string();
				size_t lastSlashPos = fullPath.find_last_of("/\\") + 1;
				std::string filenameWithExtension = fullPath.substr(lastSlashPos);
				std::string filename = FileSystem::RemoveFileExtension(filenameWithExtension);
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
				size_t dotPos = filenameString.find('.');
				std::string filenameWithoutExtension = filenameString.substr(0, dotPos);
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
				Entity& droppedEntity = *(Entity*)payload->Data;
				VX_CONSOLE_LOG_INFO("Dropped Entity Name: {}", droppedEntity.GetName());
				// Todo Create a new prefab here from the entity's uuid
				//SharedRef<Prefab> prefab = Prefab::Create((Project::GetProjectDirectory() / droppedEntity.GetName() / ".vprefab"));
				SharedRef<Prefab> prefab = Prefab::Create(droppedEntity);
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

		std::filesystem::path fullyQualifiedAssetDirectory = m_BaseDirectory;
		size_t lastSlashPos = fullyQualifiedAssetDirectory.string().find_last_of("/\\") + 1;
		std::string relativeProjectAssetDirectory = fullyQualifiedAssetDirectory.string().substr(lastSlashPos, fullyQualifiedAssetDirectory.string().size());

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

		std::filesystem::path relativePath = FileSystem::Relative(m_CurrentDirectory, m_BaseDirectory);
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

			std::string label = entry + "##" + std::to_string(i);
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
				break;

			UI::ShiftCursorY(1.0f);
			Gui::Text((const char*)VX_ICON_CHEVRON_RIGHT);

			Gui::SameLine();
		}

		// Search Bar + Filtering
		float inputTextSize = Gui::GetWindowWidth() / 3.0f;
		UI::ShiftCursorX(Gui::GetContentRegionAvail().x - inputTextSize);
		Gui::SetNextItemWidth(inputTextSize);
		const bool isSearching = Gui::InputTextWithHint("##ItemAssetSearch", "Search...", m_SearchInputTextFilter.InputBuf, IM_ARRAYSIZE(m_SearchInputTextFilter.InputBuf));
		if (isSearching)
			m_SearchInputTextFilter.Build();
	}

	void ContentBrowserPanel::RenderRightClickItemPopup(const std::filesystem::path& currentItemPath)
	{
		static bool confirmDeletionPopupOpen = false;

		// Right-click on directory or file for utilities popup
		if (Gui::BeginPopupContextItem())
		{
			if (Gui::MenuItem("Rename"))
			{
				m_ItemPathToRename = currentItemPath;
				Gui::CloseCurrentPopup();
			}
			UI::Draw::Underline();

			bool isCSharpFile = currentItemPath.filename().extension() == ".cs";

			if (isCSharpFile && Gui::MenuItem("Open with VsCode"))
			{
				Platform::LaunchProcess("code", std::format("{} {}", currentItemPath.parent_path().string(), currentItemPath.string()).c_str());
				Gui::CloseCurrentPopup();
			}
			if (isCSharpFile)
				UI::Draw::Underline();

			if (Gui::MenuItem("Open in Explorer"))
			{
				if (std::filesystem::is_directory(currentItemPath))
				{
					FileDialogue::OpenInFileExplorer(currentItemPath.string().c_str());
				}
				else
				{
					std::filesystem::path parentDirectory = FileSystem::GetParentDirectory(currentItemPath);
					FileDialogue::OpenInFileExplorer(parentDirectory.string().c_str());
				}

				Gui::CloseCurrentPopup();
			}
			UI::Draw::Underline();

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

		if (UI::ShowMessageBox("Confirm", { 500, 165 }))
		{
			UI::Draw::Underline();
			Gui::Spacing();

			ImVec2 button_size(Gui::GetFontSize() * 8.65f, 0.0f);

			Gui::TextCentered(fmt::format("Are you sure you want to permanently delete '{}' ?", currentItemPath.filename()).c_str(), 40.0f);
			Gui::TextCentered("This cannot be undone.", 60.0f);

			Gui::Spacing();
			UI::Draw::Underline();

			UI::ShiftCursorY(40.0f);

			if (Gui::Button("Yes", button_size))
			{
				FileSystem::Remove(currentItemPath);
				Gui::CloseCurrentPopup();
			}

			Gui::SameLine();

			if (Gui::Button("No", button_size))
				Gui::CloseCurrentPopup();

			Gui::SameLine();

			if (Gui::Button("Cancel", button_size))
				Gui::CloseCurrentPopup();

			Gui::EndPopup();
		}

		if (currentItemPath == m_ItemPathToRename)
		{
			// Find the last backslash in the path and copy the filename to the buffer
			char buffer[256];
			size_t pos = currentItemPath.string().find_last_of('\\');
			std::string oldFilenameWithExtension = currentItemPath.string().substr(pos + 1, currentItemPath.string().length());
			memcpy(buffer, oldFilenameWithExtension.c_str(), sizeof(buffer));

			Gui::SetKeyboardFocusHere();
			Gui::SetNextItemWidth(m_ThumbnailSize);
			if (Gui::InputText("##RenameInputText", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
			{
				const bool inputTextEmpty = strlen(buffer) == 0;
				const bool consistentPaths = (m_CurrentDirectory / std::filesystem::path(buffer)) == currentItemPath;

				if (!inputTextEmpty && !consistentPaths)
				{
					// Get the new path from the input text buffer relative to the current directory
					std::filesystem::path newFilePath = m_CurrentDirectory / std::filesystem::path(buffer);

					// Temporary until asset manager is sorted
					std::filesystem::rename(currentItemPath, newFilePath);

					if (FileSystem::HasFileExtension(currentItemPath))
					{
						SharedReference<Asset> asset = Project::GetEditorAssetManager()->GetAssetFromFilepath(oldFilenameWithExtension);
						// TODO make this work
						//Project::GetEditorAssetManager()->RenameAsset(asset, newFilePath.string());

						// TODO this should take place in asset manager
						if (newFilePath.filename().extension() == ".vmaterial")
						{
							SharedReference<Material> material = Project::GetEditorAssetManager()->GetAssetFromFilepath(oldFilenameWithExtension);
							if (material)
							{
								material->SetName(FileSystem::RemoveFileExtension(newFilePath));
							}
						}

						// TODO this should take place in asset manager
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

								VX_CORE_ASSERT(classNameFound, "C# Class Name was not the same as filename!");

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
						if (newFilePath.filename().extension() == ".png" || newFilePath.filename().extension() == ".jpg" || newFilePath.filename().extension() == ".jpeg" || newFilePath.filename().extension() == ".tga" || newFilePath.filename().extension() == ".psd")
						{
							// TODO: Once we have an asset system, ask the asset system to rename an asset here otherwise the engine could crash by loading a non-existant file
							// we have the old filename
							//oldFilenameWithExtension
							// we also have the newFilePath
						}
					}
				}

				m_ItemPathToRename.clear();
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

	SharedReference<Texture2D> ContentBrowserPanel::FindSuitableItemIcon(const std::filesystem::directory_entry& directoryEntry, const std::filesystem::path& currentItemPath)
	{
		std::string extension = FileSystem::GetFileExtension(currentItemPath);
		SharedReference<Texture2D> itemIcon = nullptr;

		if (directoryEntry.is_directory())
		{
			itemIcon = EditorResources::DirectoryIcon;
			return itemIcon;
		}

		AssetType assetType = AssetType::None;
		if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromExtension(extension); type != AssetType::None)
			assetType = type;
		
		if (assetType == AssetType::None)
		{
			itemIcon = EditorResources::FileIcon;
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

		// This means we either haven't implemented an Icon for this extension or it's just a random extension
		if (itemIcon == nullptr)
		{
			itemIcon = EditorResources::FileIcon;
		}

		return itemIcon;
	}

	SharedReference<Texture2D> ContentBrowserPanel::FindTextureFromAssetManager(const std::filesystem::path& currentItemPath)
	{
		if (SharedReference<Asset> asset = Project::GetEditorAssetManager()->GetAssetFromFilepath(currentItemPath))
		{
			VX_CORE_ASSERT(asset.Is<Texture2D>(), "Invalid Texture!");
			return asset.As<Texture2D>();
		}

		VX_CORE_ASSERT(false, "Unknown texture!");
		return nullptr;
	}

	SharedReference<Texture2D> ContentBrowserPanel::FindEnvironmentMapFromAssetManager(const std::filesystem::path& currentItemPath)
	{
		if (SharedReference<Asset> asset = Project::GetEditorAssetManager()->GetAssetFromFilepath(currentItemPath))
		{
			VX_CORE_ASSERT(asset.Is<Skybox>(), "Invalid Environment!");
			return asset.As<Skybox>()->GetEnvironmentMap();
		}

		VX_CORE_ASSERT(false, "Unknown environment map!");
		return nullptr;
	}

	SharedReference<Texture2D> ContentBrowserPanel::FindMeshIcon(const std::filesystem::path& extension)
	{
		if (extension == ".obj")
			return EditorResources::OBJIcon;
		else if (extension == ".fbx")
			return EditorResources::FBXIcon;

		VX_CORE_ASSERT(false, "Unknown extension!");
		return nullptr;
	}

}
