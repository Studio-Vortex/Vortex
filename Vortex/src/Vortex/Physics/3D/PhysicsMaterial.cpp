#include "vxpch.h"
#include "PhysicsMaterial.h"

namespace Vortex {
	
	PhysicsMaterial::PhysicsMaterial(const std::string& name, float staticFriction, float dynamicFriction, float bounciness)
		: Name(name), StaticFriction(staticFriction), DynamicFriction(dynamicFriction), Bounciness(bounciness) { }

	PhysicsMaterial::PhysicsMaterial(float staticFriction, float dynamicFriction, float bounciness, CombineMode frictionCombine, CombineMode bouncinessCombine)
		: StaticFriction(staticFriction),
		DynamicFriction(dynamicFriction),
		Bounciness(bounciness),
		FrictionCombineMode(frictionCombine),
		BouncinessCombineMode(bouncinessCombine) { }

}
