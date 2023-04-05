#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {

	struct VORTEX_API AudioClip
	{
		std::string Filepath = "";
		std::string Name = "";
		float Length = 0.0f;
	};

}
