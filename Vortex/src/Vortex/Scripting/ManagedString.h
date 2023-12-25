#pragma once

#include <mono/metadata/object.h>

#include <string>

namespace Vortex {

	class ManagedString
	{
	public:
		ManagedString(MonoString* managedString);

		const std::string& String() const;

	private:
		std::string m_String;
	};

}
