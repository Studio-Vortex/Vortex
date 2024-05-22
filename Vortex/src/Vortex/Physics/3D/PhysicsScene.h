#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/TimeStep.h"

#include "Vortex/Physics/3D/PhysicsTypes.h"

namespace Vortex {

	class VORTEX_API PhysicsScene
	{
	public:
		static void Init();
		static void Shutdown();

		static void Simulate(TimeStep delta, bool block = true);

		static const Math::vec3& GetGravity();
		static void SetGravity(const Math::vec3& gravity);

		static uint32_t GetVelocityIterations();
		static void SetVelocityIterations(uint32_t iterations);

		static uint32_t GetPositionIterations();
		static void SetPositionIterations(uint32_t iterations);

		static void WakeUpActors();

		static void* GetScene();

		static uint32_t Raycast(const Math::vec3& origin, const Math::vec3& direction, float maxDistance, RaycastHit* outInfo);
	};

}
