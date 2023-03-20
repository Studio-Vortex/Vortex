#pragma once

#include "Vortex/Scripting/ScriptField.h"

namespace Vortex {

#define VX_SCRIPT_FIELD_MAX_BYTES 16

	struct ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}
		~ScriptFieldInstance() = default;

		template <typename TFieldType>
		TFieldType GetValue() const
		{
			static_assert(sizeof(TFieldType) <= VX_SCRIPT_FIELD_MAX_BYTES, "Type too large!");
			return *(TFieldType*)m_Buffer;
		}

		template <typename TFieldType>
		void SetValue(TFieldType value)
		{
			static_assert(sizeof(TFieldType) <= VX_SCRIPT_FIELD_MAX_BYTES, "Type too large!");
			memcpy(m_Buffer, &value, sizeof(TFieldType));
		}

	private:
		uint8_t m_Buffer[VX_SCRIPT_FIELD_MAX_BYTES];

	private:
		friend class ScriptEngine;
		friend class ScriptInstance;
	};

}
