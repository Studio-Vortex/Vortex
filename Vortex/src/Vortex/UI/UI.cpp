#include "vxpch.h"
#include "UI.h"

#include <imgui.h>

namespace Vortex::UI {

    ScopedStyle::ScopedStyle(ImGuiStyleVar var, ImVec2 value)
    {
		Gui::PushStyleVar(var, value);
    }

    ScopedStyle::ScopedStyle(ImGuiStyleVar var, float value)
    {
		Gui::PushStyleVar(var, value);
    }

    ScopedStyle::~ScopedStyle()
    {
		Gui::PopStyleVar();
    }

	ScopedColor::ScopedColor(ImGuiCol col, ImVec4 color)
	{
		Gui::PushStyleColor(col, color);
	}

	ScopedColor::~ScopedColor()
	{
		Gui::PopStyleColor();
	}

}
