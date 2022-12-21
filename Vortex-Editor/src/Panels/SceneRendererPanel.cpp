#include "SceneRendererPanel.h"

namespace Vortex {

	void SceneRendererPanel::OnGuiRender(bool showDefault)
	{
		if (s_ShowPanel || showDefault)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];
			auto largeFont = io.Fonts->Fonts[1];

			if (s_Loaded2DShaders.empty())
			{
				auto shaders2D = Renderer2D::GetShaderLibrary();

				auto it = std::unordered_map<std::string, SharedRef<Shader>>::iterator();

				for (it = shaders2D->begin(); it != shaders2D->end(); it++)
					s_Loaded2DShaders.push_back(it->second);
			}

			if (s_Loaded3DShaders.empty())
			{
				auto shaders3D = Renderer::GetShaderLibrary();

				auto it = std::unordered_map<std::string, SharedRef<Shader>>::iterator();

				for (it = shaders3D->begin(); it != shaders3D->end(); it++)
					s_Loaded3DShaders.push_back(it->second);
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

			Gui::Begin("Scene Renderer", &s_ShowPanel);

			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

			if (Gui::TreeNodeEx("Shadow Maps", treeNodeFlags))
			{
				Gui::Text("Sky Light");
				auto shadowMapID = Renderer::GetDepthMapFramebuffer()->GetDepthTextureRendererID();
				Gui::Image(reinterpret_cast<void*>(shadowMapID), { 256, 256 }, { 0, 1 }, { 1, 0 });

				Gui::TreePop();
			}

			if (Gui::TreeNodeEx("Shaders", treeNodeFlags))
			{
				Gui::PushFont(boldFont);
				Gui::Text("%u Loaded Shaders", (uint32_t)s_Loaded2DShaders.size() + (uint32_t)s_Loaded3DShaders.size());
				Gui::PopFont();

				Gui::SameLine();
				const char* buttonText = "Recompile All";
				Gui::SetCursorPosX(Gui::GetContentRegionAvail().x + (Gui::CalcTextSize(buttonText).x * 0.5f));
				if (Gui::Button(buttonText))
				{
					for (auto& shader : shaders)
						shader->Reload();
				}

				for (auto& shader : shaders)
				{
					Gui::Columns(2);

					const std::string& shaderName = shader->GetName();
					Gui::Text(shaderName.c_str());
					Gui::NextColumn();
					std::string buttonName = "Reload##" + shaderName;

					if (Gui::Button(buttonName.c_str()))
					{
						shader->Reload();
					}

					Gui::Columns(1);
				}

				Gui::TreePop();
			}

			Gui::End();
		}
	}

}
