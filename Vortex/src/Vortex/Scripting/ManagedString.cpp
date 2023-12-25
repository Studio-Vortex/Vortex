#include "vxpch.h"
#include "ManagedString.h"

namespace Vortex {
	
	ManagedString::ManagedString(MonoString* managedString)
	{
		char* data = mono_string_to_utf8(managedString);
		m_String = std::string(data);
		mono_free(data);
	}

	const std::string& ManagedString::String() const
	{
		return m_String;
	}

}
