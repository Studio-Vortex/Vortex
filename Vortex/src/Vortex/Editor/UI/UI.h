#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/Log.h"
#include "Vortex/Core/String.h"

#include "Vortex/Math/Math.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

#include "Vortex/Gui/Colors.h"

#include "Vortex/Renderer/Texture.h"

#include "Vortex/Asset/AssetTypes.h"
#include "Vortex/Asset/AssetRegistry.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"

#include "Vortex/Editor/FontAwesome.h"
#include "Vortex/Editor/EditorResources.h"

#include <string>

#include <imgui_internal.h>

namespace Vortex {

	namespace Gui = ImGui;

}

namespace Vortex::UI {

#define UI_MAX_TEXT_FILTERS 64

	static int s_UIContextID = 0;
	static uint32_t s_Counter = 0;
	static uint32_t s_CheckboxCount = 0;
	static char s_IDBuffer[16] = "##";
	static char s_LabelIDBuffer[1024];
	static ImGuiTextFilter s_TextFilters[UI_MAX_TEXT_FILTERS];

	namespace Internal {

		inline static void Init()
		{
			for (size_t i = 0; i < UI_MAX_TEXT_FILTERS; i++)
			{
				ImGuiTextFilter& filter = s_TextFilters[i];
				memset(filter.InputBuf, 0, IM_ARRAYSIZE(filter.InputBuf));
				filter.Build();
			}
		}

		inline static void Shutdown()
		{

		}

	}

	class VORTEX_API ScopedStyle
	{
	public:
		ScopedStyle(ImGuiStyleVar var, ImVec2 value);
		ScopedStyle(ImGuiStyleVar var, float value);
		~ScopedStyle();
	};

	class VORTEX_API ScopedColor
	{
	public:
		ScopedColor(ImGuiCol col, ImVec4 color);
		~ScopedColor();
	};

	VORTEX_API inline static const char* GenerateID()
	{
		_itoa_s(s_Counter++, s_IDBuffer + 2, sizeof(s_IDBuffer) - 2, 16);
		return s_IDBuffer;
	}

	class VORTEX_API ScopedColour
	{
	public:
		ScopedColour(const ScopedColour&) = delete;
		ScopedColour& operator=(const ScopedColour&) = delete;
		template<typename T>
		ScopedColour(ImGuiCol colourId, T colour) { ImGui::PushStyleColor(colourId, ImColor(colour).Value); }
		~ScopedColour() { ImGui::PopStyleColor(); }
	};

	class VORTEX_API ScopedFont
	{
	public:
		ScopedFont(const ScopedFont&) = delete;
		ScopedFont& operator=(const ScopedFont&) = delete;
		ScopedFont(ImFont* font) { ImGui::PushFont(font); }
		~ScopedFont() { ImGui::PopFont(); }
	};

	class VORTEX_API ScopedID
	{
	public:
		ScopedID(const ScopedID&) = delete;
		ScopedID& operator=(const ScopedID&) = delete;
		template<typename T>
		ScopedID(T id) { ImGui::PushID(id); }
		~ScopedID() { ImGui::PopID(); }
	};

	class VORTEX_API ScopedColourStack
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

	class VORTEX_API ScopedStyleStack
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

	// -----------------------------------------------------------------------------------------------------------------------------------------------------------
	// Colours

