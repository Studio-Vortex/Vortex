#include "SceneRendererPanel.h"

namespace Vortex {

	void SceneRendererPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		if (s_Loaded2DShaders.empty())
		{
			const ShaderLibrary& shaders2D = Renderer2D::GetShaderLibrary();

			for (const auto& [name, shader] : shaders2D)
			{
				s_Loaded2DShaders.push_back(shader);
			}
		}

		if (s_Loaded3DShaders.empty())
		{
			const ShaderLibrary& shaders3D = Renderer::GetShaderLibrary();

			for (const auto& [name, shader] : shaders3D)
			{
				s_Loaded3DShaders.push_back(shader);
			}
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

		if (UI::PropertyGridHeader("Shadow Maps", false))
		{
			Gui::Text("Sky Light");
			auto shadowMapID = Renderer::GetSkyLightDepthFramebuffer()->GetDepthTextureRendererID();
			Gui::Image(reinterpret_cast<void*>(shadowMapID), { 256, 256 }, { 0, 1 }, { 1, 0 });

			UI::EndTreeNode();
		}

		if (UI::PropertyGridHeader("Shaders", false))
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

			static const char* columns[] = { "Name", "" };

			UI::Table("Loaded Shaders", columns, VX_ARRAYCOUNT(columns), Gui::GetContentRegionAvail(), [&]()
			{
				for (auto& shader : shaders)
				{
					Gui::TableNextColumn();
					const std::string& shaderName = shader->GetName();
					Gui::Text(shaderName.c_str());
					UI::Draw::Underline();

					Gui::TableNextColumn();
					std::string buttonName = "Reload##" + shaderName;

					if (Gui::Button(buttonName.c_str()))
					{
						shader->Reload();
					}
				}
			});

			UI::EndTreeNode();
		}

		if (UI::PropertyGridHeader("Renderer", false))
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

				for (const auto e : skyboxView)
				{
					Entity entity{ e, m_ContextScene.Raw() };

					SkyboxComponent& skyboxComponent = entity.GetComponent<SkyboxComponent>();
					AssetHandle environmentHandle = skyboxComponent.Skybox;
					if (!AssetManager::IsHandleValid(environmentHandle))
						continue;

					SharedReference<Skybox> environment = AssetManager::GetAsset<Skybox>(environmentHandle);
					if (!environment)
						continue;

					environment->SetShouldReload(true);
					Renderer::CreateEnvironmentMap(skyboxComponent, environment);
					break;
				}
			};

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
				for (const auto e : lightSourceView)
				{
					Entity entity{ e, m_ContextScene.Raw() };
					const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
					
					if (lightSourceComponent.Type != LightType::Directional)
						continue;

					skylight = lightSourceComponent;
				}

				Renderer::CreateShadowMap(LightType::Directional, skylight.Source);
			}

			float sceneExposure = Renderer::GetSceneExposure();
			if (UI::Property("Exposure", sceneExposure, 0.01f, 0.01f))
			{
				Renderer::SetSceneExposure(sceneExposure);
			}

			float gamma = Renderer::GetSceneGamma();
			if (UI::Property("Gamma", gamma, 0.01f, 0.01f))
			{
				Renderer::SetSceneGamma(gamma);
			}

			static bool wireframeMode = false;
			if (UI::Property("Show Wireframe", wireframeMode))
			{
				RenderCommand::SetWireframe(wireframeMode);
			}

			static bool vsync = Application::Get().GetWindow().IsVSyncEnabled();
			if (UI::Property("Use VSync", vsync))
			{
				Application::Get().GetWindow().SetVSync(vsync);
			}

			UI::EndPropertyGrid();

			if (UI::PropertyGridHeader("Bloom", false))
			{
				UI::BeginPropertyGrid();

				bool bloomEnabled = Renderer::IsFlagSet(RenderFlag::EnableBloom);
				if (UI::Property("Enable Bloom", bloomEnabled))
				{
					Renderer::ToggleFlag(RenderFlag::EnableBloom);
				}

				if (bloomEnabled)
				{
					Math::vec3 bloomSettings = Renderer::GetBloomSettings();
					bool modified = false;
					if (UI::Property("Threshold", bloomSettings.x))
						modified = true;
					if (UI::Property("Soft Knee", bloomSettings.y))
						modified = true;
					if (UI::Property("Unknown", bloomSettings.z))
						modified = true;

					if (modified)
					{
						Renderer::SetBloomSettings(bloomSettings);
					}

					static const char* bloomBlurSampleSizes[] = { "5", "10", "15", "20", "40" };
					uint32_t bloomSampleSize = Renderer::GetBloomSampleSize();

					uint32_t currentBloomBlurSamplesSize = 0;

					if (bloomSampleSize == 5)  currentBloomBlurSamplesSize = 0;
					if (bloomSampleSize == 10) currentBloomBlurSamplesSize = 1;
					if (bloomSampleSize == 15) currentBloomBlurSamplesSize = 2;
					if (bloomSampleSize == 20) currentBloomBlurSamplesSize = 3;
					if (bloomSampleSize == 40) currentBloomBlurSamplesSize = 4;

					if (UI::PropertyDropdown("Bloom Blur Samples", bloomBlurSampleSizes, VX_ARRAYCOUNT(bloomBlurSampleSizes), currentBloomBlurSamplesSize))
					{
						switch (currentBloomBlurSamplesSize)
						{
							case 0: Renderer::SetBloomSampleSize(5);  break;
							case 1: Renderer::SetBloomSampleSize(10); break;
							case 2: Renderer::SetBloomSampleSize(15); break;
							case 3: Renderer::SetBloomSampleSize(20); break;
							case 4: Renderer::SetBloomSampleSize(40); break;
						}
					}
				}

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}

			UI::EndTreeNode();
		}

		Gui::End();
	}

    void SceneRendererPanel::SetSceneContext(const SharedReference<Scene>& scene)
    {
		m_ContextScene = scene;
    }

}
