#include "vxpch.h"
#include "PhysicsShapes.h"

#include "Vortex/Physics/3D/Physics.h"

namespace Vortex {

	void ColliderShape::SetMaterial(const PhysicsMaterialComponent& material)
	{
		if (m_Material != nullptr)
			m_Material->release();
	}

	ConvexMeshShape::ConvexMeshShape(MeshColliderComponent& component, const physx::PxRigidActor& actor, Entity entity)
		: ColliderShape(ColliderType::ConvexMesh, entity, component.UseSharedShape)
	{

	}

	ConvexMeshShape::~ConvexMeshShape()
	{

	}

	void ConvexMeshShape::SetTrigger(bool isTrigger)
	{

	}

	void ConvexMeshShape::SetFilterData(const physx::PxFilterData& filterData)
	{

	}

	void ConvexMeshShape::DetachFromActor(physx::PxRigidActor* actor)
	{

	}

	TriangleMeshShape::TriangleMeshShape(MeshColliderComponent& component, const physx::PxRigidActor& actor, Entity entity)
		: ColliderShape(ColliderType::TriangleMesh, entity, component.UseSharedShape)
	{

	}

	TriangleMeshShape::~TriangleMeshShape()
	{

	}

	void TriangleMeshShape::SetTrigger(bool isTrigger)
	{

	}

	void TriangleMeshShape::SetFilterData(const physx::PxFilterData& filterData)
	{

	}

	void TriangleMeshShape::DetachFromActor(physx::PxRigidActor* actor)
	{

	}

}
