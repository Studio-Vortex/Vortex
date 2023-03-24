#include "MaterialEditorPanel.h"

#include <Vortex/Editor/EditorResources.h>
#include <Vortex/Editor/SelectionManager.h>

namespace Vortex {

	static ImGuiTextFilter s_ShaderDropdownTextFilter;

	namespace Utils {

		static SharedReference<Texture2D> GetMaterialTexture(const SharedReference<Material>& material, uint32_t index)
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

		static void SetMaterialTexture(SharedReference<Material>& material, AssetHandle textureHandle, uint32_t index)
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

		static void RenderMaterialFlags(SharedReference<Material>& material)
		{
			static const char* displayNames[] = { "No Depth Test" };
			static MaterialFlag flags[] = { MaterialFlag::NoDepthTest };

			uint32_t count = VX_ARRAYCOUNT(displayNames);

			for (uint32_t i = 0; i < count; i++)
			{
				bool flagEnabled = material->HasFlag(flags[i]);
				if (UI::Property(displayNames[i], flagEnabled))
					material->ToggleFlag(flags[i]);
			}
		}

		static void RenderMaterialProperties(SharedReference<Material>& material)
		{
			Math::vec2 uv = material->GetUV();
			if (UI::Property("UV", uv, 0.05f))
				material->SetUV(uv);

			float opacity = material->GetOpacity();
			if (UI::Property("Opacity", opacity, 0.01f, 0.01f, 1.0f))
				material->SetOpacity(opacity);

			RenderMaterialFlags(material);
		}

		static bool MaterialTextureHasProperties(uint32_t index)
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

		using MaterialParameterCallbackFunc = const std::function<void(SharedReference<Material>&, uint32_t)>&;
		static void RenderMaterialTexturesAndProperties(SharedReference<Material>& material, MaterialParameterCallbackFunc parameterCallback)
		{
			static const char* displayNames[] =
			{
				"Albedo", "Normal", "Metallic", "Roughness", "Emission", "Parallax Occlusion", "Ambient Occlusion",
			};

			ImVec2 textureSize = { 64, 64 };
			ImVec4 bgColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			Math::vec3 albedo = material->GetAlbedo();
			ImVec4 albedoTint = { albedo.r, albedo.g, albedo.b, 1.0f };

			uint32_t count = VX_ARRAYCOUNT(displayNames);

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
							std::filesystem::path texturePath = std::filesystem::path(path);

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
						std::string filepath = FileDialogue::OpenFileDialog("Texture File (*.png;*.jpg;*.jpeg;*.tga;*.psd)\0*.png;*.jpg;*.jpeg;*.tga;*.psd\0");
						
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
					std::string utilityPopupName = std::string("MaterialUtility") + "##" + std::to_string(texture->Handle) + relativePath;
					if (hovered && rightMouseButtonClicked)
					{
						Gui::OpenPopup(utilityPopupName.c_str());
					}
					if (texture != EditorResources::CheckerboardIcon && Gui::IsPopupOpen(utilityPopupName.c_str()) && Gui::BeginPopup(utilityPopupName.c_str()))
					{
						std::string remove = fmt::format("Remove##{} - {} - {}", texture->Handle, relativePath, i);
						if (Gui::MenuItem(remove.c_str()))
						{
							SetMaterialTexture(material, (AssetHandle)0, i);
							Gui::CloseCurrentPopup();
						}

						UI::Draw::Underline();

						std::string openInExplorer = fmt::format("Open in Explorer##{}##{}", texture->GetPath(), i);
						if (Gui::MenuItem(openInExplorer.c_str()))
						{
							FileDialogue::OpenInFileExplorer(texture->GetPath().c_str());
							Gui::CloseCurrentPopup();
						}

						Gui::EndPopup();
					}

					if (parameterCallback != nullptr)
						parameterCallback(material, i);

					UI::EndPropertyGrid();
				}
			}

			UI::BeginPropertyGrid();

			RenderMaterialProperties(material);

