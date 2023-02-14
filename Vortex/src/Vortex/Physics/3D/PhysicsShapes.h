#pragma once

#include "Vortex/Core/Math.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Physics/3D/PhysXAPIHelpers.h"

namespace Vortex {

	enum class ColliderType
	{
		Box, Sphere, Capsule, ConvexMesh, TriangleMesh,
	};

	class ColliderShape
	{
	protected:
		ColliderShape(ColliderType type, Entity entity, bool isShared = false)
			: m_Type(type), m_Entity(entity), m_IsShared(isShared) { }

	public:
		virtual ~ColliderShape()
		{
			Release();
		}

		void Release()
		{
			if (m_IsShared && m_Material == nullptr)
			{
				return;
			}

			VX_CORE_ASSERT(m_Material, "Material was invalid!");
			m_Material->release();
		}

		ColliderType GetType() const { return m_Type; }

		virtual const Math::vec3& GetOffset() const = 0;
		virtual void SetOffset(const Math::vec3& offset) = 0;

		virtual bool IsTrigger() const = 0;
		virtual void SetTrigger(bool isTrigger) = 0;

		virtual void SetFilterData(const physx::PxFilterData& filterData) = 0;

		virtual void DetachFromActor(physx::PxRigidActor* actor) = 0;

		virtual const char* GetShapeName() const = 0;

		physx::PxMaterial& GetMaterial() const { return *m_Material; }
		void SetMaterial(const PhysicsMaterialComponent& material);
		bool IsShared() const { return m_IsShared; }

		virtual bool IsValid() const { return m_Material != nullptr; }

	protected:
		ColliderType m_Type;
		Entity m_Entity;
		physx::PxMaterial* m_Material = nullptr;
		bool m_IsShared = false;
	};

	class ConvexMeshShape : public ColliderShape
	{
	public:
		ConvexMeshShape(MeshColliderComponent& component, const physx::PxRigidActor& actor, Entity entity);
		~ConvexMeshShape();

		virtual const glm::vec3& GetOffset() const
		{
			static glm::vec3 defaultOffset = glm::vec3(0.0f);
			return defaultOffset;
		}
		void SetOffset(const glm::vec3& offset) {}

		bool IsTrigger() const override { return m_Entity.GetComponent<MeshColliderComponent>().IsTrigger; }
		void SetTrigger(bool isTrigger) override;

		void SetFilterData(const physx::PxFilterData& filterData) override;

		void DetachFromActor(physx::PxRigidActor* actor) override;

		const char* GetShapeName() const override { return "ConvexMeshCollider"; }
		bool IsValid() const override { return ColliderShape::IsValid() && !m_Shapes.empty(); }

		static ColliderType GetStaticType() { return ColliderType::ConvexMesh; }

	private:
		std::vector<physx::PxShape*> m_Shapes;
	};

	class TriangleMeshShape : public ColliderShape
	{
	public:
		TriangleMeshShape(MeshColliderComponent& component, const physx::PxRigidActor& actor, Entity entity);
		~TriangleMeshShape();

		virtual const glm::vec3& GetOffset() const
		{
			static glm::vec3 defaultOffset = glm::vec3(0.0f);
			return defaultOffset;
		}
		virtual void SetOffset(const glm::vec3& offset) {}

		virtual bool IsTrigger() const override { return m_Entity.GetComponent<MeshColliderComponent>().IsTrigger; }
		virtual void SetTrigger(bool isTrigger) override;

		virtual void SetFilterData(const physx::PxFilterData& filterData) override;

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

		virtual const char* GetShapeName() const override { return "TriangleMeshCollider"; }
		bool IsValid() const override { return ColliderShape::IsValid() && !m_Shapes.empty(); }

		static ColliderType GetStaticType() { return ColliderType::TriangleMesh; }

	private:
		std::vector<physx::PxShape*> m_Shapes;
	};

}
