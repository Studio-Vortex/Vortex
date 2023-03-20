#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex {

	enum class VORTEX_API ScriptFieldType
	{
		None = 0,
		Float, Double,
		Bool, Char, Short, Int, Long,
		Byte, UShort, UInt, ULong,
		Vector2, Vector3, Vector4,
		Color3, Color4,
		Entity, AssetHandle
	};

}
