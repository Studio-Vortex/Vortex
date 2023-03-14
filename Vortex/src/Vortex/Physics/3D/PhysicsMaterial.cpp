#include "vxpch.h"
#include "PhysicsMaterial.h"

#include "Vortex/Physics/3D/Physics.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Vortex {

	PhysicsMaterial::PhysicsMaterial(float staticFriction, float dynamicFriction, float bounciness, CombineMode frictionCombine, CombineMode bouncinessCombine)
		: StaticFriction(staticFriction),
		DynamicFriction(dynamicFriction),
		Bounciness(bounciness),
		FrictionCombineMode(frictionCombine),
		BouncinessCombineMode(bouncinessCombine) { }

}
