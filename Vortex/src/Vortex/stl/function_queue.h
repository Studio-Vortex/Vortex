#pragma once

#include "Vortex/Core/Base.h"

#include <functional>
#include <vector>
#include <mutex>

namespace Vortex::vxstl {

	template <typename t_return_type>
	class VORTEX_API function_queue
	{
	public:
		using VORTEX_API function_type = std::function<t_return_type()>;
		using VORTEX_API mutex_type = std::mutex;

	public:
		function_queue(const function_queue&) = delete;
		function_queue& operator=(const function_queue&) = delete;

		function_queue() = default;
		~function_queue() = default;

	public:

		VX_FORCE_INLINE void queue(const function_type& fn)
		{
			std::scoped_lock<mutex_type> lock(m_mutex);

			m_function_queue.emplace_back(fn);
		}

		VX_FORCE_INLINE void execute()
		{
			const size_t count = size();

			for (size_t i = 0; i < count; i++)
			{
				const function_type& fn = m_function_queue[i];
				std::invoke(fn);
			}
		}

		VX_FORCE_INLINE const function_type* data() const
		{
			return m_function_queue.data();
		}

		VX_FORCE_INLINE void clear()
		{
			std::scoped_lock<mutex_type> lock(m_mutex);

			m_function_queue.clear();
		}

		VX_FORCE_INLINE bool empty() const
		{
			return m_function_queue.empty();
		}

		VX_FORCE_INLINE size_t size() const
		{
			return m_function_queue.size();
		}

	private:
		std::vector<function_type> m_function_queue;
		mutex_type m_mutex;
	};

}
