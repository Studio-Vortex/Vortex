#pragma once

#include <mono/metadata/object.h>

#include <string>

namespace Vortex {

	class ManagedString
	{
	public:
		ManagedString(MonoString* managedString);
		ManagedString(const std::string& data);

		const std::string& String() const;
		MonoString* GetAddressOf() const;

	private:
		std::string m_String;
		mutable MonoString* m_ManagedString = nullptr;
	};

}
