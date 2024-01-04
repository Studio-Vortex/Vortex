#include "vxpch.h"
#include "UI.h"

#include "Vortex/Editor/UI/UI_Widgets.h"

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

    VORTEX_API bool PropertyDropdownSearchWithImages(const char* label, const char** options, uint32_t count, const std::vector<UIImage>& images, std::string& selected, const UIImage& selectedImage, ImGuiTextFilter& textFilter, const std::function<void()>& clearCallback, const char* desc)
    {
		const char* current = selected.c_str();

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}

		Gui::SameLine();
		SharedReference<Texture2D> texture = selectedImage.Texture;
		const ImVec2 imageSize = *(ImVec2*)&selectedImage.Size;
		const ImVec4 imageTintColor = *(ImVec4*)&selectedImage.TintColor;
		UI::ImageEx(texture->GetRendererID(), imageSize, imageTintColor);
		UI::SetTooltip(Fs::Path(texture->GetPath()).stem().string().c_str());

		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		bool modified = false;

		const std::string id = "##" + std::string(label);
		if (Gui::BeginCombo(id.c_str(), current))
		{
			const bool isSearching = Gui::InputTextWithHint(id.c_str(), "Search", textFilter.InputBuf, IM_ARRAYSIZE(textFilter.InputBuf));
			DrawItemActivityOutline();

			if (clearCallback != nullptr)
			{
				Gui::SameLine();
				if (Gui::Button("Clear"))
				{
					std::invoke(clearCallback);
				}
			}

			if (isSearching)
			{
				textFilter.Build();
			}

			UI::Draw::Underline();

			for (uint32_t i = 0; i < count; i++)
			{
				const bool isSelected = current == options[i];

				if (!textFilter.PassFilter(options[i]))
					continue;

				SharedReference<Texture2D> image = images[i].Texture;
				const ImVec2 imageSize = *(ImVec2*)&images[i].Size;
				const ImVec4 imageTintColor = *(ImVec4*)&images[i].TintColor;

				if (Gui::SelectableWithImage(options[i], isSelected, image, imageSize, imageTintColor) || (Gui::IsItemFocused() && Gui::IsKeyPressed(ImGuiKey_Enter)))
				{
					current = options[i];
					selected = options[i];
					modified = true;

					memset(textFilter.InputBuf, 0, IM_ARRAYSIZE(textFilter.InputBuf));
					textFilter.Build();
					Gui::CloseCurrentPopup();
				}

				if (isSelected)
				{
					Gui::SetItemDefaultFocus();
				}

				// skip last item
				if (i != count - 1)
				{
					UI::Draw::Underline();
					Gui::Spacing();
				}
			}

			Gui::EndCombo();
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
    }

}
