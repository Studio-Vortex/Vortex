#include "vxpch.h"
#include "RaycastCallback2D.h"

#include <box2d/b2_fixture.h>

namespace Vortex {
	
	RaycastCallback2D::RaycastCallback2D()
		: fixture(nullptr), point(), normal(), fraction() { }

	float RaycastCallback2D::ReportFixture(b2Fixture* fixture_, const b2Vec2& point_, const b2Vec2& normal_, float fraction_)
	{
		fixture = fixture_;
		point = point_;
		normal = normal_;
		fraction = fraction_;
		return fraction;
	}

}
