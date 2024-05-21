#include "SceneRendererPanel.h"

namespace Vortex {

	void SceneRendererPanel::OnPanelAttach()
	{
		const ShaderLibrary& shaderLibrary2D = Renderer2D::GetShaderLibrary();

		for (const auto& [name, shader] : shaderLibrary2D)
		{
			m_Loaded2DShaders.push_back(shader);
		}

		const ShaderLibrary& shaderLibrary3D = Renderer::GetShaderLibrary();

		for (const auto& [name, shader] : shaderLibrary3D)
		{
			m_Loaded3DShaders.push_back(shader);
		}

		for (const auto& shader : m_Loaded2DShaders)
		{
			m_ShaderNames.push_back(shader->GetName());
			m_Shaders.push_back(shader);
		}
		for (const auto& shader : m_Loaded3DShaders)
		{
			m_ShaderNames.push_back(shader->GetName());
			m_Shaders.push_back(shader);
		}
	}

	void SceneRendererPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

		Gui::Begin(m_PanelName.c_str(), &IsOpen, flags);

		if (Actor skyLightEntity = m_ContextScene->GetSkyLightActor())
		{
			const LightSourceComponent& lsc = skyLightEntity.GetComponent<LightSourceComponent>();
			if (lsc.CastShadows)
			{
				if (UI::PropertyGridHeader("Shadow Maps", false))
				{
					Gui::Text("Sky Light");
					auto shadowMapID = Renderer::GetSkyLightDepthFramebuffer()->GetDepthTextureRendererID();
					Gui::Image(reinterpret_cast<void*>(shadowMapID), { 256, 256 }, { 0, 1 }, { 1, 0 });

					UI::EndTreeNode();
				}
			}
		}

		if (UI::PropertyGridHeader("Shaders", false))
		{
			UI::PushFont("Bold");
			Gui::Text("%u Loaded Shaders", (uint32_t)m_Loaded2DShaders.size() + (uint32_t)m_Loaded3DShaders.size());
			UI::PopFont();

			Gui::SameLine();
			const char* buttonText = "Recompile All";
			Gui::SetCursorPosX(Gui::GetContentRegionAvail().x + (Gui::CalcTextSize(buttonText).x * 0.5f));
			if (Gui::Button(buttonText))
			{
				for (auto& shader : m_Shaders)
				{
					shader->Reload();
				}
			}

			static const char* columns[] = { "Name", "" };

			UI::Table("Loaded Shaders", columns, VX_ARRAYSIZE(columns), Gui::GetContentRegionAvail(), [&]()
			{
				for (SharedReference<Shader>& shader : m_Shaders)
				{
					Gui::TableNextColumn();
					const std::string& shaderName = shader->GetName();
					Gui::Text(shaderName.c_str());
					UI::Draw::Underline();

					Gui::TableNextColumn();
					const std::string buttonName = "Reload##" + shaderName;

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

			SharedReference<Project> project = Project::GetActive();
			ProjectProperties& properties = project->GetProperties();

			if (UI::Property("Line Width", properties.RendererProps.LineWidth, 0.01f, FLT_MIN, 4.0f))
			{
				Renderer2D::SetLineWidth(properties.RendererProps.LineWidth);
			}

			static const char* cullModes[4] = { "None", "Front", "Back", "Front And Back" };
			int32_t currentCullMode = (int32_t)Renderer::GetCullMode();

			if (UI::PropertyDropdown("Cull Mode", cullModes, VX_ARRAYSIZE(cullModes), currentCullMode))
			{
				RendererAPI::TriangleCullMode newCullMode = (RendererAPI::TriangleCullMode)currentCullMode;
				Renderer::SetCullMode(newCullMode);
				properties.RendererProps.TriangleCullMode = Utils::TriangleCullModeToString(newCullMode);
			}

			auto RecreateEnvironmentMapFunc = [&]()
			{
				auto skyboxView = m_ContextScene->GetAllActorsWith<SkyboxComponent>();

				for (const auto e : skyboxView)
				{
					Actor entity{ e, m_ContextScene.Raw() };

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

			if (UI::PropertyDropdown("Environment Map Resolution", envMapSizes, VX_ARRAYSIZE(envMapSizes), currentEnvMapSize))
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

			if (UI::PropertyDropdown("Prefilter Map Resolution", prefilterMapSizes, VX_ARRAYSIZE(prefilterMapSizes), currentPrefilterMapSize))
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

			if (UI::PropertyDropdown("Shadow Map Resolution", shadowMapSizes, VX_ARRAYSIZE(shadowMapSizes), currentShadowMapSize))
			{
				switch (currentShadowMapSize)
				{
					case 0: Renderer::SetShadowMapResolution(512.0f);  break;
					case 1: Renderer::SetShadowMapResolution(1024.0f); break;
					case 2: Renderer::SetShadowMapResolution(2048.0f); break;
					case 3: Renderer::SetShadowMapResolution(4096.0f); break;
					case 4: Renderer::SetShadowMapResolution(8192.0f); break;
				}

				Renderer::CreateShadowMap(LightType::Directional);
			}

			float sceneExposure = Renderer::GetSceneExposure();
			if (UI::Property("Exposure", sceneExposure, 0.01f, FLT_MIN, FLT_MAX))
			{
				Renderer::SetSceneExposure(sceneExposure);
			}

			float gamma = Renderer::GetSceneGamma();
			if (UI::Property("Gamma", gamma, 0.01f, FLT_MIN, FLT_MAX))
			{
				Renderer::SetSceneGamma(gamma);
			}

			float maxReflectionLOD = Renderer::GetMaxReflectionLOD();
			if (UI::Property("Max Reflection LOD", maxReflectionLOD, 1.0f, FLT_MIN, FLT_MAX))
			{
				Renderer::SetMaxReflectionLOD(maxReflectionLOD);
			}

			UI::EndPropertyGrid();

			if (UI::PropertyGridHeader("Fog", false))
			{
				UI::BeginPropertyGrid();

				bool fogEnabled = Renderer::GetFogEnabled();
				if (UI::Property("Fog Enabled", fogEnabled))
				{
					Renderer::SetFogEnabled(fogEnabled);
				}

				float fogDensity = Renderer::GetFogDensity();
				if (UI::Property("Density", fogDensity, 0.001f, FLT_MIN, FLT_MAX))
				{
					Renderer::SetFogDensity(fogDensity);
				}

				float fogGradient = Renderer::GetFogGradient();
				if (UI::Property("Gradient", fogGradient, 0.01f, FLT_MIN, FLT_MAX))
				{
					Renderer::SetFogGradient(fogGradient);
				}

				UI::EndPropertyGrid();
				UI::EndTreeNode();
			}

			UI::BeginPropertyGrid();

			bool showNormals = Renderer::GetShowNormals();
			if (UI::Property("Show Normals", showNormals))
			{
				Renderer::SetShowNormals(showNormals);
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
			UI::EndTreeNode();
		}

		Gui::End();
	}

}
