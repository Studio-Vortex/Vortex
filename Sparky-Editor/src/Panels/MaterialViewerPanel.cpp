#include "MaterialViewerPanel.h"

namespace Sparky {

	void MaterialViewerPanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Material Viewer", &s_ShowPanel);

			if (m_CurrentTexture)
			{
				ImVec2 textureSize = { 128, 128 };

				Gui::ImageButton(reinterpret_cast<void*>(m_CurrentTexture->GetRendererID()), textureSize, { 0, 1 }, { 1, 0 });

				Gui::Separator();

				Gui::Text("Path: %s", m_CurrentTexture->GetPath().c_str());
				Gui::Text("Width: %u", m_CurrentTexture->GetWidth());
				Gui::Text("Height: %u", m_CurrentTexture->GetHeight());
			}
			
			Gui::End();
		}
	}

	void MaterialViewerPanel::SetTexture(const SharedRef<Texture2D>& texture)
	{
		m_CurrentTexture = texture;
	}

}