	VORTEX_API inline static ImColor ColorWithValue(const ImColor& color, float value)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(hue, sat, std::min(value, 1.0f));
	}

	VORTEX_API inline static ImColor ColorWithSaturation(const ImColor& color, float saturation)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(hue, std::min(saturation, 1.0f), val);
	}

	VORTEX_API inline static ImColor ColorWithHue(const ImColor& color, float hue)
	{
		const ImVec4& colRaw = color.Value;
		float h, s, v;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, h, s, v);
		return ImColor::HSV(std::min(hue, 1.0f), s, v);
	}

	VORTEX_API inline static ImColor ColorWithAlpha(const ImColor& color, float multiplier)
	{
		ImVec4 colRaw = color.Value;
		colRaw.w = multiplier;
		return colRaw;
	}

	VORTEX_API inline static ImColor ColorWithMultipliedValue(const ImColor& color, float multiplier)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(hue, sat, std::min(val * multiplier, 1.0f));
	}

	VORTEX_API inline static ImColor ColorWithMultipliedSaturation(const ImColor& color, float multiplier)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(hue, std::min(sat * multiplier, 1.0f), val);
	}

	VORTEX_API inline static ImColor ColorWithMultipliedHue(const ImColor& color, float multiplier)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(std::min(hue * multiplier, 1.0f), sat, val);
	}

	VORTEX_API inline static ImColor ColorWithMultipliedAlpha(const ImColor& color, float multiplier)
	{
		ImVec4 colRaw = color.Value;
		colRaw.w *= multiplier;
		return colRaw;
	}

	VORTEX_API inline static const char* GenerateLabelID(std::string_view label)
	{
		*std::format_to_n(s_LabelIDBuffer, std::size(s_LabelIDBuffer), "{}##{}", label, s_Counter++).out = 0;
		return s_LabelIDBuffer;
	}

	VORTEX_API inline static ImRect GetItemRect()
	{
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	VORTEX_API inline static ImRect RectExpanded(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	VORTEX_API inline static ImRect RectOffset(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x += x;
		result.Min.y += y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	VORTEX_API inline static ImRect RectOffset(const ImRect& rect, ImVec2 xy)
	{
		return RectOffset(rect, xy.x, xy.y);
	}

	namespace Draw {

		VORTEX_API inline static void Underline(bool fullWidth = false, float offsetX = 0.0f, float offsetY = -1.0f)
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

		VORTEX_API inline static void Separator(ImVec2 size, ImVec4 color)
		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, color);
			ImGui::BeginChild("sep", size);
			ImGui::EndChild();
			ImGui::PopStyleColor();
		}
	}

	VORTEX_API inline static void SetTooltip(const char* message)
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

	VORTEX_API inline static void HelpMarker(const char* desc)
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

	VORTEX_API inline static void DrawItemActivityOutline(float rounding = 0.0f, bool drawWhenInactive = false, ImColor colourWhenActive = ImColor(80, 80, 80))
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

	VORTEX_API inline static void PushID()
	{
		Gui::PushID(s_UIContextID++);
	}

	VORTEX_API inline static void PopID()
	{
		Gui::PopID();
		s_UIContextID--;
	}

	VORTEX_API inline static void PushFont(const char* fontName)
	{
		if (String::FastCompare(fontName, "Bold"))
			Gui::PushFont(Gui::GetIO().Fonts->Fonts[0]);
		else if (String::FastCompare(fontName, "Large"))
			Gui::PushFont(Gui::GetIO().Fonts->Fonts[1]);
		else if (String::FastCompare(fontName, "Huge"))
			Gui::PushFont(Gui::GetIO().Fonts->Fonts[2]);
	}

	VORTEX_API inline static void PopFont()
	{
		Gui::PopFont();
	}

	VORTEX_API inline static bool IsInputEnabled()
	{
		const auto& io = ImGui::GetIO();
		return (io.ConfigFlags & ImGuiConfigFlags_NoMouse) == 0 && (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard) == 0;
	}

	VORTEX_API inline static void SetInputEnabled(bool enabled)
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

	VORTEX_API inline static void ShiftCursorX(float distance)
	{
		Gui::SetCursorPosX(Gui::GetCursorPosX() + distance);
	}

	VORTEX_API inline static void ShiftCursorY(float distance)
	{
		Gui::SetCursorPosY(Gui::GetCursorPosY() + distance);
	}

	VORTEX_API inline static void ShiftCursor(float x, float y)
	{
		ImVec2 cursorPos = Gui::GetCursorPos();
		Gui::SetCursorPos(ImVec2{ cursorPos.x + x, cursorPos.y + y });
	}

	VORTEX_API inline static void BeginPropertyGrid(uint32_t columns = 2)
	{
		PushID();
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8.0f, 8.0f });
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });
		Gui::Columns(columns);
	}

	VORTEX_API inline static void BeginPropertyGrid(float columnWidth)
	{
		PushID();
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8.0f, 8.0f });
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });
		Gui::Columns(2);
		Gui::SetColumnWidth(0, columnWidth);
	}

	VORTEX_API inline static void EndPropertyGrid()
	{
		Gui::Columns(1);
		Gui::PopStyleVar(2);
		ShiftCursorY(18.0f);
		PopID();
	}

	VORTEX_API inline static bool BeginPopup(const char* str_id, ImGuiWindowFlags flags)
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

	VORTEX_API inline static void EndPopup()
	{
		ImGui::PopStyleVar(); // WindowPadding;
		ImGui::PopStyleColor(); // HeaderHovered;
		ImGui::EndPopup();
	}

	VORTEX_API inline static bool PropertyGridHeader(const char* label, bool defaultOpen = true)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_FramePadding;

		if (defaultOpen)
		{
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

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

	VORTEX_API inline static void EndTreeNode()
	{
		Gui::TreePop();
	}

	VORTEX_API inline static void BeginCheckboxGroup(const char* label)
	{
		Gui::Text(label);
		Gui::NextColumn();
		Gui::PushItemWidth(-1);
	}

	VORTEX_API inline static void EndCheckboxGroup()
	{
		Gui::PopItemWidth();
		Gui::NextColumn();
		s_CheckboxCount = 0;
	}

	VORTEX_API inline static bool DrawVec3Controls(const std::string& label, Math::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f, float min = 0.0f, float max = 0.0f, std::function<void()> uiCallback = nullptr)
	{
		bool modified = false;

		Gui::PushID(label.c_str());

		Gui::Columns(2);
		Gui::SetColumnWidth(0, columnWidth);
		Gui::Text(label.c_str());
		Gui::NextColumn();

		Gui::PushMultiItemsWidths(values.length(), Gui::CalcItemWidth());
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		auto button = [&](auto label, auto index) {
			PushFont("Bold");
			if (Gui::Button(label, buttonSize))
			{
				values[index] = resetValue;

				if (uiCallback != nullptr)
				{
					std::invoke(uiCallback);
				}

				modified = true;
			}
			PopFont();
			Gui::PopStyleColor(3);
		};

		auto drag = [&](auto label, auto index) {
			if (Gui::DragFloat(label, &values[index], 0.1f, min > 0 ? min : -FLT_MAX, max > 0 ? max : FLT_MAX, "%.2f"))
			{
				if (uiCallback != nullptr)
				{
					std::invoke(uiCallback);
				}

				modified = true;
			}
			DrawItemActivityOutline();
			Gui::PopItemWidth();
		};

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		button("X", 0);
		Gui::SameLine();

		drag("##X", 0);
		Gui::SameLine();

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		button("Y", 1);
		Gui::SameLine();

		drag("##Y", 1);
		Gui::SameLine();

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		button("Z", 2);
		Gui::SameLine();

		drag("##Z", 2);

		Gui::PopStyleVar();
		Gui::Columns(1);
		Gui::PopID();

		return modified;
	}

	VORTEX_API inline static bool DrawVec2Controls(const std::string& label, Math::vec2& values, float resetValue = 0.0f, float columnWidth = 100.0f, float min = 0.0f, float max = 0.0f, std::function<void()> uiCallback = nullptr)
	{
		bool modified = false;

		Gui::PushID(label.c_str());

		Gui::Columns(2);
		Gui::SetColumnWidth(0, columnWidth);
		Gui::Text(label.c_str());
		Gui::NextColumn();

		Gui::PushMultiItemsWidths(values.length(), Gui::CalcItemWidth());
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		auto button = [&](auto label, auto index) {
			PushFont("Bold");
			if (Gui::Button(label, buttonSize))
			{
				values[index] = resetValue;

				if (uiCallback != nullptr)
				{
					std::invoke(uiCallback);
				}

				modified = true;
			}
			PopFont();
			Gui::PopStyleColor(3);
		};

		auto drag = [&](auto label, auto index) {
			if (Gui::DragFloat(label, &values[index], 0.1f, min > 0 ? min : -FLT_MAX, max > 0 ? max : FLT_MAX, "%.2f"))
			{
				if (uiCallback != nullptr)
				{
					std::invoke(uiCallback);
				}

				modified = true;
			}
			DrawItemActivityOutline();
			Gui::PopItemWidth();
		};

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		button("X", 0);
		Gui::SameLine();

		drag("##X", 0);
		Gui::SameLine();

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		button("Y", 1);
		Gui::SameLine();

		drag("##Y", 1);
		Gui::SameLine();

		Gui::PopStyleVar();
		Gui::Columns(1);
		Gui::PopID();

		return modified;
	}

	VORTEX_API inline static bool PropertyCheckboxGroup(const char* label, bool& value, const char* desc = nullptr)
	{
		bool modified = false;

		if (++s_CheckboxCount > 1)
			Gui::SameLine();

		Gui::Text(label);
		if (desc) {
			Gui::SameLine();
			HelpMarker(desc);
		}
		Gui::SameLine();

		if (Gui::Checkbox(GenerateID(), &value))
			modified = true;

		DrawItemActivityOutline();

		return modified;
	}

	VORTEX_API inline static bool Property(const char* label, bool& value, const char* desc = nullptr)
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

		if (Gui::Checkbox(std::format("##{}", label).c_str(), &value))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	VORTEX_API inline static bool Property(const char* label, char& value, float speed = 1.0f, int min = 0, int max = 0, const char* desc = nullptr)
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

		if (Gui::DragScalar(std::format("##{}", label).c_str(), ImGuiDataType_S8, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	VORTEX_API inline static bool Property(const char* label, unsigned char& value, float speed = 1.0f, int min = 0, int max = 0, const char* desc = nullptr)
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

		if (Gui::DragScalar(std::format("##{}", label).c_str(), ImGuiDataType_U8, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	VORTEX_API inline static bool Property(const char* label, short& value, float speed = 1.0f, int min = 0, int max = 0, const char* desc = nullptr)
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

		if (Gui::DragScalar(std::format("##{}", label).c_str(), ImGuiDataType_S16, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	VORTEX_API inline static bool Property(const char* label, unsigned short& value, float speed = 1.0f, int min = 0, int max = 0, const char* desc = nullptr)
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

		if (Gui::DragScalar(std::format("##{}", label).c_str(), ImGuiDataType_U16, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	VORTEX_API inline static bool Property(const char* label, int& value, float speed = 1.0f, int min = 0, int max = 0, const char* desc = nullptr)
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

		if (Gui::DragInt(std::format("##{}", label).c_str(), &value, speed, min, max))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	VORTEX_API inline static bool Property(const char* label, uint32_t& value, float speed = 1.0f, int min = 0, int max = 0, const char* desc = nullptr)
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

		if (Gui::DragScalar(std::format("##{}", label).c_str(), ImGuiDataType_U32, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	VORTEX_API inline static bool Property(const char* label, long long& value, float speed = 1.0f, int min = 0, int max = 0, const char* desc = nullptr)
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

		if (Gui::DragScalar(std::format("##{}", label).c_str(), ImGuiDataType_S64, &value, speed, (const void*)&min, (const void*)&max))
		{
			modified = true;
		}

		DrawItemActivityOutline();

		Gui::PopItemWidth();
		Gui::NextColumn();
		Draw::Underline();

		return modified;
	}

	VORTEX_API inline static bool Property(const char* label, unsigned long long& value, float speed = 1.0f, int min = 0, int max = 0, const char* desc = nullptr)
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

	VORTEX_API inline static bool Property(const char* label, float& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.2f", const char* desc = nullptr)
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

	VORTEX_API inline static bool Property(const char* label, double& value, float speed = 1.0f, int min = 0, int max = 0, const char* desc = nullptr)
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

	VORTEX_API inline static bool Property(const char* label, Math::vec2& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.2f", const char* desc = nullptr)
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

	VORTEX_API inline static bool Property(const char* label, Math::vec3& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.2f", const char* desc = nullptr)
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

	VORTEX_API inline static bool Property(const char* label, Math::vec4& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.2f", const char* desc = nullptr)
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

	VORTEX_API inline static bool Property(const char* label, Math::vec3* value, const char* desc = nullptr)
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

	VORTEX_API inline static bool Property(const char* label, Math::vec4* value, const char* desc = nullptr)
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
	
	VORTEX_API inline static bool Property(const char* label, std::string& value, bool readOnly = false, bool multiline = false, bool enterReturnsTrue = false, const char* desc = nullptr)
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

	VORTEX_API inline static bool PropertyWithButton(const char* label, const char* buttonLabel, std::string& value, const std::function<void()>& onClickedFn, bool readOnly = false, bool multiline = false, bool enterReturnsTrue = false, const char* desc = nullptr)
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

	VORTEX_API inline static bool PropertySlider(const char* label, float& value, float min = 0.0f, float max = 0.0f, const char* format = "%.2f", const char* desc = nullptr)
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

	VORTEX_API inline static bool MultilineTextBox(const char* label, std::string& value, bool readOnly = false, const char* desc = nullptr)
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

	VORTEX_API inline static bool ColoredButton(const char* label, const ImVec4& backgroundColor, const ImVec4& foregroundColor, ImVec2 buttonSize)
	{
		ScopedColour textColor(ImGuiCol_Text, foregroundColor);
		ScopedColour buttonColor(ImGuiCol_Button, backgroundColor);
		return ImGui::Button(label, buttonSize);
	}

	template <typename TEnum, typename TUnderlying = int32_t>
	VORTEX_API inline static bool PropertyDropdown(const char* label, const char** options, uint32_t count, TEnum& selected, const char* desc = nullptr)
	{
		TUnderlying selectedIndex = (TUnderlying)selected;
		const char* current = options[selectedIndex];

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

		const ImGuiComboFlags flags = ImGuiComboFlags_None;

		const std::string id = "##" + std::string(label);
		if (Gui::BeginCombo(id.c_str(), current, flags))
		{
			for (uint32_t i = 0; i < count; i++)
			{
				const bool isSelected = current == options[i];
				if (Gui::Selectable(options[i], isSelected) || (Gui::IsItemFocused() && Gui::IsKeyPressed(ImGuiKey_Enter)))
				{
					current = options[i];
					selected = (TEnum)i;
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

		return modified;
	}

	VORTEX_API inline static bool PropertyDropdownSearch(const char* label, const char** options, uint32_t count, std::string& selected, ImGuiTextFilter& textFilter, const std::function<void()>& clearCallback = nullptr, const char* desc = nullptr)
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

	struct VORTEX_API UIImage
	{
		SharedReference<Texture2D> Texture = nullptr;
		Math::vec2 Size = Math::vec2(64.0f, 64.0f);
		Math::vec4 TintColor = Math::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	};

	VORTEX_API bool PropertyDropdownSearchWithImages(const char* label, const char** options, uint32_t count, const std::vector<UIImage>& images, std::string& selected, const UIImage& selectedImage, ImGuiTextFilter& textFilter, const std::function<void()>& clearCallback = nullptr, const char* desc = nullptr);

	VORTEX_API inline static void ImageEx(uint32_t rendererID, const ImVec2& size = ImVec2(64, 64), const ImVec4& tintColor = ImVec4(1, 1, 1, 1), const ImVec4& borderColor = ImVec4(0, 0, 0, 0))
	{
		Gui::Image((ImTextureID)rendererID, size, { 0, 1 }, { 1, 0 }, tintColor, borderColor);
	}

	VORTEX_API inline static void ImageEx(const SharedReference<Texture2D>& texture, const ImVec2& size = ImVec2(64, 64), const ImVec4& bgColor = ImVec4(0, 0, 0, 0), const ImVec4& tintColor = ImVec4(0, 0, 0, 0))
	{
		Gui::Image((ImTextureID)texture->GetRendererID(), size, { 0, 1 }, { 1, 0 }, bgColor, tintColor);
	}

	VORTEX_API inline static bool ImageButton(const char* label, const SharedReference<Texture2D>& texture, const ImVec2& size = ImVec2(64, 64), const ImVec4& bgColor = ImVec4(0, 0, 0, 0), const ImVec4& tintColor = ImVec4(0, 0, 0, 0))
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

	VORTEX_API inline static bool ImageButtonEx(const SharedReference<Texture2D>& texture, const ImVec2& size = ImVec2(64, 64), const ImVec4& bgColor = ImVec4(0, 0, 0, 0), const ImVec4& tintColor = ImVec4(0, 0, 0, 0))
	{
		bool modified = false;

		if (Gui::ImageButton((ImTextureID)texture->GetRendererID(), size, { 0, 1 }, { 1, 0 }, -1, bgColor, tintColor))
		{
			modified = true;
		}

		return modified;
	}

	VORTEX_API inline static void DrawButtonImage(const SharedReference<Texture2D>& imageNormal, const SharedReference<Texture2D>& imageHovered, const SharedReference<Texture2D>& imagePressed,
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

	VORTEX_API inline static void DrawButtonImage(const SharedReference<Texture2D>& imageNormal, const SharedReference<Texture2D>& imageHovered, const SharedReference<Texture2D>& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	};

	VORTEX_API inline static void DrawButtonImage(const SharedReference<Texture2D>& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax);
	};

	VORTEX_API inline static void DrawButtonImage(const SharedReference<Texture2D>& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	};


	VORTEX_API inline static void DrawButtonImage(const SharedReference<Texture2D>& imageNormal, const SharedReference<Texture2D>& imageHovered, const SharedReference<Texture2D>& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	};

	VORTEX_API inline static void DrawButtonImage(const SharedReference<Texture2D>& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	};

	VORTEX_API inline static bool FontSelector(const char* label, const char** options, uint32_t count, ImFont* selected)
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

		return modified;
	}

	VORTEX_API inline static bool TreeNode(const char* label, bool defaultOpen = true)
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

	VORTEX_API inline static bool ShowMessageBox(const char* title, bool* open, const ImVec2& size)
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

	template<typename T>
	VORTEX_API inline static void Table(const char* tableName, const char** columns, uint32_t columnCount, const ImVec2& size, T callback)
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
			Draw::Underline(true, 0.0f, 5.0f);
		}

		callback();
		ImGui::EndTable();
	}

	VORTEX_API inline static bool TableRowClickable(const char* id, float rowHeight)
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

	using VORTEX_API AssetDropFn = std::function<void(const Fs::Path&)>;

	template <typename TAssetType>
	VORTEX_API inline static bool PropertyAssetReference(const char* label, const std::string& filepath, AssetHandle& assetHandle, const AssetDropFn& onAssetDroppedFn, const AssetRegistry& registry, const char* desc = nullptr)
	{
		bool modified = false;

		AssetType assetType = TAssetType::GetStaticType();

		std::vector<const char*> options;
		std::vector<AssetHandle> handles;

		std::vector<std::string> filepaths;

		for (const auto& [assetHandle, metadata] : registry)
		{
			if (metadata.Type != assetType)
				continue;

			VX_CORE_ASSERT(metadata.Handle != 0, "Invalid asset handle!");

			const Fs::Path& path = metadata.Filepath;

			if (path.empty())
				continue;

			filepaths.push_back(path.string());
			handles.push_back(assetHandle);
		}

		for (size_t i = 0; i < filepaths.size(); i++)
		{
			options.push_back(filepaths[i].c_str());
		}

		PushID();

		auto OnClearedFn = [&] {
			assetHandle = 0;
			modified = true;
			Gui::CloseCurrentPopup();
		};

		std::string current = filepath;
		BeginPropertyGrid();
		if (PropertyDropdownSearch(label, options.data(), options.size(), current, s_TextFilters[s_UIContextID - 1], OnClearedFn, desc))
		{
			size_t pos = 0;
			for (size_t i = 0; i < options.size(); i++)
			{
				if (current.find(options[i]) != std::string::npos)
				{
					pos = i;
					break;
				}
			}

			assetHandle = handles[pos];
			modified = true;
			Gui::CloseCurrentPopup();
		}
		EndPropertyGrid();

		PopID();

		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				if (onAssetDroppedFn != nullptr)
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					Fs::Path droppedFilepath = Fs::Path(path);

					std::invoke(onAssetDroppedFn, droppedFilepath);
				}
			}

			Gui::EndDragDropTarget();
		}

		return modified;
	}

	struct VORTEX_API PropertyAssetImageReferenceSettings
	{
		const char* Label = nullptr;
		std::string* CurrentFilepath = nullptr;
		AssetHandle* CurrentHandle = nullptr;
		UIImage CurrentImage;
		std::vector<UIImage> AvailableImages;
		AssetDropFn OnAssetDroppedFn = nullptr;
		const AssetRegistry* Registry = nullptr;
		const char* Description = nullptr;
	};

	template <typename TImageAssetType>
	VORTEX_API inline static bool PropertyAssetImageReference(const PropertyAssetImageReferenceSettings& settings)
	{
		bool modified = false;

		AssetType assetType = TImageAssetType::GetStaticType();
		
		static const AssetType availableAssetTypes[] = { AssetType::TextureAsset, AssetType::EnvironmentAsset };
		static const size_t availableAssetTypesCount = VX_ARRAYSIZE(availableAssetTypes);

		bool isAvailableAssetType = false;

		for (size_t i = 0; i < availableAssetTypesCount; i++)
		{
			AssetType availableAssetType = availableAssetTypes[i];
			if (assetType != availableAssetType)
				continue;

			isAvailableAssetType = true;
		}

		if (!isAvailableAssetType)
		{
			std::string availableAssetTypesStr = "";
			for (size_t i = 0; i < availableAssetTypesCount; i++)
			{
				const std::string assetTypeStr = Utils::StringFromAssetType(availableAssetTypes[i]);
				const bool isLastType = (i == availableAssetTypesCount - 1);
				availableAssetTypesStr += assetTypeStr + (isLastType ? " " : ", ");
			}
			VX_CONSOLE_LOG_ERROR("UI::PropertyAssetImageReference<T>() expected one of these types [{}], got: '{}'", availableAssetTypesStr, Utils::StringFromAssetType(assetType));
			return false;
		}

		std::vector<const char*> options;
		std::vector<AssetHandle> handles;
		std::vector<std::string> filepaths;

		for (const auto& [assetHandle, metadata] : *settings.Registry)
		{
			if (metadata.Type != assetType)
				continue;

			VX_CORE_ASSERT(metadata.Handle != 0, "Invalid asset handle!");

			const Fs::Path& path = metadata.Filepath;

			if (path.empty())
				continue;

			filepaths.push_back(path.string());
			handles.push_back(assetHandle);
		}

		for (size_t i = 0; i < filepaths.size(); i++)
		{
			options.push_back(filepaths[i].c_str());
		}

		PushID();

		auto OnClearedFn = [&] {
			*settings.CurrentHandle = 0;
			modified = true;
			Gui::CloseCurrentPopup();
		};

		std::string currentFilepath = *settings.CurrentFilepath;
		BeginPropertyGrid();
		if (PropertyDropdownSearchWithImages(settings.Label, options.data(), options.size(), settings.AvailableImages, currentFilepath, settings.CurrentImage, s_TextFilters[s_UIContextID - 1], OnClearedFn, settings.Description))
		{
			size_t pos = 0;
			for (size_t i = 0; i < options.size(); i++)
			{
				if (currentFilepath.find(options[i]) != std::string::npos)
				{
					pos = i;
					break;
				}
			}

			*settings.CurrentHandle = handles[pos];
			modified = true;
			Gui::CloseCurrentPopup();
		}
		EndPropertyGrid();

		PopID();

		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				if (settings.OnAssetDroppedFn != nullptr)
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					Fs::Path droppedFilepath = Fs::Path(path);

					std::invoke(settings.OnAssetDroppedFn, droppedFilepath);
				}
			}

			Gui::EndDragDropTarget();
		}

		return modified;
	}

}
