#pragma once

#include <box2d/b2_world_callbacks.h>
#include <box2d/b2_math.h>

namespace Vortex {

	// Derived class of Box2D RayCastCallback
	class RaycastCallback2D : public b2RayCastCallback
	{
	public:
		RaycastCallback2D();

		float ReportFixture(b2Fixture* fixture_, const b2Vec2& point_, const b2Vec2& normal_, float fraction_) override;

		b2Fixture* fixture; // This is the fixture that was hit by the raycast
		b2Vec2 point;
		b2Vec2 normal;
		float fraction;
	};

}
