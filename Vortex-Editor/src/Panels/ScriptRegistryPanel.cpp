#include "ScriptRegistryPanel.h"

#include <Vortex/Scripting/ScriptEngine.h>

namespace Vortex {

	void ScriptRegistryPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

		Gui::Begin(m_PanelName.c_str(), &IsOpen, flags);

		const ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		Math::vec2 tableSize = { contentRegionAvail.x, contentRegionAvail.y };

		tableSize.y -= Gui::CalcTextSize("Dummy Text").y * 1.25f;

		RenderScriptCoreTypedefInfo(tableSize);

		Gui::End();
	}

	void ScriptRegistryPanel::RenderScriptCoreTypedefInfo(const Math::vec2& size)
	{
		static std::vector<ScriptAssemblyTypedefInfo> scriptCoreTypedefInfo = ScriptEngine::GetCoreAssemblyTypeInfo();

		UI::PushFont("Bold");
		Gui::Text("Script Core Typedefs: %u", (uint32_t)scriptCoreTypedefInfo.size());
		UI::PopFont();

		static const char* columns[] = { "Registered Class Name", "Field Count" };

		UI::Table("Script Core Registry", columns, VX_ARRAYSIZE(columns), *(ImVec2*)&size, []()
		{
			Gui::TableNextColumn();
			for (const ScriptAssemblyTypedefInfo& typedefInfo : scriptCoreTypedefInfo)
			{
				UI::ShiftCursorX(10.0f);
				Gui::Text("%s.%s", typedefInfo.Namespace, typedefInfo.Name);
			}

			Gui::TableNextColumn();
			for (const ScriptAssemblyTypedefInfo& typedefInfo : scriptCoreTypedefInfo)
			{
				UI::ShiftCursorX(10.0f);
				Gui::Text("%u", typedefInfo.FieldCount);
			}
		});
	}

}
