#pragma once

#include "Vortex/Scripting/ScriptFieldTypes.h"

#include <unordered_map>
#include <string>

namespace Vortex {

	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single",      ScriptFieldType::Float       },
		{ "System.Double",      ScriptFieldType::Double      },
		{ "System.Boolean",     ScriptFieldType::Bool        },
		{ "System.Char",        ScriptFieldType::Char        },
		{ "System.Int16",       ScriptFieldType::Short       },
		{ "System.Int32",       ScriptFieldType::Int         },
		{ "System.Int64",       ScriptFieldType::Long        },
		{ "System.Byte",        ScriptFieldType::Byte        },
		{ "System.UInt16",      ScriptFieldType::UShort      },
		{ "System.UInt32",      ScriptFieldType::UInt        },
		{ "System.UInt64",      ScriptFieldType::ULong       },
		{ "Vortex.Vector2",     ScriptFieldType::Vector2     },
		{ "Vortex.Vector3",     ScriptFieldType::Vector3     },
		{ "Vortex.Vector4",     ScriptFieldType::Vector4     },
		{ "Vortex.Color3",      ScriptFieldType::Color3      },
		{ "Vortex.Color4",      ScriptFieldType::Color4      },
		{ "Vortex.Entity",      ScriptFieldType::Actor      },
		{ "Vortex.AssetHandle", ScriptFieldType::AssetHandle },
	};

}
