#include "vxpch.h"
#include "ManagedString.h"

#include <mono/jit/jit.h>

namespace Vortex {
	
	ManagedString::ManagedString(MonoString* managedString)
	{
		char* data = mono_string_to_utf8(managedString);
		m_String = std::string(data);
		mono_free(data);
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
