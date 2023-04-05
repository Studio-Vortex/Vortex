#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"

namespace Vortex {

	struct VORTEX_API AudioCone
	{
		float InnerAngle = Math::Deg2Rad(10.0f);
		float OuterAngle = Math::Deg2Rad(45.0f);
		float OuterGain = 0.0f;
	};

}