			UI::EndPropertyGrid();
		}

	}

	void MaterialEditorPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		Entity selected = SelectionManager::GetSelectedEntity();

		Gui::Begin("Material Editor", &s_ShowPanel);

		if (!selected)
		{
			Gui::End();
			return;
		}

		RenderMeshMaterial(selected);

		Gui::End();
	}

	void MaterialEditorPanel::RenderMeshMaterial(Entity selectedEntity)
	{
		if (!selectedEntity.HasAny<MeshRendererComponent, StaticMeshRendererComponent>())
			return;

		const ShaderLibrary& shaderLibrary = Renderer::GetShaderLibrary();
		size_t shaderCount = shaderLibrary.Size();
		static std::vector<const char*> shaderNames;

		if (shaderNames.size() < shaderCount)
		{
			shaderNames.clear();

			for (const auto& [shaderName, shader] : shaderLibrary)
			{
				shaderNames.emplace_back(shaderName.c_str());
			}
		}

		if (selectedEntity.HasComponent<MeshRendererComponent>())
		{
			AssetHandle meshHandle = selectedEntity.GetComponent<MeshRendererComponent>().Mesh;
			if (!AssetManager::IsHandleValid(meshHandle))
			{
				Gui::End();
				return;
			}

			SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
			if (!mesh)
			{
				Gui::End();
				return;
			}

			const auto& submesh = mesh->GetSubmesh();

			SharedReference<Material> material = submesh.GetMaterial();

			if (!material)
			{
				Gui::End();
				return;
			}

			const std::string& name = material->GetName() + " / " + submesh.GetName();

			if (UI::PropertyGridHeader(name.c_str()))
			{
				UI::BeginPropertyGrid();

				std::string currentShaderName = material->GetShader()->GetName();
				if (UI::PropertyDropdownSearch("Shader", shaderNames.data(), shaderNames.size(), currentShaderName, s_ShaderDropdownTextFilter))
				{
					if (shaderLibrary.Exists(currentShaderName))
					{
						SharedReference<Shader> shader = shaderLibrary.Get(currentShaderName);
						material->SetShader(shader);
					}
				}

				UI::EndPropertyGrid();

				Utils::RenderMaterialTexturesAndProperties(material, VX_BIND_CALLBACK(MaterialEditorPanel::ParameterCallback));

				UI::EndTreeNode();
			}
		}
		else if (selectedEntity.HasComponent<StaticMeshRendererComponent>())
		{
			auto& staticMeshRendererComponent = selectedEntity.GetComponent<StaticMeshRendererComponent>();
			AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
			if (!AssetManager::IsHandleValid(staticMeshHandle))
			{
				Gui::End();
				return;
			}

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
			if (!staticMesh)
			{
				Gui::End();
				return;
			}

			const auto& submeshes = staticMesh->GetSubmeshes();

			auto& materialTable = staticMeshRendererComponent.Materials;

			for (const auto& [submeshIndex, submesh] : submeshes)
			{
				VX_CORE_ASSERT(materialTable->HasMaterial(submeshIndex), "Material Table not synchronized with component!");

				AssetHandle materialHandle = materialTable->GetMaterial(submeshIndex);
				if (!AssetManager::IsHandleValid(materialHandle))
					continue;

				SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
				if (!material)
					continue;

				const std::string& name = material->GetName() + " / " + submesh.GetName();

				if (UI::PropertyGridHeader(name.c_str()))
				{
					UI::BeginPropertyGrid();

					std::string currentShaderName = material->GetShader()->GetName();
					if (UI::PropertyDropdownSearch("Shader", shaderNames.data(), shaderNames.size(), currentShaderName, s_ShaderDropdownTextFilter))
					{
						if (shaderLibrary.Exists(currentShaderName))
						{
							SharedReference<Shader> shader = shaderLibrary.Get(currentShaderName);
							material->SetShader(shader);
						}
					}

					UI::EndPropertyGrid();

					Utils::RenderMaterialTexturesAndProperties(material, VX_BIND_CALLBACK(MaterialEditorPanel::ParameterCallback));
					
					UI::EndTreeNode();
				}
			}

		}
	}

	void MaterialEditorPanel::ParameterCallback(SharedReference<Material>& material, uint32_t materialIndex)
	{
		switch (materialIndex)
		{
			case 0:
			{
				Math::vec3 albedo = material->GetAlbedo();
				if (UI::Property("Albedo", &albedo))
					material->SetAlbedo(albedo);

				break;
			}
			case 1:
			{
				break;
			}
			case 2:
			{
				float metallic = material->GetMetallic();
				if (UI::PropertySlider("Metallic", metallic, 0.01f, 1.0f))
					material->SetMetallic(metallic);

				break;
			}
			case 3:
			{
				float roughness = material->GetRoughness();
				if (UI::PropertySlider("Roughness", roughness, 0.01f, 1.0f))
					material->SetRoughness(roughness);

				break;
			}
			case 4:
			{
				float emission = material->GetEmission();
				if (UI::Property("Emission", emission))
					material->SetEmission(emission);

				break;
			}
			case 5:
			{
				float parallaxHeightScale = material->GetParallaxHeightScale();
				if (UI::Property("Height Scale", parallaxHeightScale, 0.01f, 0.01f, 1.0f))
					material->SetParallaxHeightScale(parallaxHeightScale);

				break;
			}
			case 6:
			{
				break;
			}
		}
	}

}
