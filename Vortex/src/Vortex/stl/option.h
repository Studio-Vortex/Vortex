#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex::vxstl {

	template <typename t_value_type>
	class VORTEX_API option
	{
	public:
		option(const option&) = delete;
		option& operator=(const option&) = delete;

		option(const t_value_type& value)
			: m_value(value), m_some(1) { }

		option()
			: m_value(t_value_type()), m_some(0) { }

	public:

		VX_FORCE_INLINE const t_value_type& value() const
		{
			VX_CORE_ASSERT(some(), "value was none!");
			return m_value;
		}

		VX_FORCE_INLINE bool some() const { return m_some; }

	private:
		t_value_type m_value;
		bool m_some;
	};

	template <typename t_value_type>
	option<t_value_type> make_option(const t_value_type& value)
	{
		return option<t_value_type>(value);
	}

	template <typename t_value_type>
	option<t_value_type> make_option()
	{
		return option<t_value_type>();
	}



}
