#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex::vxstl {

	template <typename t_value_type>
	class VORTEX_API result
	{
	public:
		using VORTEX_API error_type = std::string;

	public:
		result() = delete;
		result(const result&) = delete;
		result& operator=(const result&) = delete;

		result(const t_value_type& value)
			: m_data(value), m_ok(1) { }

		result(const error_type& error)
			: m_data(error), m_ok(0) { }

		VX_FORCE_INLINE bool ok() const { return m_ok; }

		VX_FORCE_INLINE const t_value_type& value() const
		{
			VX_CORE_ASSERT(ok(), "result was error!");
			return m_data.value;
		}
		
		VX_FORCE_INLINE const error_type& err() const
		{
			VX_CORE_ASSERT(!ok(), "result was ok!");
			return m_data.error;
		}

	private:
		union result_data
		{
			t_value_type value;
			error_type error;

			result_data(const t_value_type& value)
				: value(value) { }
			result_data(const error_type& error)
				: error(error) { }
		} m_data;
		bool m_ok;
	};

}
