#pragma once

#include "Event.h"

#include "Vortex/Utils/FileSystem.h"

#include <vector>

namespace Vortex {

	class VORTEX_API WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height)
			: m_Width(width), m_Height(height) { }

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		uint32_t m_Width;
		uint32_t m_Height;
	};

	class VORTEX_API WindowMoveEvent : public Event
	{
	public:
		WindowMoveEvent(uint32_t positionX, uint32_t positionY)
			: m_PositionX(positionX), m_PositionY(positionY) { }

		inline uint32_t GetPositionX() const { return m_PositionX; }
		inline uint32_t GetPositionY() const { return m_PositionY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowMoveEvent: " << m_PositionX << ", " << m_PositionY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowMove)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		uint32_t m_PositionX;
		uint32_t m_PositionY;
	};

	class VORTEX_API WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() { }

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class VORTEX_API WindowDragDropEvent : public Event
	{
	public:
		WindowDragDropEvent(const std::vector<Fs::Path>& paths)
			: m_Filepaths(paths) { }

		WindowDragDropEvent(std::vector<Fs::Path>&& paths)
			: m_Filepaths(std::move(paths)) { }

		const std::vector<Fs::Path>& GetPaths() const { return m_Filepaths; }

		EVENT_CLASS_TYPE(WindowDragDrop)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		std::vector<Fs::Path> m_Filepaths;
	};

}
