#include "AssetManagerPanel.h"

namespace Sparky {

	void AssetManagerPanel::OnGuiRender(bool showDefault)
	{
		if (s_ShowPanel || showDefault)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];
			auto largeFont = io.Fonts->Fonts[1];

			if (s_Loaded2DShaders.empty())
			{
				auto shaders2D = Renderer2D::GetLoadedShaders();

				for (const auto& shader : shaders2D)
					s_Loaded2DShaders.push_back(shader);
			}

			if (s_Loaded3DShaders.empty())
			{
				auto shaders3D = Renderer::GetLoadedShaders();

				for (const auto& shader : shaders3D)
					s_Loaded3DShaders.push_back(shader);
			}

			static std::vector<std::string> shaderNames;
			static std::vector<SharedRef<Shader>> shaders;

			bool shadersLoaded = !s_Loaded2DShaders.empty() && !s_Loaded3DShaders.empty();

			if (shadersLoaded)
			{
				if (shaderNames.empty() && shaders.empty())
				{
					for (const auto& shader : s_Loaded2DShaders)
					{
						shaderNames.push_back(shader->GetName());
						shaders.push_back(shader);
					}
					for (const auto& shader : s_Loaded3DShaders)
					{
						shaderNames.push_back(shader->GetName());
						shaders.push_back(shader);
					}
				}
			}

			Gui::Begin("Asset Manager");

			Gui::PushFont(boldFont);
			Gui::Text("%u Loaded Shaders", (uint32_t)s_Loaded2DShaders.size() + (uint32_t)s_Loaded3DShaders.size());
			Gui::PopFont();

			if (Gui::BeginTable("Shaders", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				Gui::TableSetupColumn("Shader");
				Gui::TableSetupColumn("Recompile");
				Gui::TableHeadersRow();

				uint32_t offset = 80;
				Gui::TableNextColumn();
				for (const auto& shaderName : shaderNames)
				{
					Gui::SetCursorPosY((float)offset);
					Gui::Text(shaderName.c_str());
					offset += 28;
				}

				Gui::TableNextColumn();
				for (auto& shader : shaders)
				{
					if (shader->GetName() != "Renderer_Model") continue;
					if (Gui::Button("Recompile"))
						shader->ReCompile();
				}

				Gui::EndTable();
			}

			Gui::End();
		}
	}

}
