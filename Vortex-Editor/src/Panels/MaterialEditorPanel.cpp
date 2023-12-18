#include "MaterialEditorPanel.h"

#include <Vortex/Editor/EditorResources.h>
#include <Vortex/Editor/SelectionManager.h>

namespace Vortex {

	void MaterialEditorPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

		Gui::Begin(m_PanelName.c_str(), &IsOpen, flags);

		if (Actor selected = SelectionManager::GetSelectedActor())
		{
			RenderMeshMaterial(selected);
		}

		Gui::End();
	}

	void MaterialEditorPanel::RenderMeshMaterial(Actor selectedActor)
	{
		if (!selectedActor.HasAny<MeshRendererComponent, StaticMeshRendererComponent>())
			return;

		const ShaderLibrary& shaderLibrary = Renderer::GetShaderLibrary();
		static std::vector<const char*> shaderNames;

		if (shaderNames.empty())
		{
			for (const auto& [shaderName, shader] : shaderLibrary)
			{
				shaderNames.emplace_back(shaderName.c_str());
			}
		}

		if (selectedActor.HasComponent<MeshRendererComponent>())
		{
			AssetHandle meshHandle = selectedActor.GetComponent<MeshRendererComponent>().Mesh;
			if (!AssetManager::IsHandleValid(meshHandle))
			{
				Gui::End();
				return;
			}

			SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
			if (mesh == nullptr)
			{
				Gui::End();
				return;
			}

			const auto& submesh = mesh->GetSubmesh();

			SharedReference<Material> material = submesh.GetMaterial();
			if (material == nullptr)
			{
				Gui::End();
				return;
			}

			const std::string& name = material->GetName() + " / " + submesh.GetName();

			if (UI::PropertyGridHeader(name.c_str()))
			{
				UI::BeginPropertyGrid();

				std::string currentShaderName = material->GetShader()->GetName();
				if (UI::PropertyDropdownSearch("Shader", shaderNames.data(), shaderNames.size(), currentShaderName, m_ShaderDropdownTextFilter))
				{
					if (shaderLibrary.Exists(currentShaderName))
					{
						SharedReference<Shader> shader = shaderLibrary.Get(currentShaderName);
						if (shader != nullptr)
						{
							material->SetShader(shader);
						}
					}
				}

				UI::EndPropertyGrid();

				RenderMaterialTexturesAndProperties(material, VX_BIND_CALLBACK(MaterialEditorPanel::ParameterCallback));

				UI::EndTreeNode();
			}
		}
		else if (selectedActor.HasComponent<StaticMeshRendererComponent>())
		{
			auto& staticMeshRendererComponent = selectedActor.GetComponent<StaticMeshRendererComponent>();
			AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
			if (!AssetManager::IsHandleValid(staticMeshHandle))
			{
				Gui::End();
				return;
			}

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
			if (staticMesh == nullptr)
			{
				Gui::End();
				return;
			}

			const auto& submeshes = staticMesh->GetSubmeshes();

			SharedReference<MaterialTable> materialTable = staticMeshRendererComponent.Materials;

			for (const auto& [submeshIndex, submesh] : submeshes)
			{
				VX_CORE_ASSERT(materialTable->HasMaterial(submeshIndex), "Material Table not synchronized with component!");

				AssetHandle materialHandle = materialTable->GetMaterial(submeshIndex);
				if (!AssetManager::IsHandleValid(materialHandle))
					continue;

				SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
				if (material == nullptr)
					continue;

				const std::string& name = material->GetName() + " / " + submesh.GetName();

				if (UI::PropertyGridHeader(name.c_str()))
				{
					UI::BeginPropertyGrid();

					std::string currentShaderName = material->GetShader()->GetName();
					if (UI::PropertyDropdownSearch("Shader", shaderNames.data(), shaderNames.size(), currentShaderName, m_ShaderDropdownTextFilter))
					{
						if (shaderLibrary.Exists(currentShaderName))
						{
							SharedReference<Shader> shader = shaderLibrary.Get(currentShaderName);
							material->SetShader(shader);
						}
					}

					UI::EndPropertyGrid();

					RenderMaterialTexturesAndProperties(material, VX_BIND_CALLBACK(MaterialEditorPanel::ParameterCallback));
					
					UI::EndTreeNode();
				}
			}
		}
	}

	void MaterialEditorPanel::ParameterCallback(SharedReference<Material> material, uint32_t materialIndex)
	{
		switch (materialIndex)
		{
			case 0:
			{
				Math::vec3 albedo = material->GetAlbedo();
				if (UI::Property("Albedo", &albedo))
				{
					material->SetAlbedo(albedo);
				}

				break;
			}
			case 1:
			{
				// normals don't have parameters
				break;
			}
			case 2:
			{
				float metallic = material->GetMetallic();
				if (UI::PropertySlider("Metallic", metallic, FLT_MIN, 1.0f))
				{
					material->SetMetallic(metallic);
				}

				break;
			}
			case 3:
			{
				float roughness = material->GetRoughness();
				if (UI::PropertySlider("Roughness", roughness, FLT_MIN, 1.0f))
				{
					material->SetRoughness(roughness);
				}

				break;
			}
			case 4:
			{
				float emission = material->GetEmission();
				if (UI::Property("Emission", emission, 1.0f, FLT_MIN, FLT_MAX))
				{
					material->SetEmission(emission);
				}

				break;
			}
			case 5:
			{
				float parallaxHeightScale = material->GetParallaxHeightScale();
				if (UI::Property("Height Scale", parallaxHeightScale, 0.01f, FLT_MIN, 1.0f))
				{
					material->SetParallaxHeightScale(parallaxHeightScale);
				}

				break;
			}
			case 6:
			{
				// ao doesn't have parameters
				break;
			}
		}
	}

	SharedReference<Texture2D> MaterialEditorPanel::GetMaterialTexture(SharedReference<Material> material, uint32_t index)
	{
		VX_CORE_ASSERT(index <= 6, "Index out of bounds!");

		AssetHandle textureHandle;

		switch (index)
		{
			case 0: textureHandle = material->GetAlbedoMap();            break;
			case 1: textureHandle = material->GetNormalMap();            break;
			case 2: textureHandle = material->GetMetallicMap();          break;
			case 3: textureHandle = material->GetRoughnessMap();         break;
			case 4: textureHandle = material->GetEmissionMap();          break;
			case 5: textureHandle = material->GetParallaxOcclusionMap(); break;
			case 6: textureHandle = material->GetAmbientOcclusionMap();  break;
		}

		if (!AssetManager::IsHandleValid(textureHandle))
			return nullptr;

		SharedReference<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		if (!texture)
			return nullptr;

		return texture;
	}

	void MaterialEditorPanel::SetMaterialTexture(SharedReference<Material> material, AssetHandle textureHandle, uint32_t index)
	{
		VX_CORE_ASSERT(index <= 6, "Index out of bounds!");

		switch (index)
		{
			case 0: material->SetAlbedoMap(textureHandle);            break;
			case 1: material->SetNormalMap(textureHandle);            break;
			case 2: material->SetMetallicMap(textureHandle);          break;
			case 3: material->SetRoughnessMap(textureHandle);         break;
			case 4: material->SetEmissionMap(textureHandle);          break;
			case 5: material->SetParallaxOcclusionMap(textureHandle); break;
			case 6: material->SetAmbientOcclusionMap(textureHandle);  break;
		}
	}

	void MaterialEditorPanel::RenderMaterialFlags(SharedReference<Material> material)
	{
		static const char* displayNames[] = { "No Depth Test" };
		static MaterialFlag flags[] = { MaterialFlag::NoDepthTest };

		uint32_t count = VX_ARRAYSIZE(displayNames);

		for (uint32_t i = 0; i < count; i++)
		{
			bool flagEnabled = material->HasFlag(flags[i]);
			if (UI::Property(displayNames[i], flagEnabled))
			{
				material->ToggleFlag(flags[i]);
			}
		}
	}

	void MaterialEditorPanel::RenderMaterialProperties(SharedReference<Material> material)
	{
		Math::vec2 uv = material->GetUV();
		if (UI::Property("UV", uv, 0.01f))
		{
			material->SetUV(uv);
		}

		float opacity = material->GetOpacity();
		if (UI::Property("Opacity", opacity, 0.01f, FLT_MIN, 1.0f))
		{
			material->SetOpacity(opacity);
		}
	}

	bool MaterialEditorPanel::MaterialTextureHasProperties(uint32_t index)
	{
		switch (index)
		{
			case 0: return true;
			case 1: return false;
			case 2: return true;
			case 3: return true;
			case 4: return true;
			case 5: return true;
			case 6: return false;
		}

		VX_CORE_ASSERT(index <= 6, "Index out of bounds!");
		return -1;
	}

	void MaterialEditorPanel::RenderMaterialTexturesAndProperties(SharedReference<Material> material, MaterialParameterCallbackFunc parameterCallback)
	{
		static const char* displayNames[] =
		{
			"Albedo", "Normal", "Metallic", "Roughness", "Emission", "Parallax Occlusion", "Ambient Occlusion",
		};

		const ImVec2 textureSize = { 64, 64 };
		const ImVec4 bgColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		const ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		const Math::vec3& albedo = material->GetAlbedo();
		const ImVec4 albedoTint = { albedo.r, albedo.g, albedo.b, 1.0f };

		uint32_t count = VX_ARRAYSIZE(displayNames);

		bool hovered = false;
		bool leftMouseButtonClicked = false;
		bool rightMouseButtonClicked = false;

		for (uint32_t i = 0; i < count; i++)
		{
			SharedReference<Texture2D> texture = EditorResources::CheckerboardIcon;
			if (SharedReference<Texture2D> entry = GetMaterialTexture(material, i))
			{
				texture = entry;
			}

			std::string headerName = std::string(displayNames[i]) + "##" + material->GetName() + std::to_string(i);
			if (Gui::CollapsingHeader(headerName.c_str()))
			{
				UI::BeginPropertyGrid();

				ImVec4 backgroundColor = (i == 0) ? albedoTint : bgColor;
				UI::ImageEx(texture, textureSize, backgroundColor, tintColor);

				if (MaterialTextureHasProperties(i))
					Gui::SameLine();

				hovered = Gui::IsItemHovered();
				leftMouseButtonClicked = Gui::IsItemClicked(ImGuiMouseButton_Left);
				rightMouseButtonClicked = Gui::IsItemClicked(ImGuiMouseButton_Right);

				const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(texture->GetPath());
				std::string relativePath = metadata.Filepath.string();
				UI::SetTooltip(relativePath.c_str());

				if (Gui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						const Fs::Path texturePath = Fs::Path(path);

						// Make sure we are recieving an actual texture otherwise we will have trouble opening it
						if (AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(texturePath); type == AssetType::TextureAsset)
						{
							AssetHandle textureHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(texturePath);

							if (AssetManager::IsHandleValid(textureHandle))
							{
								SetMaterialTexture(material, textureHandle, i);
							}
							else
							{
								VX_CONSOLE_LOG_WARN("Could not load texture {}", texturePath.filename().string());
							}
						}
						else
						{
							VX_CONSOLE_LOG_WARN("Could not load texture", texturePath.filename().string());
						}
					}

					Gui::EndDragDropTarget();
				}

				if (hovered && leftMouseButtonClicked)
				{
					const std::string filepath = FileDialogue::OpenFileDialog("Texture File (*.png;*.jpg;*.jpeg;*.tga;*.psd)\0*.png;*.jpg;*.jpeg;*.tga;*.psd\0");

					if (!filepath.empty())
					{
						AssetHandle textureHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);

						if (AssetManager::IsHandleValid(textureHandle))
						{
							SetMaterialTexture(material, textureHandle, i);
						}
					}
				}

				// right click for utilities
				const std::string utilityPopupName = "MaterialUtility##" + std::to_string(texture->Handle) + relativePath;
				if (hovered && rightMouseButtonClicked)
				{
					Gui::OpenPopup(utilityPopupName.c_str());
				}
				if (texture != EditorResources::CheckerboardIcon && Gui::IsPopupOpen(utilityPopupName.c_str()) && Gui::BeginPopup(utilityPopupName.c_str()))
				{
					const std::string remove = fmt::format("Remove##{}-{}-{}", texture->Handle, relativePath, i);
					if (Gui::MenuItem(remove.c_str()))
					{
						SetMaterialTexture(material, (AssetHandle)0, i);
						Gui::CloseCurrentPopup();
					}

					UI::Draw::Underline();

					const std::string openInExplorer = fmt::format("Open in Explorer##{}-{}", texture->GetPath(), i);
					if (Gui::MenuItem(openInExplorer.c_str()))
					{
						FileDialogue::OpenInFileExplorer(texture->GetPath().c_str());
						Gui::CloseCurrentPopup();
					}

					Gui::EndPopup();
				}

				if (parameterCallback != nullptr)
				{
					std::invoke(parameterCallback, material, i);
				}

				UI::EndPropertyGrid();
			}
		}

		UI::BeginPropertyGrid();

		RenderMaterialProperties(material);
		RenderMaterialFlags(material);

		UI::EndPropertyGrid();
	}

}
