#pragma once

#include "Vortex/Core/Math.h"

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
		Gui::BeginTooltip();
		Gui::Text(message);
		Gui::EndTooltip();
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
	
	inline static bool Property(const char* label, std::string& value, bool readOnly = false)
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		char buffer[256];
		strcpy_s(buffer, sizeof(buffer), value.c_str());

		if (Gui::InputText(fmt::format("##{}", label).c_str(), buffer, 256, readOnly ? ImGuiInputTextFlags_ReadOnly : 0))
		{
			value = buffer;
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();

		return modified;
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
