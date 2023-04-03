#pragma once

#include "Event.h"

#include "Vortex/Utils/FileSystem.h"

#include <vector>

namespace Vortex {

	class VORTEX_API WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height)
			: m_Width(width), m_Height(height) {}

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizedEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		uint32_t m_Width;
		uint32_t m_Height;
	};

	class VORTEX_API WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class VORTEX_API WindowDragDropEvent : public Event
	{
	public:
		WindowDragDropEvent(const std::vector<std::filesystem::path>& paths)
			: m_Filepaths(paths)
		{
		}

		WindowDragDropEvent(std::vector<std::filesystem::path>&& paths)
			: m_Filepaths(std::move(paths))
		{
		}

		const std::vector<std::filesystem::path>& GetPaths() const { return m_Filepaths; }

		EVENT_CLASS_TYPE(WindowDragDrop)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		std::vector<std::filesystem::path> m_Filepaths;
	};

}
