#pragma once

#include "Vortex/Core/Math/Math.h"
#include "Vortex/Gui/Colors.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Editor/FontAwesome.h"
#include "Vortex/Editor/EditorResources.h"

#include <imgui_internal.h>

namespace Vortex {

	namespace Gui = ImGui;

}

namespace Vortex::UI {

	static int s_UIContextID = 0;
	static uint32_t s_Counter = 0;
	static uint32_t s_CheckboxCount = 0;
	static char s_IDBuffer[16] = "##";
	static char s_LabelIDBuffer[1024];

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

	inline static const char* GenerateID()
	{
		_itoa_s(s_Counter++, s_IDBuffer + 2, sizeof(s_IDBuffer) - 2, 16);
		return s_IDBuffer;
	}

	class ScopedColour
	{
	public:
		ScopedColour(const ScopedColour&) = delete;
		ScopedColour& operator=(const ScopedColour&) = delete;
		template<typename T>
		ScopedColour(ImGuiCol colourId, T colour) { ImGui::PushStyleColor(colourId, ImColor(colour).Value); }
		~ScopedColour() { ImGui::PopStyleColor(); }
	};

	class ScopedFont
	{
	public:
		ScopedFont(const ScopedFont&) = delete;
		ScopedFont& operator=(const ScopedFont&) = delete;
		ScopedFont(ImFont* font) { ImGui::PushFont(font); }
		~ScopedFont() { ImGui::PopFont(); }
	};

	class ScopedID
	{
	public:
		ScopedID(const ScopedID&) = delete;
		ScopedID& operator=(const ScopedID&) = delete;
		template<typename T>
		ScopedID(T id) { ImGui::PushID(id); }
		~ScopedID() { ImGui::PopID(); }
	};

	class ScopedColourStack
	{
	public:
		ScopedColourStack(const ScopedColourStack&) = delete;
		ScopedColourStack& operator=(const ScopedColourStack&) = delete;

		template <typename ColourType, typename... OtherColours>
		ScopedColourStack(ImGuiCol firstColourID, ColourType firstColour, OtherColours&& ... otherColourPairs)
			: m_Count((sizeof... (otherColourPairs) / 2) + 1)
		{
			static_assert ((sizeof... (otherColourPairs) & 1u) == 0,
				"ScopedColourStack constructor expects a list of pairs of colour IDs and colours as its arguments");

			PushColour(firstColourID, firstColour, std::forward<OtherColours>(otherColourPairs)...);
		}

		~ScopedColourStack() { ImGui::PopStyleColor(m_Count); }

	private:
		int m_Count;

		template <typename ColourType, typename... OtherColours>
		void PushColour(ImGuiCol colourID, ColourType colour, OtherColours&& ... otherColourPairs)
		{
			if constexpr (sizeof... (otherColourPairs) == 0)
			{
				ImGui::PushStyleColor(colourID, ImColor(colour).Value);
			}
			else
			{
				ImGui::PushStyleColor(colourID, ImColor(colour).Value);
				PushColour(std::forward<OtherColours>(otherColourPairs)...);
			}
		}
	};

	class ScopedStyleStack
	{
	public:
		ScopedStyleStack(const ScopedStyleStack&) = delete;
		ScopedStyleStack& operator=(const ScopedStyleStack&) = delete;

		template <typename ValueType, typename... OtherStylePairs>
		ScopedStyleStack(ImGuiStyleVar firstStyleVar, ValueType firstValue, OtherStylePairs&& ... otherStylePairs)
			: m_Count((sizeof... (otherStylePairs) / 2) + 1)
		{
			static_assert ((sizeof... (otherStylePairs) & 1u) == 0,
				"ScopedStyleStack constructor expects a list of pairs of colour IDs and colours as its arguments");

			PushStyle(firstStyleVar, firstValue, std::forward<OtherStylePairs>(otherStylePairs)...);
		}

		~ScopedStyleStack() { ImGui::PopStyleVar(m_Count); }

	private:
		int m_Count;

