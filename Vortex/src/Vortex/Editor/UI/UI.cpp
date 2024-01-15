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

	bool Property(const char* label, unsigned long long& value, float speed, int min, int max, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(std::format("##{}", label).c_str(), ImGuiDataType_U64, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool Property(const char* label, float& value, float speed, float min, float max, const char* format, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragFloat(std::format("##{}", label).c_str(), &value, speed, min, max, format))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool Property(const char* label, double& value, float speed, int min, int max, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(std::format("##{}", label).c_str(), ImGuiDataType_Double, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool Property(const char* label, Math::vec2& value, float speed, float min, float max, const char* format, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragFloat2(std::format("##{}", label).c_str(), Math::ValuePtr(value), speed, min, max, format))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool Property(const char* label, Math::vec3& value, float speed, float min, float max, const char* format, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragFloat3(std::format("##{}", label).c_str(), Math::ValuePtr(value), speed, min, max, format))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool Property(const char* label, Math::vec4& value, float speed, float min, float max, const char* format, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragFloat4(std::format("##{}", label).c_str(), Math::ValuePtr(value), speed, min, max, format))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool Property(const char* label, Math::vec3* value, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::ColorEdit3(std::format("##{}", label).c_str(), Math::ValuePtr(*value)))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool Property(const char* label, Math::vec4* value, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::ColorEdit4(std::format("##{}", label).c_str(), Math::ValuePtr(*value)))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool Property(const char* label, std::string& value, bool readOnly, bool multiline, bool enterReturnsTrue, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		char buffer[256];
		strcpy_s(buffer, sizeof(buffer), value.c_str());

		ImGuiInputTextFlags flags = 0;
		if (readOnly)
			flags |= ImGuiInputTextFlags_ReadOnly;
		if (enterReturnsTrue)
			flags |= ImGuiInputTextFlags_EnterReturnsTrue;

		if (Gui::InputText(std::format("##{}", label).c_str(), buffer, 256, flags))
		{
			value = buffer;
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool PropertyWithButton(const char* label, const char* buttonLabel, std::string& value, const std::function<void()>& onClickedFn, bool readOnly, bool multiline, bool enterReturnsTrue, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		char buffer[256];
		strcpy_s(buffer, sizeof(buffer), value.c_str());

		ImGuiInputTextFlags flags = 0;
		if (readOnly)
			flags |= ImGuiInputTextFlags_ReadOnly;
		if (enterReturnsTrue)
			flags |= ImGuiInputTextFlags_EnterReturnsTrue;

		if (Gui::Button(buttonLabel))
		{
			if (onClickedFn != nullptr)
			{
				std::invoke(onClickedFn);
			}
		}

		Gui::SameLine();

		if (Gui::InputText(std::format("##{}", label).c_str(), buffer, 256, flags))
		{
			value = buffer;
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool PropertySlider(const char* label, float& value, float min, float max, const char* format, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::SliderFloat(std::format("##{}", label).c_str(), &value, min, max, format))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool MultilineTextBox(const char* label, std::string& value, bool readOnly, const char* desc)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		char buffer[2048];
		strcpy_s(buffer, sizeof(buffer), value.c_str());

		const ImGuiInputTextFlags inputTextFlags = readOnly ? ImGuiInputTextFlags_ReadOnly : 0;

		if (Gui::InputTextMultiline(std::format("##{}", label).c_str(), buffer, 2048, ImVec2{ 0, 0 }, inputTextFlags))
		{
			value = buffer;
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool ColoredButton(const char* label, const ImVec4& backgroundColor, const ImVec4& foregroundColor, ImVec2 buttonSize)
	{
		ScopedColour textColor(ImGuiCol_Text, foregroundColor);
		ScopedColour buttonColor(ImGuiCol_Button, backgroundColor);
		return ImGui::Button(label, buttonSize);
	}

	bool PropertyDropdownSearch(const char* label, const char** options, uint32_t count, std::string& selected, ImGuiTextFilter& textFilter, const std::function<void()>& clearCallback, const char* desc)
	{
		const char* current = selected.c_str();

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
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
				if (UI::ImageButtonEx(EditorResources::ClearIcon, { 22, 22 }, { 0, 0, 0, 0 }, { 1, 1, 1, 1 }))
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

				if (Gui::Selectable(options[i], isSelected) || (Gui::IsItemFocused() && Gui::IsKeyPressed(ImGuiKey_Enter)))
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

	bool PropertyDropdownSearchWithImages(const char* label, const char** options, uint32_t count, const std::vector<UIImage>& images, std::string& selected, const UIImage& selectedImage, ImGuiTextFilter& textFilter, const std::function<void()>& clearCallback, const char* desc)
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

	void ImageEx(uint32_t rendererID, const ImVec2& size, const ImVec4& tintColor, const ImVec4& borderColor)
	{
		Gui::Image((ImTextureID)rendererID, size, { 0, 1 }, { 1, 0 }, tintColor, borderColor);
	}

	void ImageEx(const SharedReference<Texture2D>& texture, const ImVec2& size, const ImVec4& bgColor, const ImVec4& tintColor)
	{
		Gui::Image((ImTextureID)texture->GetRendererID(), size, { 0, 1 }, { 1, 0 }, bgColor, tintColor);
	}

	bool ImageButton(const char* label, const SharedReference<Texture2D>& texture, const ImVec2& size, const ImVec4& bgColor, const ImVec4& tintColor)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::ImageButton((ImTextureID)texture->GetRendererID(), size, { 0, 1 }, { 1, 0 }, -1, bgColor, tintColor))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	bool ImageButtonEx(const SharedReference<Texture2D>& texture, const ImVec2& size, const ImVec4& bgColor, const ImVec4& tintColor)
	{
		bool modified = false;

		if (Gui::ImageButton((ImTextureID)texture->GetRendererID(), size, { 0, 1 }, { 1, 0 }, -1, bgColor, tintColor))
		{
			modified = true;
		}

		return modified;
	}

	void DrawButtonImage(const SharedReference<Texture2D>& imageNormal, const SharedReference<Texture2D>& imageHovered, const SharedReference<Texture2D>& imagePressed, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed, ImVec2 rectMin, ImVec2 rectMax)
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		if (ImGui::IsItemActive())
			drawList->AddImage((ImTextureID)imagePressed->GetRendererID(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintPressed);
		else if (ImGui::IsItemHovered())
			drawList->AddImage((ImTextureID)imageHovered->GetRendererID(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintHovered);
		else
			drawList->AddImage((ImTextureID)imageNormal->GetRendererID(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintNormal);
	}

	void DrawButtonImage(const SharedReference<Texture2D>& imageNormal, const SharedReference<Texture2D>& imageHovered, const SharedReference<Texture2D>& imagePressed, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed, ImRect rectangle)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	}

	void DrawButtonImage(const SharedReference<Texture2D>& image, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed, ImVec2 rectMin, ImVec2 rectMax)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax);
	}

	void DrawButtonImage(const SharedReference<Texture2D>& image, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed, ImRect rectangle)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	}

	void DrawButtonImage(const SharedReference<Texture2D>& imageNormal, const SharedReference<Texture2D>& imageHovered, const SharedReference<Texture2D>& imagePressed, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	void DrawButtonImage(const SharedReference<Texture2D>& image, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	bool FontSelector(const char* label, const char** options, uint32_t count, ImFont* selected)
	{
		const char* current = Gui::GetFont()->GetDebugName();

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		bool modified = false;

		const std::string id = "##" + std::string(label);
		if (Gui::BeginCombo(id.c_str(), current))
		{
			ImGuiIO& io = Gui::GetIO();
			for (uint32_t i = 0; i < count; i++)
			{
				const bool isSelected = current == selected->GetDebugName();
				if (Gui::Selectable(options[i], isSelected))
				{
					current = options[i];
					selected = io.Fonts->Fonts[i];
					io.FontDefault = selected;
					modified = true;

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

		return modified;;
	}

	bool TreeNode(const char* label, bool defaultOpen)
	{
		bool opened = false;
		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (defaultOpen)
			treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

		if (Gui::TreeNodeEx(label, treeNodeFlags))
		{
			opened = true;
		}

		return opened;
	}

	bool ShowMessageBox(const char* title, bool* open, const ImVec2& size)
	{
		bool opened = false;

		if (Gui::IsPopupOpen(title))
		{
			Gui::SetNextWindowSize(size, ImGuiCond_Always);
			ImVec2 center = Gui::GetMainViewport()->GetCenter();
			Gui::SetNextWindowPos({ center.x - (size.x * 0.5f), center.y - (size.y * 0.5f) }, ImGuiCond_Appearing);
		}

		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;

		if (Gui::BeginPopupModal(title, open, flags))
		{
			opened = true;
		}

		return opened;
	}

	bool TableRowClickable(const char* id, float rowHeight)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		window->DC.CurrLineSize.y = rowHeight;

		ImGui::TableNextRow(0, rowHeight);
		ImGui::TableNextColumn();

		window->DC.CurrLineTextBaseOffset = 3.0f;
		ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
		ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x, rowAreaMin.y + rowHeight };

		ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);

		bool isRowHovered, held;
		bool isRowClicked = ImGui::ButtonBehavior(ImRect(rowAreaMin, rowAreaMax), ImGui::GetID(id), &isRowHovered, &held, ImGuiButtonFlags_AllowItemOverlap);

		ImGui::SetItemAllowOverlap();
		ImGui::PopClipRect();

		return isRowClicked;
	}

}
