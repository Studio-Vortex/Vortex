#include "MaterialViewerPanel.h"

namespace Vortex {

	static void RenderTexture(SharedRef<Texture2D> texture)
	{
		ImVec2 textureSize = { 112, 112 };
		Gui::ImageButton(reinterpret_cast<void*>(texture->GetRendererID()), textureSize, { 0, 1 }, { 1, 0 });

		Gui::Separator();
		Gui::Text("Path: %s", texture->GetPath().c_str());
		Gui::Text("Width: %u", texture->GetWidth());
		Gui::Text("Height: %u", texture->GetHeight());
		Gui::Separator();
	}

	void MaterialViewerPanel::OnGuiRender(Entity selectedEntity, bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		static SharedRef<Texture2D> checkerboardIcon = Texture2D::Create("Resources/Icons/Inspector/Checkerboard.png");

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Material Viewer", &s_ShowPanel);

			if (selectedEntity && selectedEntity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Model> model = selectedEntity.GetComponent<MeshRendererComponent>().Mesh;

				if (model)
				{
					SharedRef<Material> material = model->GetMaterial();

					if (material)
					{
						SharedRef<Texture2D> albedoMap = material->GetAlbedoMap();
						if (albedoMap)
						{
							RenderTexture(albedoMap);
						}
						else
						{
							RenderTexture(checkerboardIcon);
						}

						SharedRef<Texture2D> normalMap = material->GetNormalMap();
						if (normalMap)
						{
							RenderTexture(normalMap);
						}
						else
						{
							RenderTexture(checkerboardIcon);
						}

						SharedRef<Texture2D> metallicMap = material->GetMetallicMap();
						if (metallicMap)
						{
							RenderTexture(metallicMap);
						}
						else
						{
							RenderTexture(checkerboardIcon);
						}

						SharedRef<Texture2D> roughnessMap = material->GetRoughnessMap();
						if (roughnessMap)
						{
							RenderTexture(roughnessMap);
						}
						else
						{
							RenderTexture(checkerboardIcon);
						}

						SharedRef<Texture2D> ambientOcclusionMap = material->GetAmbientOcclusionMap();
						if (ambientOcclusionMap)
						{
							RenderTexture(ambientOcclusionMap);
						}
						else
						{
							RenderTexture(checkerboardIcon);
						}
					}
				}
			}
			
			Gui::End();
		}
	}

}
