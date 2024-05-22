#include "vxpch.h"
#include "ManagedString.h"

#include "Vortex/Scripting/ScriptUtils.h"

#include <mono/jit/jit.h>

namespace Vortex {
	
	ManagedString::ManagedString(MonoString* managedString)
	{
		if (!managedString)
		{
			return;
		}

		if (mono_string_length(managedString) == 0)
		{
			return;
		}

		MonoError error;
		char* utf8 = mono_string_to_utf8_checked(managedString, &error);
		if (ScriptUtils::RT_CheckError(&error))
		{
			return;
		}

		m_String = std::string(utf8);
		mono_free(utf8);
	}

	ManagedString::ManagedString(const std::string& data)
	{
		m_ManagedString = mono_string_new_len(mono_domain_get(), data.c_str(), data.size());
	}

	const std::string& ManagedString::String() const
	{
		return m_String;
	}

	MonoString* ManagedString::GetAddressOf() const
	{
		return m_ManagedString;
	}

}
