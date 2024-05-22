#include "vxpch.h"
#include "PhysicsShapes.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/3D/PhysicsTypes.h"
#include "Vortex/Physics/3D/PhysicsUtils.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Asset/AssetManager.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Vortex {

	ColliderShape::ColliderShape(ColliderType type, Actor actor, bool isShared)
		: m_Type(type), m_Material(nullptr), m_Actor(actor), m_IsShared(isShared) { }

	ColliderShape::~ColliderShape()
	{
		Release();
	}

	void ColliderShape::Release()
	{
		if (m_Material == nullptr)
		{
			return;
		}

		VX_CORE_ASSERT(m_Material, "Material was invalid!");

		m_Material->release();
	}

	void ColliderShape::SetMaterial(SharedReference<PhysicsMaterial>& material)
	{
		if (m_Material != nullptr)
			m_Material->release();

		m_Material = ((physx::PxPhysics*)Physics::GetPhysicsSDK())->createMaterial(material->StaticFriction, material->DynamicFriction, material->Bounciness);
		m_Material->setFrictionCombineMode((physx::PxCombineMode::Enum)material->FrictionCombineMode);
		m_Material->setRestitutionCombineMode((physx::PxCombineMode::Enum)material->BouncinessCombineMode);
	}

	BoxColliderShape::BoxColliderShape(BoxColliderComponent& component, physx::PxRigidActor& pxActor, Actor actor)
		: ColliderShape(ColliderType::Box, actor)
	{
		SharedReference<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);
		if (!material)
		{
			component.Material = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>(0.6f, 0.6f, 0.0f);
			material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);
		}

		SetMaterial(material);

		Scene* scene = Physics::GetContextScene();
		TransformComponent worldSpaceTransform = scene->GetWorldSpaceTransform(actor);

		Math::vec3 colliderSize = Math::Abs(worldSpaceTransform.Scale * component.HalfSize);
		physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(colliderSize.x, colliderSize.y, colliderSize.z);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(pxActor, boxGeometry, *m_Material);
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !component.IsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.IsTrigger);
		m_Shape->setLocalPose(PhysicsUtils::ToPhysXTransform(component.Offset, Math::vec3(0.0f)));
		m_Shape->userData = this;
	}

	const Math::vec3& BoxColliderShape::GetHalfSize() const
	{
		return m_Actor.GetComponent<BoxColliderComponent>().HalfSize;
	}

	void BoxColliderShape::SetHalfSize(const Math::vec3& halfSize)
	{
		const PhysicsBodyData* physicsBodyData = Physics::GetPhysicsBodyData(m_Actor.GetUUID());
		TransformComponent worldSpaceTransform = physicsBodyData->ContextScene->GetWorldSpaceTransform(m_Actor);

		Math::vec3 colliderSize = Math::Abs(worldSpaceTransform.Scale * halfSize);

		physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(colliderSize.x, colliderSize.y, colliderSize.z);
		m_Shape->setGeometry(boxGeometry);

		BoxColliderComponent& boxCollider = m_Actor.GetComponent<BoxColliderComponent>();
		boxCollider.HalfSize = halfSize;
	}

	const Math::vec3& BoxColliderShape::GetOffset() const
	{
		return m_Actor.GetComponent<BoxColliderComponent>().Offset;
	}

	void BoxColliderShape::SetOffset(const Math::vec3& offset)
	{
		BoxColliderComponent& boxCollider = m_Actor.GetComponent<BoxColliderComponent>();
		m_Shape->setLocalPose(PhysicsUtils::ToPhysXTransform(offset, Math::vec3(0.0f)));
		boxCollider.Offset = offset;
	}

	bool BoxColliderShape::IsTrigger() const
	{
		return m_Actor.GetComponent<BoxColliderComponent>().IsTrigger;
	}

	void BoxColliderShape::SetTrigger(bool isTrigger)
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		BoxColliderComponent& boxCollider = m_Actor.GetComponent<BoxColliderComponent>();
		boxCollider.IsTrigger = isTrigger;
	}

	void BoxColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void BoxColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		VX_CORE_ASSERT(actor, "Physics Actor was invalid!");
		VX_CORE_ASSERT(m_Shape, "Shape was invalid!");
		actor->detachShape(*m_Shape);
	}

	SphereColliderShape::SphereColliderShape(SphereColliderComponent& component, physx::PxRigidActor& pxActor, Actor actor)
		: ColliderShape(ColliderType::Sphere, actor)
	{
		SharedReference<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);
		if (!material)
		{
			component.Material = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>();
			material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);
		}

		SetMaterial(material);

		Scene* scene = Physics::GetContextScene();
		TransformComponent worldSpaceTransform = scene->GetWorldSpaceTransform(actor);

		float largestComponent = Math::Max(worldSpaceTransform.Scale.x, Math::Max(worldSpaceTransform.Scale.y, worldSpaceTransform.Scale.z));

		physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(largestComponent * component.Radius);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(pxActor, sphereGeometry, *m_Material);
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !component.IsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.IsTrigger);
		m_Shape->setLocalPose(PhysicsUtils::ToPhysXTransform(component.Offset, Math::vec3(0.0f)));
		m_Shape->userData = this;
	}

	float SphereColliderShape::GetRadius() const
	{
		return m_Actor.GetComponent<SphereColliderComponent>().Radius;
	}

	void SphereColliderShape::SetRadius(float radius)
	{
		const PhysicsBodyData* physicsBodyData = Physics::GetPhysicsBodyData(m_Actor.GetUUID());
		TransformComponent worldSpaceTransform = physicsBodyData->ContextScene->GetWorldSpaceTransform(m_Actor);

		float largestComponent = Math::Max(worldSpaceTransform.Scale.x, Math::Max(worldSpaceTransform.Scale.y, worldSpaceTransform.Scale.z));

		physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(largestComponent * radius);
		m_Shape->setGeometry(sphereGeometry);

		SphereColliderComponent& sphereCollider = m_Actor.GetComponent<SphereColliderComponent>();
		sphereCollider.Radius = radius;
	}

	const Math::vec3& SphereColliderShape::GetOffset() const
	{
		return m_Actor.GetComponent<SphereColliderComponent>().Offset;
	}

	void SphereColliderShape::SetOffset(const Math::vec3& offset)
	{
		SphereColliderComponent& sphereCollider = m_Actor.GetComponent<SphereColliderComponent>();
		m_Shape->setLocalPose(PhysicsUtils::ToPhysXTransform(offset, Math::vec3(0.0f)));
		sphereCollider.Offset = offset;
	}

	bool SphereColliderShape::IsTrigger() const
	{
		return m_Actor.GetComponent<SphereColliderComponent>().IsTrigger;
	}

	void SphereColliderShape::SetTrigger(bool isTrigger)
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		SphereColliderComponent& sphereCollider = m_Actor.GetComponent<SphereColliderComponent>();
		sphereCollider.IsTrigger = isTrigger;
	}

	void SphereColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void SphereColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		VX_CORE_ASSERT(actor, "Physics Actor was invalid!");
		VX_CORE_ASSERT(m_Shape, "Shape was invalid!");
		actor->detachShape(*m_Shape);
	}

	CapsuleColliderShape::CapsuleColliderShape(CapsuleColliderComponent& component, physx::PxRigidActor& pxActor, Actor actor)
		: ColliderShape(ColliderType::Capsule, actor)
	{
		SharedReference<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);
		if (!material)
		{
			component.Material = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>(0.6f, 0.6f, 0.0f);
			material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);
		}

		SetMaterial(material);

		Scene* scene = Physics::GetContextScene();
		TransformComponent worldSpaceTransform = scene->GetWorldSpaceTransform(actor);

		float radiusScale = Math::Max(worldSpaceTransform.Scale.x, worldSpaceTransform.Scale.z);

		physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(radiusScale * component.Radius, (component.Height * 0.5f) * worldSpaceTransform.Scale.y);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(pxActor, capsuleGeometry, *m_Material);
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !component.IsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.IsTrigger);
		m_Shape->setLocalPose(PhysicsUtils::ToPhysXTransform(component.Offset, Math::vec3(0.0f, 0.0f, physx::PxHalfPi)));
		m_Shape->userData = this;
	}

	float CapsuleColliderShape::GetRadius() const
	{
		return m_Actor.GetComponent<CapsuleColliderComponent>().Radius;
	}

	void CapsuleColliderShape::SetRadius(float radius)
	{
		const PhysicsBodyData* physicsBodyData = Physics::GetPhysicsBodyData(m_Actor.GetUUID());
		TransformComponent worldSpaceTransform = physicsBodyData->ContextScene->GetWorldSpaceTransform(m_Actor);

		float radiusScale = Math::Max(worldSpaceTransform.Scale.x, worldSpaceTransform.Scale.z);

		physx::PxCapsuleGeometry oldGeometry;
		m_Shape->getCapsuleGeometry(oldGeometry);

		physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(radiusScale * radius, oldGeometry.halfHeight);
		m_Shape->setGeometry(capsuleGeometry);

		CapsuleColliderComponent& capsuleCollider = m_Actor.GetComponent<CapsuleColliderComponent>();
		capsuleCollider.Radius = radius;
	}

	float CapsuleColliderShape::GetHeight() const
	{
		return m_Actor.GetComponent<CapsuleColliderComponent>().Height;
	}

	void CapsuleColliderShape::SetHeight(float height)
	{
		const PhysicsBodyData* physicsBodyData = Physics::GetPhysicsBodyData(m_Actor.GetUUID());
		TransformComponent worldSpaceTransform = physicsBodyData->ContextScene->GetWorldSpaceTransform(m_Actor);

		physx::PxCapsuleGeometry oldGeometry;
		m_Shape->getCapsuleGeometry(oldGeometry);

		physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(oldGeometry.radius, (height * 0.5f) * worldSpaceTransform.Scale.y);
		m_Shape->setGeometry(capsuleGeometry);

		CapsuleColliderComponent& capsuleCollider = m_Actor.GetComponent<CapsuleColliderComponent>();
		capsuleCollider.Height = height;
	}

	const Math::vec3& CapsuleColliderShape::GetOffset() const
	{
		return m_Actor.GetComponent<CapsuleColliderComponent>().Offset;
	}

	void CapsuleColliderShape::SetOffset(const Math::vec3& offset)
	{
		CapsuleColliderComponent& capsuleCollider = m_Actor.GetComponent<CapsuleColliderComponent>();
		m_Shape->setLocalPose(PhysicsUtils::ToPhysXTransform(offset, Math::vec3(0.0f, 0.0f, physx::PxHalfPi)));
		capsuleCollider.Offset = offset;
	}

	bool CapsuleColliderShape::IsTrigger() const
	{
		return m_Actor.GetComponent<CapsuleColliderComponent>().IsTrigger;
	}

	void CapsuleColliderShape::SetTrigger(bool isTrigger)
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		CapsuleColliderComponent& capsuleCollider = m_Actor.GetComponent<CapsuleColliderComponent>();
		capsuleCollider.IsTrigger = isTrigger;
	}

	void CapsuleColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void CapsuleColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		VX_CORE_ASSERT(actor, "Physics Actor was invalid!");
		VX_CORE_ASSERT(m_Shape, "Shape was invalid!");
		actor->detachShape(*m_Shape);
	}

	ConvexMeshShape::ConvexMeshShape(MeshColliderComponent& component, physx::PxRigidActor& pxActor, Actor actor)
		: ColliderShape(ColliderType::ConvexMesh, actor, component.UseSharedShape)
	{

	}

	const Math::vec3& ConvexMeshShape::GetOffset() const
	{
		return Math::vec3(); // TODO: insert return statement here
	}

	void ConvexMeshShape::SetOffset(const Math::vec3& offset)
	{
	}

	bool ConvexMeshShape::IsTrigger() const
	{
		return false;
	}

	void ConvexMeshShape::SetTrigger(bool isTrigger)
	{

	}

	void ConvexMeshShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		for (auto& shape : m_Shapes)
		{
			shape->setSimulationFilterData(filterData);
		}
	}

	void ConvexMeshShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		VX_CORE_ASSERT(actor, "Physics Actor was invalid!");

		for (auto& shape : m_Shapes)
		{
			VX_CORE_ASSERT(shape, "Shape was invalid!");
			actor->detachShape(*shape);
		}
	}

	TriangleMeshShape::TriangleMeshShape(MeshColliderComponent& component, physx::PxRigidActor& pxActor, Actor actor)
		: ColliderShape(ColliderType::TriangleMesh, actor, component.UseSharedShape)
	{

	}

	const Math::vec3& TriangleMeshShape::GetOffset() const
	{
		return Math::vec3(); // TODO: insert return statement here
	}

	void TriangleMeshShape::SetOffset(const Math::vec3& offset)
	{
	}

	bool TriangleMeshShape::IsTrigger() const
	{
		return false;
	}

	void TriangleMeshShape::SetTrigger(bool isTrigger)
	{

	}

	void TriangleMeshShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		for (auto& shape : m_Shapes)
		{
			shape->setSimulationFilterData(filterData);
		}
	}

	void TriangleMeshShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		VX_CORE_ASSERT(actor, "Physics Actor was invalid!");

		for (auto& shape : m_Shapes)
		{
			VX_CORE_ASSERT(shape, "Shape was invalid!");
			actor->detachShape(*shape);
		}
	}

	SharedShapeManager::SharedShapeData* SharedShapeManager::CreateSharedShapeData(ColliderType type, AssetHandle colliderHandle)
	{
		return nullptr;
	}

}
