#include "SceneRendererPanel.h"

namespace Vortex {

	void SceneRendererPanel::SetContext(const SharedRef<Scene>& context)
	{
		m_ContextScene = context;
	}

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

			if (UI::TreeNode("Shadow Maps", false))
			{
				Gui::Text("Sky Light");
				auto shadowMapID = Renderer::GetDepthMapFramebuffer()->GetDepthTextureRendererID();
				Gui::Image(reinterpret_cast<void*>(shadowMapID), { 256, 256 }, { 0, 1 }, { 1, 0 });

				Gui::TreePop();
			}

			if (UI::TreeNode("Shaders", false))
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

			if (UI::TreeNode("Renderer", false))
			{
				UI::BeginPropertyGrid();

				float lineWidth = Renderer2D::GetLineWidth();
				if (UI::Property("Line Width", lineWidth, 0.1f, 0.1f, 4.0f))
					Renderer2D::SetLineWidth(lineWidth);

				static const char* cullModes[4] = { "None", "Front", "Back", "Front And Back" };
				int32_t currentCullMode = (int32_t)Renderer::GetCullMode();

				SharedRef<Project> activeProject = Project::GetActive();
				ProjectProperties& projectProps = activeProject->GetProperties();

				if (UI::PropertyDropdown("Cull Mode", cullModes, VX_ARRAYCOUNT(cullModes), currentCullMode))
				{
					RendererAPI::TriangleCullMode newCullMode = (RendererAPI::TriangleCullMode)currentCullMode;
					Renderer::SetCullMode(newCullMode);
					projectProps.RendererProps.TriangleCullMode = Utils::TriangleCullModeToString(newCullMode);
				}

				auto RecreateEnvironmentMapFunc = [&]()
				{
					auto skyboxView = m_ContextScene->GetAllEntitiesWith<SkyboxComponent>();

					Entity entity{ skyboxView[0], m_ContextScene.get() };
					SkyboxComponent& skyboxComponent = entity.GetComponent<SkyboxComponent>();
					SharedRef<Skybox> skybox = skyboxComponent.Source;
					skybox->Reload();
					Renderer::CreateEnvironmentMap(skyboxComponent);
				};

				enum class EnvironmentMapResolution { e512, e1024, e2048 };
				static const char* envMapSizes[3] = { "512", "1024", "2048" };
				float envMapResolution = Renderer::GetEnvironmentMapResolution();
				
				int32_t currentEnvMapSize = 0;

				if (envMapResolution == 512.0f)  currentEnvMapSize = 0;
				if (envMapResolution == 1024.0f) currentEnvMapSize = 1;
				if (envMapResolution == 2048.0f) currentEnvMapSize = 2;

				if (UI::PropertyDropdown("Environment Map Resolution", envMapSizes, VX_ARRAYCOUNT(envMapSizes), currentEnvMapSize))
				{
					switch (currentEnvMapSize)
					{
						case 0: Renderer::SetEnvironmentMapResolution(512.0f);  break;
						case 1: Renderer::SetEnvironmentMapResolution(1024.0f); break;
						case 2: Renderer::SetEnvironmentMapResolution(2048.0f); break;
					}

					RecreateEnvironmentMapFunc();
				}

				enum class PrefilterMapResolution { e128, e256, e512 };
				static const char* prefilterMapSizes[3] = { "128", "256", "512" };
				float prefilterMapResolution = Renderer::GetPrefilterMapResolution();

				int32_t currentPrefilterMapSize = 0;

				if (prefilterMapResolution == 128.0f)  currentPrefilterMapSize = 0;
				if (prefilterMapResolution == 256.0f)  currentPrefilterMapSize = 1;
				if (prefilterMapResolution == 512.0f)  currentPrefilterMapSize = 2;

				if (UI::PropertyDropdown("Prefilter Map Resolution", prefilterMapSizes, VX_ARRAYCOUNT(prefilterMapSizes), currentPrefilterMapSize))
				{
					switch (currentPrefilterMapSize)
					{
						case 0: Renderer::SetPrefilterMapResolution(128.0f);  break;
						case 1: Renderer::SetPrefilterMapResolution(256.0f);  break;
						case 2: Renderer::SetPrefilterMapResolution(512.0f);  break;
					}

					RecreateEnvironmentMapFunc();
				}

				enum class ShadowMapResolution { e512, e1024, e2048, e4096, e8192 };
				static const char* shadowMapSizes[5] = { "512", "1024", "2048", "4096", "8192" };
				float shadowMapResolution = Renderer::GetShadowMapResolution();
				
				int32_t currentShadowMapSize = 0;

				if (shadowMapResolution == 512.0f)  currentShadowMapSize = 0;
				if (shadowMapResolution == 1024.0f) currentShadowMapSize = 1;
				if (shadowMapResolution == 2048.0f) currentShadowMapSize = 2;
				if (shadowMapResolution == 4096.0f) currentShadowMapSize = 3;
				if (shadowMapResolution == 8192.0f) currentShadowMapSize = 4;

				if (UI::PropertyDropdown("Shadow Map Resolution", shadowMapSizes, VX_ARRAYCOUNT(shadowMapSizes), currentShadowMapSize))
				{
					switch (currentShadowMapSize)
					{
						case 0: Renderer::SetShadowMapResolution(512.0f);  break;
						case 1: Renderer::SetShadowMapResolution(1024.0f); break;
						case 2: Renderer::SetShadowMapResolution(2048.0f); break;
						case 3: Renderer::SetShadowMapResolution(4096.0f); break;
						case 4: Renderer::SetShadowMapResolution(8192.0f); break;
					}

					LightSourceComponent skylight;
					auto lightSourceView = m_ContextScene->GetAllEntitiesWith<LightSourceComponent>();
					for (auto& e : lightSourceView)
					{
						Entity entity{ e, m_ContextScene.get() };
						if (const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>(); lightSourceComponent.Type == LightType::Directional)
							skylight = lightSourceComponent;
					}

					Renderer::CreateShadowMap(LightType::Directional, skylight.Source);
				}

				float sceneExposure = Renderer::GetSceneExposure();
				if (UI::Property("Exposure", sceneExposure, 0.01f, 0.01f, 1.0f))
					Renderer::SetSceneExposure(sceneExposure);

				float gamma = Renderer::GetSceneGamma();
				if (UI::Property("Gamma", gamma, 0.01f, 0.01f))
					Renderer::SetSceneGamma(gamma);

				static bool wireframeMode = false;
				if (UI::Property("Show Wireframe", wireframeMode))
					RenderCommand::SetWireframe(wireframeMode);

				static bool vsync = Application::Get().GetWindow().IsVSyncEnabled();
				if (UI::Property("Use VSync", vsync))
					Application::Get().GetWindow().SetVSync(vsync);

				UI::EndPropertyGrid();

				Gui::TreePop();
			}

			Gui::End();
		}
	}

}
