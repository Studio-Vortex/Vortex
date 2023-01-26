#pragma once

#include <Vortex/Editor/EditorPanel.h>

#include "Panels/ProjectSettingsPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ScriptRegistryPanel.h"
#include "Panels/MaterialEditorPanel.h"
#include "Panels/AssetRegistryPanel.h"
#include "Panels/BuildSettingsPanel.h"
#include "Panels/SceneRendererPanel.h"
#include "Panels/ShaderEditorPanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/AboutPanel.h"

#include <unordered_map>

namespace Vortex {

	struct PanelData
	{
		Entity SelectedEntity;
		size_t EntityCount;
	};

	class PanelManager
	{
	public:
		bool HasPanel(const char* panelID)
		{
			return m_Panels.contains(panelID);
		}

		template <typename TPanel, typename... TArgs>
		SharedRef<TPanel> RegisterPanel(const char* panelID, TArgs&&... args)
		{
			if (HasPanel(panelID))
			{
				VX_CORE_WARN("Panel Manager already has panel with id '{}'", panelID);
				return nullptr;
			}

			SharedRef<TPanel> panel = SharedRef<TPanel>::Create(std::forward<TArgs>(args)...);
			m_Panels.insert({ panelID, panel }); // TODO hash panel id
			return panel;
		}

		template <typename TPanel>
		SharedRef<TPanel> GetPanel(const char* panelID)
		{
			VX_CORE_ASSERT(HasPanel(panelID), "Panel Manager doesn't contain panel!");
			return m_Panels[panelID].As<TPanel>();
		}

		void RenderPanels(const PanelData& data)
		{
			m_Panels.find("Asset Registry")->second.As<AssetRegistryPanel>()->OnGuiRender();
			m_Panels.find("Console")->second.As<ConsolePanel>()->OnGuiRender();
			m_Panels.find("Content Browser")->second.As<ContentBrowserPanel>()->OnGuiRender();
			m_Panels.find("Material Editor")->second.As<MaterialEditorPanel>()->OnGuiRender(data.SelectedEntity);
			m_Panels.find("Performance")->second.As<PerformancePanel>()->OnGuiRender(data.EntityCount);
			m_Panels.find("Scene Renderer")->second.As<SceneRendererPanel>()->OnGuiRender();
			m_Panels.find("Script Registry")->second.As<ScriptRegistryPanel>()->OnGuiRender();
			m_Panels.find("Shader Editor")->second.As<ShaderEditorPanel>()->OnGuiRender();
			m_Panels.find("Build Settings")->second.As<BuildSettingsPanel>()->OnGuiRender();
			m_Panels.find("Project Settings")->second.As<ProjectSettingsPanel>()->OnGuiRender();
			m_Panels.find("About")->second.As<AboutPanel>()->OnGuiRender();
		}

		template <typename TPanel>
		void RenderMenuItem(const char* panelID, bool useSeparator = true)
		{
			Gui::MenuItem(panelID, nullptr, &m_Panels.find(panelID)->second.As<TPanel>()->IsOpen());
			if (useSeparator)
				Gui::Separator();
		}

		void RenderMenuButtons(bool& scenePanelOpen)
		{
			RenderMenuItem<AssetRegistryPanel>("Asset Registry");
			RenderMenuItem<ConsolePanel>("Console");
			RenderMenuItem<ContentBrowserPanel>("Content Browser");

			RenderMenuItem<MaterialEditorPanel>("Material Editor");
			RenderMenuItem<PerformancePanel>("Performance");

			const char* scenePanelID = "Scene";
			Gui::MenuItem(scenePanelID, nullptr, &scenePanelOpen);
			Gui::Separator();

			RenderMenuItem<SceneRendererPanel>("Scene Renderer");
			RenderMenuItem<ScriptRegistryPanel>("Script Registry");
			RenderMenuItem<ShaderEditorPanel>("Shader Editor");
			RenderMenuItem<BuildSettingsPanel>("Build Settings");
			RenderMenuItem<ProjectSettingsPanel>("Project Settings", false);
		}

		void SetProjectContext(SharedRef<Project> project)
		{
			for (auto& [panelID, panel] : m_Panels)
			{
				panel->SetProjectContext(project);
			}
		}

		void SetSceneContext(SharedRef<Scene> scene)
		{
			for (auto& [panelID, panel] : m_Panels)
			{
				panel->SetSceneContext(scene);
			}
		}

	private:
		std::unordered_map<const char*, SharedRef<EditorPanel>> m_Panels;
	};

}
