#include "MaterialEditorPanel.h"
#include <Vortex/Editor/EditorResources.h>
#include <Vortex/Utils/PlatformUtils.h>

namespace Vortex {

	namespace Utils {

		static void RenderTextureButton(const char* label, SharedRef<Texture2D>& texture)
		{
			SharedRef<Texture2D> tex = texture ? texture : EditorResources::CheckerboardIcon;
			ImVec2 textureSize = { 96, 96 };
			ImVec4 bgColor = { 0.0f, 0.0f, 0.0f, 0.0f };
			ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

			if (UI::ImageButton(label, tex, textureSize, bgColor, tintColor))
			{
				std::string filepath = FileSystem::OpenFileDialog("Texture File (*.png;*.jpg;*.tga)\0*.png;*.jpg;*.tga\0");
				if (!filepath.empty())
				{
					std::string projectDir = Project::GetProjectDirectory().string();
					std::string relativePath = std::filesystem::relative(filepath, projectDir).string();
					texture = Texture2D::Create(relativePath);
				}
			}

			UI::SetTooltip(tex->GetPath().c_str());

			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(path);

					// Make sure we are recieving an actual texture otherwise we will have trouble opening it
					if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga")
					{
						SharedRef<Texture2D> newTexture = Texture2D::Create(texturePath.string());

						if (newTexture->IsLoaded())
							texture = newTexture;
						else
							VX_WARN("Could not load texture {}", texturePath.filename().string());
					}
					else
						VX_WARN("Could not load texture, not a '.png', '.jpg' or '.tga' - {}", texturePath.filename().string());
				}
				Gui::EndDragDropTarget();
			}

			// right click for utilities
			if (tex != EditorResources::CheckerboardIcon && Gui::BeginPopupContextItem())
			{
				std::string remove = "Remove##" + tex->GetPath();
				if (Gui::MenuItem(remove.c_str()))
				{
					texture = nullptr;
					Gui::CloseCurrentPopup();
				}
				Gui::Separator();

				std::string openInExplorer = "Open in Explorer##" + tex->GetPath();
				if (Gui::MenuItem(openInExplorer.c_str()))
				{
					FileSystem::OpenInFileExplorer(tex->GetPath().c_str());
					Gui::CloseCurrentPopup();
				}

				Gui::EndPopup();
			}
		}

	}

	void MaterialEditorPanel::OnGuiRender(Entity selectedEntity)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		Gui::Begin("Material Editor", &s_ShowPanel);

		if (!selectedEntity || !selectedEntity.HasComponent<MeshRendererComponent>())
		{
			Gui::End();
			return;
		}

		SharedRef<Model> model = selectedEntity.GetComponent<MeshRendererComponent>().Mesh;

		if (!model)
		{
			Gui::End();
			return;
		}

		SharedRef<Material> material = model->GetMaterial();

		if (!material)
		{
			Gui::End();
			return;
		}

		static const char* displayNames[] = {
			"Albedo", "Normal", "Metallic", "Roughness", "Emission", "Parallax Occlusion", "Ambient Occlusion"
		};
		static SharedRef<Texture2D> textures[] = {
			material->GetAlbedoMap(),
			material->GetNormalMap(),
			material->GetMetallicMap(),
			material->GetRoughnessMap(),
			material->GetEmissionMap(),
			material->GetParallaxOcclusionMap(),
			material->GetAmbientOcclusionMap()
		};

		uint32_t count = VX_ARRAYCOUNT(displayNames);

		UI::BeginPropertyGrid();

		for (uint32_t i = 0; i < count; i++)
		{
			Utils::RenderTextureButton(displayNames[i], textures[i]);
		}

		UI::EndPropertyGrid();

		Gui::End();
	}

}
