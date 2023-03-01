#include "MaterialEditorPanel.h"

#include <Vortex/Editor/EditorResources.h>

namespace Vortex {

	static ImGuiTextFilter s_ShaderDropdownTextFilter;

	namespace Utils {

		static SharedRef<Texture2D> GetMaterialTexture(const SharedRef<Material>& material, uint32_t index)
		{
			VX_CORE_ASSERT(index <= 6, "Index out of bounds!");

			switch (index)
			{
				case 0: return material->GetAlbedoMap();
				case 1: return material->GetNormalMap();
				case 2: return material->GetMetallicMap();
				case 3: return material->GetRoughnessMap();
				case 4: return material->GetEmissionMap();
				case 5: return material->GetParallaxOcclusionMap();
				case 6: return material->GetAmbientOcclusionMap();
			}
		}

		static void SetMaterialTexture(SharedRef<Material> material, const SharedRef<Texture2D>& texture, uint32_t index)
		{
			VX_CORE_ASSERT(index <= 6, "Index out of bounds!");

			switch (index)
			{
				case 0: material->SetAlbedoMap(texture);            break;
				case 1: material->SetNormalMap(texture);            break;
				case 2: material->SetMetallicMap(texture);          break;
				case 3: material->SetRoughnessMap(texture);         break;
				case 4: material->SetEmissionMap(texture);          break;
				case 5: material->SetParallaxOcclusionMap(texture); break;
				case 6: material->SetAmbientOcclusionMap(texture);  break;
			}
		}

		static void RenderMaterialFlags(const SharedRef<Material>& material)
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

		static void RenderMaterialProperties(const SharedRef<Material>& material)
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

		using MaterialParameterCallbackFunc = const std::function<void(const SharedRef<Material>&, uint32_t)>&;
		static void RenderMaterialTexturesAndProperties(const SharedRef<Material>& material, MaterialParameterCallbackFunc parameterCallback)
		{
			static const char* displayNames[] = {
				"Albedo", "Normal", "Metallic", "Roughness", "Emission", "Parallax Occlusion", "Ambient Occlusion"
			};

			ImVec2 textureSize = { 64, 64 };
			ImVec4 bgColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			Math::vec3 albedo = material->GetAlbedo();
			ImVec4 albedoTint = { albedo.r, albedo.g, albedo.b, 1.0f };

			uint32_t count = VX_ARRAYCOUNT(displayNames);

			for (uint32_t i = 0; i < count; i++)
			{
				SharedRef<Texture2D> texture = nullptr;
				if (SharedRef<Texture2D> entry = GetMaterialTexture(material, i))
				{
					texture = entry;
				}
				else
				{
					texture = EditorResources::CheckerboardIcon;
				}

				bool hovered = false;
				bool leftMouseButtonClicked = false;
				bool rightMouseButtonClicked = false;

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

					UI::SetTooltip(texture->GetPath().c_str());

					if (Gui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path texturePath = std::filesystem::path(path);

							// Make sure we are recieving an actual texture otherwise we will have trouble opening it
							if (texturePath.filename().extension() == ".png" || texturePath.filename().extension() == ".jpg" || texturePath.filename().extension() == ".tga" || texturePath.filename().extension() == ".psd")
							{
								TextureProperties imageProps;
								imageProps.Filepath = texturePath.string();
								imageProps.WrapMode = ImageWrap::Repeat;

								SharedRef<Texture2D> newTexture = Texture2D::Create(imageProps);

								if (newTexture->IsLoaded())
								{
									SetMaterialTexture(material, newTexture, i);
								}
								else
								{
									VX_WARN("Could not load texture {}", texturePath.filename().string());
								}
							}
							else
							{
								VX_WARN("Could not load texture, not a '.png', '.jpg', '.tga' or '.psd' - {}", texturePath.filename().string());
							}
						}

						Gui::EndDragDropTarget();
					}

					if (hovered && leftMouseButtonClicked)
					{
						std::string filepath = FileDialogue::OpenFileDialog("Texture File (*.png;*.jpg;*.tga;*.psd)\0*.png;*.jpg;*.tga;*.psd\0");
						
						if (!filepath.empty())
						{
							std::string projectDir = Project::GetProjectDirectory().string();
							std::string relativePath = FileSystem::Relative(filepath, projectDir).string();
							
							TextureProperties imageProps;
							imageProps.Filepath = relativePath;
							imageProps.WrapMode = ImageWrap::Repeat;

							SetMaterialTexture(material, Texture2D::Create(imageProps), i);
						}
					}

					// right click for utilities
					if (hovered && rightMouseButtonClicked)
					{
						Gui::OpenPopup("MaterialUtility");
					}
					if (texture && texture != EditorResources::CheckerboardIcon && Gui::BeginPopup("MaterialUtility"))
					{
						std::string remove = fmt::format("Remove##{}##{}", texture->GetPath(), i);
						if (Gui::MenuItem(remove.c_str()))
						{
							SetMaterialTexture(material, nullptr, i);
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

	void MaterialEditorPanel::OnGuiRender(Entity selectedEntity)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		Gui::Begin("Material Editor", &s_ShowPanel);

		if (!selectedEntity)
		{
			Gui::End();
			return;
		}

		const ShaderLibrary& shaderLibrary = *Renderer::GetShaderLibrary();
		std::vector<const char*> shaderNames;

		for (const auto& [name, shader] : shaderLibrary)
		{
			shaderNames.emplace_back(name.c_str());
		}

		if (selectedEntity.HasComponent<MeshRendererComponent>())
		{
			SharedRef<Mesh> mesh = selectedEntity.GetComponent<MeshRendererComponent>().Mesh;

			if (!mesh)
			{
				Gui::End();
				return;
			}

			const auto& submesh = mesh->GetSubmesh();

			SharedRef<Material> material = submesh.GetMaterial();

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
						SharedRef<Shader> shader = shaderLibrary.Get(currentShaderName);
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
			SharedRef<StaticMesh> staticMesh = selectedEntity.GetComponent<StaticMeshRendererComponent>().StaticMesh;

			if (!staticMesh)
			{
				Gui::End();
				return;
			}

			const auto& submeshes = staticMesh->GetSubmeshes();

			for (const auto& submesh : submeshes)
			{
				SharedRef<Material> material = submesh.GetMaterial();

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
							SharedRef<Shader> shader = shaderLibrary.Get(currentShaderName);
							material->SetShader(shader);
						}
					}

					UI::EndPropertyGrid();

					Utils::RenderMaterialTexturesAndProperties(material, VX_BIND_CALLBACK(MaterialEditorPanel::ParameterCallback));

					UI::EndTreeNode();
				}
			}
		}

		Gui::End();
	}

	void MaterialEditorPanel::ParameterCallback(SharedRef<Material> material, uint32_t materialIndex)
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
