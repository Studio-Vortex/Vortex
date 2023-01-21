#pragma once

#include "Vortex/Core/Math.h"
#include "Vortex/Renderer/Texture.h"

#include <imgui_internal.h>

namespace Vortex::UI {

	namespace Gui = ImGui;

	static uint32_t s_UIContextID = 0;

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
		Gui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });
		Gui::BeginTooltip();
		Gui::Text(message);
		Gui::EndTooltip();
		Gui::PopStyleColor();
		Gui::PopStyleVar();
	}

	inline static void HelpMarker(const char* desc)
	{
		Gui::TextDisabled("(?)");

		if (Gui::IsItemHovered())
		{
			Gui::BeginTooltip();
			Gui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			Gui::TextUnformatted(desc);
			Gui::PopTextWrapPos();
			Gui::EndTooltip();
		}
	}

	inline static void DrawVec3Controls(const std::string& label, Math::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f, std::function<void()> uiCallback = nullptr)
	{
		ImGuiIO& io = Gui::GetIO();
		const auto& boldFont = io.Fonts->Fonts[0];

		Gui::PushID(label.c_str());

		Gui::Columns(2);
		Gui::SetColumnWidth(0, columnWidth);
		Gui::Text(label.c_str());
		Gui::NextColumn();

		Gui::PushMultiItemsWidths(3, Gui::CalcItemWidth());
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("X", buttonSize))
		{
			values.x = resetValue;

			if (uiCallback != nullptr)
				uiCallback();
		}
		Gui::PopFont();
		Gui::PopStyleColor(3);

		Gui::SameLine();
		if (Gui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f") && uiCallback != nullptr)
			uiCallback();
		Gui::PopItemWidth();
		Gui::SameLine();

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("Y", buttonSize))
		{
			values.y = resetValue;

			if (uiCallback != nullptr)
				uiCallback();
		}
		Gui::PopFont();
		Gui::PopStyleColor(3);

		Gui::SameLine();
		if (Gui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f") && uiCallback != nullptr)
			uiCallback();
		Gui::PopItemWidth();
		Gui::SameLine();

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushFont(boldFont);
		if (Gui::Button("Z", buttonSize))
		{
			values.z = resetValue;

			if (uiCallback != nullptr)
				uiCallback();
		}
		Gui::PopFont();
		Gui::PopStyleColor(3);

		Gui::SameLine();
		if (Gui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f") && uiCallback != nullptr)
			uiCallback();
		Gui::PopItemWidth();

		Gui::PopStyleVar();
		Gui::Columns(1);
		Gui::PopID();
	}

	inline void PushID()
	{
		Gui::PushID(s_UIContextID++);
	}

	inline void PopID()
	{
		Gui::PopID();
		s_UIContextID--;
	}

	inline void ShiftCursorX(float distance)
	{
		Gui::SetCursorPosX(Gui::GetCursorPosX() + distance);
	}

	inline void ShiftCursorY(float distance)
	{
		Gui::SetCursorPosY(Gui::GetCursorPosY() + distance);
	}

	inline void ShiftCursor(float x, float y)
	{
		ImVec2 cursorPos = Gui::GetCursorPos();
		Gui::SetCursorPos(ImVec2{ cursorPos.x + x, cursorPos.y + y });
	}

	inline static void BeginPropertyGrid(uint32_t columns = 2)
	{
		PushID();
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8.0f, 8.0f });
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });
		Gui::Columns(columns);
	}

	inline static void BeginPropertyGrid(float columnWidth)
	{
		PushID();
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8.0f, 8.0f });
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });
		Gui::Columns(2);
		Gui::SetColumnWidth(0, columnWidth);
	}

	inline static void EndPropertyGrid()
	{
		Gui::Columns(1);
		Gui::PopStyleVar(2);
		ShiftCursorY(18.0f);
		PopID();
	}

	inline static bool Property(const char* label, bool& value)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::Checkbox(fmt::format("##{}", label).c_str(), &value))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, char& value, float speed = 1.0f, int min = 0, int max = 0)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(fmt::format("##{}", label).c_str(), ImGuiDataType_S8, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, unsigned char& value, float speed = 1.0f, int min = 0, int max = 0)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(fmt::format("##{}", label).c_str(), ImGuiDataType_U8, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, short& value, float speed = 1.0f, int min = 0, int max = 0)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(fmt::format("##{}", label).c_str(), ImGuiDataType_S16, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, unsigned short& value, float speed = 1.0f, int min = 0, int max = 0)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(fmt::format("##{}", label).c_str(), ImGuiDataType_U16, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, int& value, float speed = 1.0f, int min = 0, int max = 0)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragInt(fmt::format("##{}", label).c_str(), &value, speed, min, max))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, uint32_t& value, float speed = 1.0f, int min = 0, int max = 0)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(fmt::format("##{}", label).c_str(), ImGuiDataType_U32, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, long long& value, float speed = 1.0f, int min = 0, int max = 0)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(fmt::format("##{}", label).c_str(), ImGuiDataType_S64, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, unsigned long long& value, float speed = 1.0f, int min = 0, int max = 0)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(fmt::format("##{}", label).c_str(), ImGuiDataType_U64, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, float& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.2f")
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragFloat(fmt::format("##{}", label).c_str(), &value, speed, min, max, format))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}


	inline static bool Property(const char* label, double& value, float speed = 1.0f, int min = 0, int max = 0)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragScalar(fmt::format("##{}", label).c_str(), ImGuiDataType_Double, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, Math::vec2& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.2f")
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragFloat2(fmt::format("##{}", label).c_str(), Math::ValuePtr(value), speed, min, max, format))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, Math::vec3& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.2f")
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragFloat3(fmt::format("##{}", label).c_str(), Math::ValuePtr(value), speed, min, max, format))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, Math::vec4& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.2f")
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::DragFloat4(fmt::format("##{}", label).c_str(), Math::ValuePtr(value), speed, min, max, format))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, Math::vec3* value)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::ColorEdit3(fmt::format("##{}", label).c_str(), Math::ValuePtr(*value)))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool Property(const char* label, Math::vec4* value)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::ColorEdit4(fmt::format("##{}", label).c_str(), Math::ValuePtr(*value)))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}
	
	inline static bool Property(const char* label, std::string& value, bool readOnly = false, bool multiline = false)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		char buffer[256];
		strcpy_s(buffer, sizeof(buffer), value.c_str());

		const ImGuiInputTextFlags inputTextFlags = readOnly ? ImGuiInputTextFlags_ReadOnly : 0;

		if (Gui::InputText(fmt::format("##{}", label).c_str(), buffer, 256, inputTextFlags))
		{
			value = buffer;
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool MultilineTextBox(const char* label, std::string& value, bool readOnly = false)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		char buffer[2048];
		strcpy_s(buffer, sizeof(buffer), value.c_str());

		const ImGuiInputTextFlags inputTextFlags = readOnly ? ImGuiInputTextFlags_ReadOnly : 0;

		if (Gui::InputTextMultiline(fmt::format("##{}", label).c_str(), buffer, 2048, ImVec2{ 0, 0 }, inputTextFlags))
		{
			value = buffer;
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	template <typename TEnum, typename TUnderlying = int32_t>
	inline static bool PropertyDropdown(const char* label, const char** options, uint32_t count, TEnum& selected)
	{
		TUnderlying selectedIndex = (TUnderlying)selected;
		const char* current = options[selectedIndex];

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		bool modified = false;

		const std::string id = "##" + std::string(label);
		if (Gui::BeginCombo(id.c_str(), current))
		{
			for (uint32_t i = 0; i < count; i++)
			{
				const bool isSelected = current == options[i];
				if (Gui::Selectable(options[i], isSelected))
				{
					current = options[i];
					selected = (TEnum)i;
					modified = true;
				}

				if (isSelected)
					Gui::SetItemDefaultFocus();
			}

			Gui::EndCombo();
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool PropertyDropdownSearch(const char* label, const char** options, uint32_t count, std::string& selected, ImGuiTextFilter& textFilter)
	{
		const char* current = selected.c_str();

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		bool modified = false;

		const std::string id = "##" + std::string(label);
		if (Gui::BeginCombo(id.c_str(), current))
		{
			bool isSearching = Gui::InputTextWithHint("##ClassNameSearch", "Search", textFilter.InputBuf, IM_ARRAYSIZE(textFilter.InputBuf));
			if (isSearching)
				textFilter.Build();

			Gui::Separator();

			for (uint32_t i = 0; i < count; i++)
			{
				const bool isSelected = current == options[i];

				if (!textFilter.PassFilter(options[i]))
					continue;

				if (Gui::Selectable(options[i], isSelected))
				{
					current = options[i];
					selected = options[i];
					modified = true;

					memset(textFilter.InputBuf, 0, IM_ARRAYSIZE(textFilter.InputBuf));
					textFilter.Build();
				}

				if (isSelected)
					Gui::SetItemDefaultFocus();
			}

			Gui::EndCombo();
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static bool FontSelector(const char* label, const char** options, uint32_t count, ImFont* selected)
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
				}

				if (isSelected)
					Gui::SetItemDefaultFocus();
			}

			Gui::EndCombo();
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
	}

	inline static void ImageEx(uint32_t rendererID, const ImVec2& size = ImVec2(64, 64), const ImVec4& tintColor = ImVec4(1, 1, 1, 1), const ImVec4& borderColor = ImVec4(0, 0, 0, 0))
	{
		Gui::Image((ImTextureID)rendererID, size, { 0, 1 }, { 1, 0 }, tintColor, borderColor);
	}

	inline static void ImageEx(const SharedRef<Texture2D>& texture, const ImVec2& size = ImVec2(64, 64), const ImVec4& bgColor = ImVec4(0, 0, 0, 0), const ImVec4& tintColor = ImVec4(0, 0, 0, 0))
	{
		Gui::Image((ImTextureID)texture->GetRendererID(), size, { 0, 1 }, { 1, 0 }, bgColor, tintColor);
	}

	inline static bool ImageButton(const char* label, const SharedRef<Texture2D>& texture, const ImVec2& size = ImVec2(64, 64), const ImVec4& bgColor = ImVec4(0, 0, 0, 0), const ImVec4& tintColor = ImVec4(0, 0, 0, 0))
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

		return modified;
	}

	inline static bool ImageButtonEx(const SharedRef<Texture2D>& texture, const ImVec2& size = ImVec2(64, 64), const ImVec4& bgColor = ImVec4(0, 0, 0, 0), const ImVec4& tintColor = ImVec4(0, 0, 0, 0))
	{
		bool modified = false;

		if (Gui::ImageButton((ImTextureID)texture->GetRendererID(), size, { 0, 1 }, { 1, 0 }, -1, bgColor, tintColor))
		{
			modified = true;
		}

		return modified;
	}

	inline static bool TreeNode(const char* label, bool defaultOpen = true)
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

	inline static bool ShowMessageBox(const char* title, const ImVec2& size)
	{
		bool opened = false;

		if (Gui::IsPopupOpen(title))
		{
			Gui::SetNextWindowSize(size, ImGuiCond_Always);
			ImVec2 center = Gui::GetMainViewport()->GetCenter();
			Gui::SetNextWindowPos({ center.x - (size.x * 0.5f), center.y - (size.y * 0.5f) }, ImGuiCond_Appearing);
		}

		if (Gui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_NoResize))
		{
			opened = true;
		}

		return opened;
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
