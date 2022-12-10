#pragma once

#include "Sparky/Scene/Entity.h"

namespace Sparky {

	struct TransformComponent;
	struct RigidBodyComponent;

	class PhysicsActor
	{
	public:
		PhysicsActor(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody);
		~PhysicsActor() = default;

		inline Entity GetEntity() const { return m_Entity; }

		inline operator bool() const { return m_Entity; }
		inline bool operator ==(const PhysicsActor& other) const
		{
			return m_Entity == other.m_Entity;
		}

	private:
		Entity m_Entity;
	};

}

