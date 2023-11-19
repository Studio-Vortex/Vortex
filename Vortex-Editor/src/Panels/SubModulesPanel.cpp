#include "SubModulesPanel.h"

namespace Vortex {

	void SubModulesPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		const ModuleLibrary& moduleLib = Application::Get().GetModules();
		Gui::PushFont(boldFont);
		Gui::Text("Modules Loaded: %u", (uint32_t)moduleLib.Size());
		Gui::PopFont();

		static ModuleResult res = moduleLib.ResolveModules();

		Gui::Text("Resolved Modules: ");
		Gui::SameLine();
		ImVec4 color = (res.Success ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		Gui::TextColored(color, (const char*)(res.Success ? VX_ICON_CHECK : VX_ICON_TIMES));

		if (!res.Success)
		{
			Gui::TextColored(color, (const char*)VX_ICON_EXCLAMATION);
			Gui::SameLine();
			Gui::Text(" %u unresolved module reference", res.UnresolvedModules);
		}

		UI::Draw::Underline();

		for (const auto& module : moduleLib)
		{
			RenderModule(module);
		}

		Gui::End();
	}

	void SubModulesPanel::RenderModule(const SubModule& submodule)
	{
		std::string moduleName = submodule.GetName();
		Gui::Text("Module Name: %s", moduleName.c_str());
		Version apiVersion = submodule.GetAPIVersion();
		std::string versionStr = apiVersion.ToString();
		Gui::Text("API Version: %s", versionStr.c_str());
		std::string headerName = std::string("Required Modules##") + moduleName;
		if (UI::PropertyGridHeader(headerName.c_str(), false))
		{
			const std::vector<std::string>& requiredModules = submodule.GetRequiredModules();
			
			if (requiredModules.empty())
			{
				Gui::Text("None");
			}
			else
			{
				uint32_t i = 1;
				for (const auto& requiredModule : requiredModules)
				{
					Gui::Text("%u. %s", i++, requiredModule.c_str());
				}
			}

			UI::EndTreeNode();
		}
		UI::Draw::Underline();
	}

}
