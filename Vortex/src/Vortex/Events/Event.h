#pragma once

#include "Vortex/Core/Base.h"

#include <sstream>
#include <string>

namespace Vortex {

	enum class VORTEX_API EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocused, WindowLostFocus, WindowMoved, WindowDragDrop,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum VORTEX_API EventCategory
	{
		None = 0,
		EventCategoryApplication   = BIT(0),
		EventCategoryInput         = BIT(1),
		EventCategoryKeyboard      = BIT(2),
		EventCategoryMouse         = BIT(3),
		EventCategoryMouseButton   = BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class VORTEX_API Event
	{
	public:
		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	};

	class VORTEX_API EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event) {}

		// F will be deduced by the compiler
		template <typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}

			return false;
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& stream, const Event& e)
	{
		return stream << e.ToString();
	}

}
