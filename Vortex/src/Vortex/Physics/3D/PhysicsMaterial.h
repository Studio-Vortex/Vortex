#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Physics/3D/PhysXTypes.h"

namespace Vortex {

	class PhysicsMaterial : public Asset
	{
	public:
		float StaticFriction = 0.6f;
		float DynamicFriction = 0.6f;
		float Bounciness = 0.0f;

		CombineMode FrictionCombineMode = CombineMode::Average;
		CombineMode BouncinessCombineMode = CombineMode::Average;

	public:
		PhysicsMaterial() = default;
		PhysicsMaterial(
			float staticFriction,
			float dynamicFriction,
			float bounciness,
			CombineMode frictionCombine = CombineMode::Average,
			CombineMode bouncinessCombine = CombineMode::Average
		);
		~PhysicsMaterial() = default;

		static AssetType GetStaticType() { return AssetType::PhysicsMaterial; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }
	};

}
