#pragma once

#include "Vortex/Core/Math.h"

#include <imgui_internal.h>

namespace Vortex::UI {

	namespace Gui = ImGui;

	inline static ImRect RectExpanded(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	inline static void SetTooltip(const char* message)
	{
		if (!Gui::IsItemHovered())
			return;

		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5.0f, 5.0f });
		Gui::BeginTooltip();
		Gui::Text(message);
		Gui::EndTooltip();
		Gui::PopStyleVar();
	}

	class ScopedStyle
	{
	public:
		ScopedStyle(ImGuiStyleVar var, ImVec2 value);
		ScopedStyle(ImGuiStyleVar var, float value);
		~ScopedStyle();
	};

	class ScopedColor
	{
	public:
		ScopedColor(ImGuiCol col, ImVec4 color);
		~ScopedColor();
	};

}
