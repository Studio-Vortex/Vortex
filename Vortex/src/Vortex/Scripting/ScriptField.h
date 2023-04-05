#pragma once

#include "Vortex/Scripting/ScriptFieldTypes.h"

#include <string>

extern "C"
{
	typedef struct _MonoClassField MonoClassField;
}

namespace Vortex {

	struct VORTEX_API ScriptField
	{
		ScriptFieldType Type = ScriptFieldType::None;
		std::string Name = "";

		MonoClassField* ClassField = nullptr;
	};

}
