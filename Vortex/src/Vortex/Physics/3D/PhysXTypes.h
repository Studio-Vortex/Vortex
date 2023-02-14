#pragma once

#include "Vortex/Core/Base.h"

#include <cstdint>

namespace Vortex {

	enum class BroadphaseType
	{
		SweepAndPrune,
		MultiBoxPrune,
		AutomaticBoxPrune
	};

	enum class FrictionType
	{
		Patch,
		OneDirectional,
		TwoDirectional
	};

	enum class CombineMode : uint8_t
	{
		Average,
		Max,
		Min,
		Mulitply
	};

	enum class CookingResult
	{
		None,
		Success,
		ZeroAreaTestFailed,
		PolygonLimitReached,
		LargeTriangle,
		InvalidMesh,
		Failure
	};

	enum class FilterGroup : uint32_t
	{
		Static = BIT(0),
		Dynamic = BIT(1),
		Kinematic = BIT(2),
		All = Static | Dynamic | Kinematic
	};

	enum class ECollisionComplexity : uint8_t
	{
		Default = 0, // Use simple for collision and complex for scene queries
		UseComplexAsSimple = 1, // Use complex for collision AND scene queries
		UseSimpleAsComplex = 2 // Use simple for collision AND scene queries
	};

	enum class ActorLockFlag : uint8_t
	{
		TranslationX = BIT(0),
		TranslationY = BIT(1),
		TranslationZ = BIT(2),
		Translation = TranslationX | TranslationY | TranslationZ,

		RotationX = BIT(3),
		RotationY = BIT(4),
		RotationZ = BIT(5),
		Rotation = RotationX | RotationY | RotationZ
	};

	enum class ForceMode : uint8_t
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum class NonWalkableMode : uint8_t
	{
		PreventClimbing = 0,
		PreventClimbingAndForceSliding = 1,
	};

	enum class CapsuleClimbMode : uint8_t
	{
		Easy = 0,
		Constrained = 1,
	};

}
