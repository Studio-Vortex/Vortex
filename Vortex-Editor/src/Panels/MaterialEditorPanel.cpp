#include "MaterialEditorPanel.h"

namespace Vortex {

	namespace Utils {

		static void RenderTextureButton(SharedRef<Texture2D> texture)
		{
			static SharedRef<Texture2D> checkerboardIcon = Texture2D::Create("Resources/Icons/Inspector/Checkerboard.png");

			SharedRef<Texture2D> tex = texture ? texture : checkerboardIcon;

			ImVec2 textureSize = { 112, 112 };
			Gui::ImageButton(reinterpret_cast<void*>(tex->GetRendererID()), textureSize, { 0, 1 }, { 1, 0 });

			Gui::Separator();
			Gui::Text("Path: %s", tex->GetPath().c_str());
			Gui::Text("Width: %u", tex->GetWidth());
			Gui::Text("Height: %u", tex->GetHeight());
			Gui::Separator();
		}

	}

	void MaterialEditorPanel::OnGuiRender(Entity selectedEntity, bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Material Editor", &s_ShowPanel);

			if (selectedEntity && selectedEntity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Model> model = selectedEntity.GetComponent<MeshRendererComponent>().Mesh;

				if (model)
				{
					SharedRef<Material> material = model->GetMaterial();

					if (!material)
					{
						Gui::End();
						return;
					}

					SharedRef<Texture2D> albedoMap = material->GetAlbedoMap();
					Utils::RenderTextureButton(albedoMap);

					SharedRef<Texture2D> normalMap = material->GetNormalMap();
					Utils::RenderTextureButton(normalMap);

					SharedRef<Texture2D> metallicMap = material->GetMetallicMap();
					Utils::RenderTextureButton(metallicMap);

					SharedRef<Texture2D> roughnessMap = material->GetRoughnessMap();
					Utils::RenderTextureButton(roughnessMap);

					SharedRef<Texture2D> ambientOcclusionMap = material->GetAmbientOcclusionMap();
					Utils::RenderTextureButton(ambientOcclusionMap);
				}
			}
			
			Gui::End();
		}
	}

}
