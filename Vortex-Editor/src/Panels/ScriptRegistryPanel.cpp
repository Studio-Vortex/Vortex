#include "ScriptRegistryPanel.h"

#include <Vortex/Scripting/ScriptEngine.h>

namespace Vortex {

	void ScriptRegistryPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel)
		{
			Gui::Begin("Script Registry", &s_ShowPanel);

			std::vector<MonoAssemblyTypeInfo> coreAssemblyTypeInfo = ScriptEngine::GetCoreAssemblyTypeInfo();

			Gui::PushFont(boldFont);
			Gui::Text("Core Assembly Classes: %u", (uint32_t)coreAssemblyTypeInfo.size());
			Gui::PopFont();

			if (Gui::BeginTable("Registry", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				Gui::TableSetupColumn("Registered Class Name");
				Gui::TableSetupColumn("Field Count");
				Gui::TableHeadersRow();

				Gui::TableNextColumn();
				for (const auto& typeInfo : coreAssemblyTypeInfo)
					Gui::Text("%s.%s", typeInfo.Namespace, typeInfo.Name);

				Gui::TableNextColumn();
				for (const auto& typeInfo : coreAssemblyTypeInfo)
					Gui::Text("%u", typeInfo.FieldCount);

				Gui::EndTable();
			}

			Gui::End();
		}
	}

}