		template <typename ValueType, typename... OtherStylePairs>
		void PushStyle(ImGuiStyleVar styleVar, ValueType value, OtherStylePairs&& ... otherStylePairs)
		{
			if constexpr (sizeof... (otherStylePairs) == 0)
			{
				ImGui::PushStyleVar(styleVar, value);
			}
			else
			{
				ImGui::PushStyleVar(styleVar, value);
				PushStyle(std::forward<OtherStylePairs>(otherStylePairs)...);
			}
		}
	};

	inline static ImColor ColorWithMultipliedValue(const ImColor& color, float multiplier)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(hue, sat, std::min(val * multiplier, 1.0f));
	}

	inline static const char* GenerateLabelID(std::string_view label)
	{
		*fmt::format_to_n(s_LabelIDBuffer, std::size(s_LabelIDBuffer), "{}##{}", label, s_Counter++).out = 0;
		return s_LabelIDBuffer;
	}

	static inline ImRect GetItemRect()
	{
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	static inline ImRect RectExpanded(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	static inline ImRect RectOffset(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x += x;
		result.Min.y += y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	static inline ImRect RectOffset(const ImRect& rect, ImVec2 xy)
	{
		return RectOffset(rect, xy.x, xy.y);
	}

	namespace Draw {

		static void Underline(bool fullWidth = false, float offsetX = 0.0f, float offsetY = -1.0f)
		{
			if (fullWidth)
			{
				if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr)
					ImGui::PushColumnsBackground();
				else if (ImGui::GetCurrentTable() != nullptr)
					ImGui::TablePushBackgroundChannel();
			}

			const float width = fullWidth ? ImGui::GetWindowWidth() : ImGui::GetContentRegionAvail().x;
			const ImVec2 cursor = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddLine(ImVec2(cursor.x + offsetX, cursor.y + offsetY),
				ImVec2(cursor.x + width, cursor.y + offsetY),
				Colors::Theme::backgroundDark, 1.0f);

			if (fullWidth)
			{
				if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr)
					ImGui::PopColumnsBackground();
				else if (ImGui::GetCurrentTable() != nullptr)
					ImGui::TablePopBackgroundChannel();
			}
		}
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

	inline void PushFont(const char* fontName)
	{
		if (fontName == "Bold")
			Gui::PushFont(Gui::GetIO().Fonts->Fonts[0]);
		else if (fontName == "Large")
			Gui::PushFont(Gui::GetIO().Fonts->Fonts[1]);
	}

	inline void PopFont()
	{
		Gui::PopFont();
	}

	inline bool IsInputEnabled()
	{
		const auto& io = ImGui::GetIO();
		return (io.ConfigFlags & ImGuiConfigFlags_NoMouse) == 0 && (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard) == 0;
	}

	inline void SetInputEnabled(bool enabled)
	{
		auto& io = ImGui::GetIO();

		if (enabled)
		{
			io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			io.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;
		}
		else
		{
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
		}
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

	inline static bool BeginPopup(const char* str_id, ImGuiWindowFlags flags)
	{
		bool opened = false;

		if (ImGui::BeginPopup(str_id, flags))
		{
			opened = true;
			// Fill background wiht nice gradient
			const float padding = ImGui::GetStyle().WindowBorderSize;
			const ImRect windowRect = UI::RectExpanded(ImGui::GetCurrentWindow()->Rect(), -padding, -padding);
			ImGui::PushClipRect(windowRect.Min, windowRect.Max, false);
			const ImColor col1 = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
			const ImColor col2 = UI::ColorWithMultipliedValue(col1, 0.8f);
			ImGui::GetWindowDrawList()->AddRectFilledMultiColor(windowRect.Min, windowRect.Max, col1, col1, col2, col2);
			ImGui::GetWindowDrawList()->AddRect(windowRect.Min, windowRect.Max, UI::ColorWithMultipliedValue(col1, 1.1f));
			ImGui::PopClipRect();

			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 80));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));
		}

		return opened;
	}

	inline static void EndPopup()
	{
		ImGui::PopStyleVar(); // WindowPadding;
		ImGui::PopStyleColor(); // HeaderHovered;
		ImGui::EndPopup();
	}

	inline static bool PropertyGridHeader(const char* label, bool defaultOpen = true)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_FramePadding;

		if (defaultOpen)
			flags |= ImGuiTreeNodeFlags_DefaultOpen;

		bool open = false;
		const float framePaddingX = 6.0f;
		const float framePaddingY = 6.0f;

		UI::ScopedStyle headerRounding(ImGuiStyleVar_FrameRounding, 0.0f);
		UI::ScopedStyle headerPaddingAndHeight(ImGuiStyleVar_FramePadding, { framePaddingX, framePaddingY });

		Gui::PushID(label);
		open = Gui::TreeNodeEx("##dummy_id", flags, label);
		Gui::PopID();

		return open;
	}

	inline static void EndTreeNode()
	{
		Gui::TreePop();
	}

	inline static void BeginCheckboxGroup(const char* label)
	{
		Gui::Text(label);
		Gui::NextColumn();
		Gui::PushItemWidth(-1);
	}

	inline static void EndCheckboxGroup()
	{
		Gui::PopItemWidth();
		Gui::NextColumn();
		s_CheckboxCount = 0;
	}

	inline static bool PropertyCheckboxGroup(const char* label, bool& value)
	{
		bool modified = false;

		if (++s_CheckboxCount > 1)
			Gui::SameLine();

		Gui::Text(label);
		Gui::SameLine();

		if (Gui::Checkbox(GenerateID(), &value))
			modified = true;

		return modified;
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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

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
		Draw::Underline();

		return modified;
	}

	inline static bool PropertySlider(const char* label, float& value, float min = 0.0f, float max = 0.0f, const char* format = "%.2f")
	{
		bool modified = false;

		ShiftCursor(10.0f, 9.0f);
		Gui::Text(label);
		Gui::NextColumn();
		ShiftCursorY(4.0f);
		Gui::PushItemWidth(-1);

		if (Gui::SliderFloat(fmt::format("##{}", label).c_str(), &value, min, max, format))
		{
			modified = true;
		}

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

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
		Draw::Underline();

		return modified;
	}

	inline bool ColoredButton(const char* label, const ImVec4& backgroundColor, const ImVec4& foregroundColor, ImVec2 buttonSize)
	{
		ScopedColour textColor(ImGuiCol_Text, foregroundColor);
		ScopedColour buttonColor(ImGuiCol_Button, backgroundColor);
		return ImGui::Button(label, buttonSize);
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

				// skip last item
				if (i != count - 1)
				{
					UI::Draw::Underline();
					Gui::Spacing();
				}
			}

			Gui::EndCombo();
		}

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

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
			const bool isSearching = Gui::InputTextWithHint(id.c_str(), "Search", textFilter.InputBuf, IM_ARRAYSIZE(textFilter.InputBuf));

			if (isSearching)
				textFilter.Build();

			UI::Draw::Underline();

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

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	inline static void ImageEx(uint32_t rendererID, const ImVec2& size = ImVec2(64, 64), const ImVec4& tintColor = ImVec4(1, 1, 1, 1), const ImVec4& borderColor = ImVec4(0, 0, 0, 0))
	{
		Gui::Image((ImTextureID)rendererID, size, { 0, 1 }, { 1, 0 }, tintColor, borderColor);
	}

	inline static void ImageEx(const SharedReference<Texture2D>& texture, const ImVec2& size = ImVec2(64, 64), const ImVec4& bgColor = ImVec4(0, 0, 0, 0), const ImVec4& tintColor = ImVec4(0, 0, 0, 0))
	{
		Gui::Image((ImTextureID)texture->GetRendererID(), size, { 0, 1 }, { 1, 0 }, bgColor, tintColor);
	}

	inline static bool ImageButton(const char* label, const SharedReference<Texture2D>& texture, const ImVec2& size = ImVec2(64, 64), const ImVec4& bgColor = ImVec4(0, 0, 0, 0), const ImVec4& tintColor = ImVec4(0, 0, 0, 0))
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

	inline static bool ImageButtonEx(const SharedReference<Texture2D>& texture, const ImVec2& size = ImVec2(64, 64), const ImVec4& bgColor = ImVec4(0, 0, 0, 0), const ImVec4& tintColor = ImVec4(0, 0, 0, 0))
	{
		bool modified = false;

		if (Gui::ImageButton((ImTextureID)texture->GetRendererID(), size, { 0, 1 }, { 1, 0 }, -1, bgColor, tintColor))
		{
			modified = true;
		}

		return modified;
	}

	static void DrawButtonImage(const SharedReference<Texture2D>& imageNormal, const SharedReference<Texture2D>& imageHovered, const SharedReference<Texture2D>& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax)
	{
		auto* drawList = ImGui::GetWindowDrawList();
		if (ImGui::IsItemActive())
			drawList->AddImage((ImTextureID)imagePressed->GetRendererID(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintPressed);
		else if (ImGui::IsItemHovered())
			drawList->AddImage((ImTextureID)imageHovered->GetRendererID(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintHovered);
		else
			drawList->AddImage((ImTextureID)imageNormal->GetRendererID(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintNormal);
	};

	static void DrawButtonImage(const SharedReference<Texture2D>& imageNormal, const SharedReference<Texture2D>& imageHovered, const SharedReference<Texture2D>& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	};

	static void DrawButtonImage(const SharedReference<Texture2D>& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax);
	};

	static void DrawButtonImage(const SharedReference<Texture2D>& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	};


	static void DrawButtonImage(const SharedReference<Texture2D>& imageNormal, const SharedReference<Texture2D>& imageHovered, const SharedReference<Texture2D>& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	};

	static void DrawButtonImage(const SharedReference<Texture2D>& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	};

	inline static void DrawItemActivityOutline(float rounding = 0.0f, bool drawWhenInactive = false, ImColor colourWhenActive = ImColor(80, 80, 80))
	{
		auto* drawList = ImGui::GetWindowDrawList();
		const ImRect rect = RectExpanded(GetItemRect(), 1.0f, 1.0f);
		if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
		{
			drawList->AddRect(rect.Min, rect.Max,
				ImColor(60, 60, 60), rounding, 0, 1.5f);
		}
		if (ImGui::IsItemActive())
		{
			drawList->AddRect(rect.Min, rect.Max,
				colourWhenActive, rounding, 0, 1.0f);
		}
		else if (!ImGui::IsItemHovered() && drawWhenInactive)
		{
			drawList->AddRect(rect.Min, rect.Max,
				ImColor(50, 50, 50), rounding, 0, 1.0f);
		}
	};

	template <typename StringType, typename... OtherReplacements>
	inline static std::string Replace(StringType textToSearch, std::string_view firstToReplace, std::string_view firstReplacement,
		OtherReplacements&&... otherPairsOfStringsToReplace)
	{
		static_assert ((sizeof... (otherPairsOfStringsToReplace) & 1u) == 0,
			"This function expects a list of pairs of strings as its arguments");

		if constexpr (std::is_same<const StringType, const std::string_view>::value || std::is_same<const StringType, const char* const>::value)
		{
			return Replace(std::string(textToSearch), firstToReplace, firstReplacement,
				std::forward<OtherReplacements>(otherPairsOfStringsToReplace)...);
		}
		else if constexpr (sizeof... (otherPairsOfStringsToReplace) == 0)
		{
			size_t pos = 0;

			for (;;)
			{
				pos = textToSearch.find(firstToReplace, pos);

				if (pos == std::string::npos)
					return textToSearch;

				textToSearch.replace(pos, firstToReplace.length(), firstReplacement);
				pos += firstReplacement.length();
			}
		}
		else
		{
			return Replace(Replace(std::move(textToSearch), firstToReplace, firstReplacement),
				std::forward<OtherReplacements>(otherPairsOfStringsToReplace)...);
		}
	}

	template <typename CharStartsDelimiter, typename CharIsInDelimiterBody>
	inline static std::vector<std::string> SplitString(std::string_view source,
		CharStartsDelimiter&& isDelimiterStart,
		CharIsInDelimiterBody&& isDelimiterBody,
		bool keepDelimiters)
	{
		std::vector<std::string> tokens;
		auto tokenStart = source.begin();
		auto pos = tokenStart;

		while (pos != source.end())
		{
			if (isDelimiterStart(*pos))
			{
				auto delimiterStart = pos++;

				while (pos != source.end() && isDelimiterBody(*pos))
					++pos;

				if (pos != source.begin())
					tokens.push_back({ tokenStart, keepDelimiters ? pos : delimiterStart });

				tokenStart = pos;
			}
			else
			{
				++pos;
			}
		}

		if (pos != source.begin())
			tokens.push_back({ tokenStart, pos });

		return tokens;
	}

	inline static bool IsWhitespace(char c) { return c == ' ' || (c <= 13 && c >= 9); }

	inline static std::vector<std::string> SplitAtWhitespace(std::string_view text, bool keepDelimiters = false)
	{
		return SplitString(text,
			[](char c) { return IsWhitespace(c); },
			[](char c) { return IsWhitespace(c); },
			keepDelimiters);
	}

	inline static std::string& ToLower(std::string& string)
	{
		std::transform(string.begin(), string.end(), string.begin(),
			[](const unsigned char c) { return std::tolower(c); });

		return string;
	}

	inline static bool IsMatchingSearch(const std::string& item, std::string_view searchQuery, bool caseSensitive = false, bool stripWhiteSpaces = false, bool stripUnderscores = false)
	{
		if (searchQuery.empty())
			return true;

		if (item.empty())
			return false;

		std::string itemSanitized = stripUnderscores ? Replace(item, "_", " ") : item;

		if (stripWhiteSpaces)
			itemSanitized = Replace(itemSanitized, " ", "");

		std::string searchString = stripWhiteSpaces ? Replace(searchQuery, " ", "") : std::string(searchQuery);

		if (!caseSensitive)
		{
			itemSanitized = ToLower(itemSanitized);
			searchString = ToLower(searchString);
		}

		bool result = false;
		if (searchString.find(" ") != std::string::npos)
		{
			std::vector<std::string> searchTerms = SplitAtWhitespace(searchString);
			for (const auto& searchTerm : searchTerms)
			{
				if (!searchTerm.empty() && itemSanitized.find(searchTerm) != std::string::npos)
					result = true;
				else
				{
					result = false;
					break;
				}
			}
		}
		else
		{
			result = itemSanitized.find(searchString) != std::string::npos;
		}

		return result;
	}

	template<uint32_t BuffSize = 256, typename StringType>
	inline static bool SearchWidget(StringType& searchString, const char* hint = "Search...", bool* grabFocus = nullptr)
	{
		PushID();

		ShiftCursorY(1.0f);

		const bool layoutSuspended = []
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (window->DC.CurrentLayout)
			{
				ImGui::SuspendLayout();
				return true;
			}
			return false;
		}();

		bool modified = false;
		bool searching = false;

		const float areaPosX = ImGui::GetCursorPosX();
		const float framePaddingY = ImGui::GetStyle().FramePadding.y;

		UI::ScopedStyle rounding(ImGuiStyleVar_FrameRounding, 3.0f);
		UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(28.0f, framePaddingY));

		if constexpr (std::is_same<StringType, std::string>::value)
		{
			char searchBuffer[BuffSize]{};
			strcpy_s<BuffSize>(searchBuffer, searchString.c_str());
			if (ImGui::InputText(GenerateID(), searchBuffer, BuffSize))
			{
				searchString = searchBuffer;
				modified = true;
			}
			else if (ImGui::IsItemDeactivatedAfterEdit())
			{
				searchString = searchBuffer;
				modified = true;
			}

			searching = searchBuffer[0] != 0;
		}
		else
		{
			static_assert(std::is_same<decltype(&searchString[0]), char*>::value,
				"searchString paramenter must be std::string& or char*");

			if (ImGui::InputText(GenerateID(), searchString, BuffSize))
			{
				modified = true;
			}
			else if (ImGui::IsItemDeactivatedAfterEdit())
			{
				modified = true;
			}

			searching = searchString[0] != 0;
		}

		if (grabFocus && *grabFocus)
		{
			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
				&& !ImGui::IsAnyItemActive()
				&& !ImGui::IsMouseClicked(0))
			{
				ImGui::SetKeyboardFocusHere(-1);
			}

			if (ImGui::IsItemFocused())
				*grabFocus = false;
		}

		UI::DrawItemActivityOutline(3.0f, true, Colors::Theme::accent);
		ImGui::SetItemAllowOverlap();

		ImGui::SameLine(areaPosX + 5.0f);

		if (layoutSuspended)
			ImGui::ResumeLayout();

		ImGui::BeginHorizontal(GenerateID(), ImGui::GetItemRectSize());
		const ImVec2 iconSize(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight());

		// Search icon
		{
			const float iconYOffset = framePaddingY - 3.0f;
			UI::ShiftCursorY(iconYOffset);
			UI::ImageEx(EditorResources::SearchIcon, iconSize, ImVec4(1.0f, 1.0f, 1.0f, 0.2f), ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
			UI::ShiftCursorY(-iconYOffset);

			// Hint
			if (!searching)
			{
				UI::ShiftCursorY(-framePaddingY + 1.0f);
				UI::ScopedColour text(ImGuiCol_Text, Colors::Theme::textDarker);
				UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, framePaddingY));
				ImGui::TextUnformatted(hint);
				UI::ShiftCursorY(-1.0f);
			}
		}

		ImGui::Spring();

		// Clear icon
		if (searching)
		{
			const float spacingX = 4.0f;
			const float lineHeight = ImGui::GetItemRectSize().y - framePaddingY / 2.0f;

			if (ImGui::InvisibleButton(GenerateID(), ImVec2{ lineHeight, lineHeight }))
			{
				if constexpr (std::is_same<StringType, std::string>::value)
					searchString.clear();
				else
					memset(searchString, 0, BuffSize);

				modified = true;
			}

			if (ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
				ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

			UI::DrawButtonImage(EditorResources::ClearIcon, IM_COL32(160, 160, 160, 200),
				IM_COL32(170, 170, 170, 255),
				IM_COL32(160, 160, 160, 150),
				UI::RectExpanded(UI::GetItemRect(), -2.0f, -2.0f));

			ImGui::Spring(-1.0f, spacingX * 2.0f);
		}

		ImGui::EndHorizontal();
		UI::ShiftCursorY(-1.0f);
		UI::PopID();
		return modified;
	}

	inline static bool EntitySearchPopup(const char* ID, SharedReference<Scene>& scene, UUID& selected, bool* cleared = nullptr, const char* hint = "Search Entities", ImVec2 size = { 250.0f, 350.0f })
	{
		UI::ScopedColour popupBG(ImGuiCol_PopupBg, UI::ColorWithMultipliedValue(Colors::Theme::background, 1.6f));

		bool modified = false;

		auto entities = scene->GetAllEntitiesWith<IDComponent, TagComponent>();
		UUID current = selected;

		ImGui::SetNextWindowSize({ size.x, 0.0f });

		static bool s_GrabFocus = true;

		if (UI::BeginPopup(ID, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
		{
			static std::string searchString;

			if (ImGui::GetCurrentWindow()->Appearing)
			{
				s_GrabFocus = true;
				searchString.clear();
			}

			// Search widget
			UI::ShiftCursor(3.0f, 2.0f);
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() * 2.0f);
			SearchWidget(searchString, hint, &s_GrabFocus);

			const bool searching = !searchString.empty();

			// Clear property button
			if (cleared != nullptr)
			{
				UI::ScopedColourStack buttonColours(
					ImGuiCol_Button, UI::ColorWithMultipliedValue(Colors::Theme::background, 1.0f),
					ImGuiCol_ButtonHovered, UI::ColorWithMultipliedValue(Colors::Theme::background, 1.2f),
					ImGuiCol_ButtonActive, UI::ColorWithMultipliedValue(Colors::Theme::background, 0.9f));

				UI::ScopedStyle border(ImGuiStyleVar_FrameBorderSize, 0.0f);

				ImGui::SetCursorPosX(0);

				ImGui::PushItemFlag(ImGuiItemFlags_NoNav, searching);

				if (ImGui::Button("CLEAR", { ImGui::GetWindowWidth(), 0.0f }))
				{
					*cleared = true;
					modified = true;
				}

				ImGui::PopItemFlag();
			}

			// List of entities
			{
				UI::ScopedColour listBoxBg(ImGuiCol_FrameBg, IM_COL32_DISABLE);
				UI::ScopedColour listBoxBorder(ImGuiCol_Border, IM_COL32_DISABLE);

				ImGuiID listID = ImGui::GetID("##SearchListBox");
				if (ImGui::BeginListBox("##SearchListBox", ImVec2(-FLT_MIN, 0.0f)))
				{
					bool forwardFocus = false;

					ImGuiContext& g = *GImGui;
					if (g.NavJustMovedToId != 0)
					{
						if (g.NavJustMovedToId == listID)
						{
							forwardFocus = true;
							// ActivateItem moves keyboard navigation focuse inside of the window
							ImGui::ActivateItem(listID);
							ImGui::SetKeyboardFocusHere(1);
						}
					}

					for (auto enttID : entities)
					{
						const auto& idComponent = entities.get<IDComponent>(enttID);
						const auto& tagComponent = entities.get<TagComponent>(enttID);

						if (!searchString.empty() && !UI::IsMatchingSearch(tagComponent.Tag, searchString))
							continue;

						bool is_selected = current == idComponent.ID;
						if (ImGui::Selectable(tagComponent.Tag.c_str(), is_selected))
						{
							current = selected = idComponent.ID;
							modified = true;
						}

						if (forwardFocus)
							forwardFocus = false;
						else if (is_selected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndListBox();
				}
			}
			if (modified)
				ImGui::CloseCurrentPopup();

			UI::EndPopup();
		}

		return modified;
	}

	inline static bool PropertyEntityReference(const char* label, UUID& entityID, SharedReference<Scene>& currentScene)
	{
		bool receivedValidEntity = false;

		ShiftCursor(10.0f, 9.0f);
		ImGui::Text(label);
		ImGui::NextColumn();
		ShiftCursorY(4.0f);
		ImGui::PushItemWidth(-1);

		ImVec2 originalButtonTextAlign = ImGui::GetStyle().ButtonTextAlign;
		{
			ImGui::GetStyle().ButtonTextAlign = { 0.0f, 0.5f };
			float width = ImGui::GetContentRegionAvail().x;
			float itemHeight = 28.0f;

			std::string buttonText = "Null";

			Entity entity = currentScene->TryGetEntityWithUUID(entityID);
			if (entity)
				buttonText = entity.GetComponent<TagComponent>().Tag;

			// PropertyEntityReference could be called multiple times in same "context"
			// and so we need a unique id for the asset search popup each time.
			// notes
			// - don't use GenerateID(), that's inviting id clashes, which would be super confusing.
			// - don't store return from GenerateLabelId in a const char* here. Because its pointing to an internal
			//   buffer which may get overwritten by the time you want to use it later on.
			std::string assetSearchPopupID = GenerateLabelID("ARSP");
			{
				UI::ScopedColour buttonLabelColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(Colors::Theme::text));
				ImGui::Button(GenerateLabelID(buttonText), { width, itemHeight });

				const bool isHovered = ImGui::IsItemHovered();
				if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					ImGui::OpenPopup(assetSearchPopupID.c_str());
			}

			ImGui::GetStyle().ButtonTextAlign = originalButtonTextAlign;

			bool clear = false;
			if (EntitySearchPopup(assetSearchPopupID.c_str(), currentScene, entityID, &clear))
			{
				if (clear)
					entityID = 0;
				receivedValidEntity = true;
			}
		}

		if (!(ImGui::GetItemFlags() & ImGuiItemFlags_Disabled))
		{
			if (ImGui::BeginDragDropTarget())
			{
				auto data = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM");

				if (data)
				{
					entityID = *(UUID*)data->Data;
					receivedValidEntity = true;
				}

				ImGui::EndDragDropTarget();
			}
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return receivedValidEntity;
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
		Draw::Underline();

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

	template<typename T>
	inline static void Table(const char* tableName, const char** columns, uint32_t columnCount, const ImVec2& size, T callback)
	{
		if (size.x <= 0.0f || size.y <= 0.0f)
			return;

		float edgeOffset = 4.0f;

		ScopedStyle cellPadding(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 0.0f));
		ImColor backgroundColor = ImColor(Colors::Theme::background);
		const ImColor colRowAlt = ColorWithMultipliedValue(backgroundColor, 1.2f);
		ScopedColour rowColor(ImGuiCol_TableRowBg, backgroundColor);
		ScopedColour rowAltColor(ImGuiCol_TableRowBgAlt, colRowAlt);
		ScopedColour tableColor(ImGuiCol_ChildBg, backgroundColor);

		ImGuiTableFlags flags = ImGuiTableFlags_NoPadInnerX
			| ImGuiTableFlags_Resizable
			| ImGuiTableFlags_Reorderable
			| ImGuiTableFlags_ScrollY
			| ImGuiTableFlags_RowBg;

		if (!ImGui::BeginTable(tableName, columnCount, flags, size))
			return;

		const float cursorX = ImGui::GetCursorScreenPos().x;

		for (uint32_t i = 0; i < columnCount; i++)
			ImGui::TableSetupColumn(columns[i]);

		// Headers
		{
			const ImColor activeColor = ColorWithMultipliedValue(backgroundColor, 1.3f);
			ScopedColourStack headerCol(ImGuiCol_HeaderHovered, activeColor, ImGuiCol_HeaderActive, activeColor);

			ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);
			ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f);

			for (uint32_t i = 0; i < columnCount; i++)
			{
				ImGui::TableSetColumnIndex(i);
				const char* columnName = ImGui::TableGetColumnName(i);
				ImGui::PushID(columnName);
				ShiftCursor(edgeOffset * 3.0f, edgeOffset * 2.0f);
				ImGui::TableHeader(columnName);
				ShiftCursor(-edgeOffset * 3.0f, -edgeOffset * 2.0f);
				ImGui::PopID();
			}
			ImGui::SetCursorScreenPos(ImVec2(cursorX, ImGui::GetCursorScreenPos().y));
			Draw::Underline(true, 0.0f, 5.0f);
		}

		callback();
		ImGui::EndTable();
	}

	inline bool TableRowClickable(const char* id, float rowHeight)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		window->DC.CurrLineSize.y = rowHeight;

		ImGui::TableNextRow(0, rowHeight);
		ImGui::TableNextColumn();

		window->DC.CurrLineTextBaseOffset = 3.0f;
		const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
		const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x, rowAreaMin.y + rowHeight };

		ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);

		bool isRowHovered, held;
		bool isRowClicked = ImGui::ButtonBehavior(ImRect(rowAreaMin, rowAreaMax), ImGui::GetID(id),
			&isRowHovered, &held, ImGuiButtonFlags_AllowItemOverlap);

		ImGui::SetItemAllowOverlap();
		ImGui::PopClipRect();

		return isRowClicked;
	}

	inline void Separator(ImVec2 size, ImVec4 color)
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg, color);
		ImGui::BeginChild("sep", size);
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}

}
