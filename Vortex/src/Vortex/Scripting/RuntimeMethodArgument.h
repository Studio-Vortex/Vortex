#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/TimeStep.h"

#include "Vortex/Physics/3D/PhysicsData.h"

namespace Vortex {

	union VORTEX_API RuntimeMethodArgument
	{
		TimeStep Delta;
		Collision CollisionArg;
		std::pair<Math::vec3, Math::vec3> ForceAndTorque;

		RuntimeMethodArgument(TimeStep delta)
			: Delta(delta) { }

		RuntimeMethodArgument(const Collision& collision)
			: CollisionArg(collision) { }
		
		RuntimeMethodArgument(const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
			: ForceAndTorque(forceAndTorque) { }
	};

}
