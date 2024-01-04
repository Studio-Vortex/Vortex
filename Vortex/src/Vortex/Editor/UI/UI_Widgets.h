#pragma once

#include "Vortex/Renderer/Texture.h"

#include "Vortex/Editor/UI/UI.h"

namespace ImGui {

	void TextCentered(const char* text, float y = 0.0f, ...);
	bool SelectableWithImage(const char* label, bool selected, Vortex::SharedReference<Vortex::Texture2D> image, ImVec2 imageSize, ImVec4 imageTintColor, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0));

}