#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/TimeStep.h"

#include "Vortex/Physics/3D/PhysicsTypes.h"

namespace Vortex {

	enum class VORTEX_API RuntimeArgumentType
	{
		TimeStep,
		Collision,
		ForceAndTorque,
	};

	union VORTEX_API RuntimeArgument
	{
		TimeStep Delta;
		Collision CollisionArg;
		std::pair<Math::vec3, Math::vec3> ForceAndTorque;

		RuntimeArgument(TimeStep delta)
			: Delta(delta) { }

		RuntimeArgument(const Collision& collision)
			: CollisionArg(collision) { }

		RuntimeArgument(const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
			: ForceAndTorque(forceAndTorque) { }
	};

	struct VORTEX_API RuntimeMethodArgument
	{
		RuntimeArgument Arg;
		RuntimeArgumentType Type;

		RuntimeMethodArgument(TimeStep delta)
			: Arg(delta), Type(RuntimeArgumentType::TimeStep) { }

		RuntimeMethodArgument(const Collision& collision)
			: Arg(collision), Type(RuntimeArgumentType::Collision) { }

		RuntimeMethodArgument(const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
			: Arg(forceAndTorque), Type(RuntimeArgumentType::ForceAndTorque) { }

		RuntimeArgument GetArgument() const { return Arg; }
		RuntimeArgumentType GetType() const { return Type; }

		bool Is(RuntimeArgumentType type) const { return Type == type; }

		TimeStep AsTimeStep() const
		{
			RuntimeArgumentType type = RuntimeArgumentType::TimeStep;
			VX_CORE_ASSERT(Is(type), "unexpected argument type!");

			if (!Is(type))
			{
				VX_CORE_ASSERT(false, "unexpected argument type!");
				return TimeStep();
			}

			return Arg.Delta;
		}

		Collision AsCollision() const
		{
			RuntimeArgumentType type = RuntimeArgumentType::Collision;
			VX_CORE_ASSERT(Is(type), "unexpected argument type!");

			if (!Is(type))
			{
				VX_CORE_ASSERT(false, "unexpected argument type!");
				return Collision();
			}

			return Arg.CollisionArg;
		}

		std::pair<Math::vec3, Math::vec3> AsForceAndTorque() const
		{
			RuntimeArgumentType type = RuntimeArgumentType::ForceAndTorque;
			VX_CORE_ASSERT(Is(type), "unexpected argument type!");

			if (!Is(type))
			{
				VX_CORE_ASSERT(false, "unexpected argument type!");
				return std::pair<Math::vec3, Math::vec3>();
			}

			return Arg.ForceAndTorque;
		}
	};

}
