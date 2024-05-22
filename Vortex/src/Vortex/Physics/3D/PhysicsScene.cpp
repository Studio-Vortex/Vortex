#include "vxpch.h"
#include "PhysicsScene.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/3D/PhysicsUtils.h"
#include "Vortex/Physics/3D/PhysicsFilterShader.h"
#include "Vortex/Physics/3D/PhysicsContactListener.h"

#include "Vortex/Project/Project.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Vortex {

	struct PhysicsSceneInternalData
	{
		physx::PxScene* Scene = nullptr;

		PhysicsContactListener ContactListener;

		Math::vec3 Gravity = Math::vec3(0.0f, -9.81f, 0.0f);
		uint32_t PositionIterations = 8;
		uint32_t VeloctiyIterations = 2;
	};

	static PhysicsSceneInternalData s_Data;

	void PhysicsScene::Init()
	{
		physx::PxSceneDesc sceneDescription = physx::PxSceneDesc(*((physx::PxTolerancesScale*)Physics::GetTolerancesScale()));
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		sceneDescription.gravity = PhysicsUtils::ToPhysXVector(s_Data.Gravity);

		SharedReference<Project> project = Project::GetActive();
		const ProjectProperties& properties = project->GetProperties();

		sceneDescription.broadPhaseType = PhysicsUtils::VortexBroadphaseTypeToPhysXBroadphaseType(properties.PhysicsProps.BroadphaseModel);
		sceneDescription.frictionType = PhysicsUtils::VortexFrictionTypeToPhysXFrictionType(properties.PhysicsProps.FrictionModel);

		sceneDescription.cpuDispatcher = ((physx::PxDefaultCpuDispatcher*)Physics::GetDispatcher());
		sceneDescription.filterShader = PhysicsFilterShader::FilterShader;
		sceneDescription.simulationEventCallback = &s_Data.ContactListener;

		s_Data.Scene = ((physx::PxPhysics*)Physics::GetPhysicsSDK())->createScene(sceneDescription);
	}

	void PhysicsScene::Shutdown()
	{
		s_Data.Scene->release();
		s_Data.Scene = nullptr;
	}

	void PhysicsScene::Simulate(TimeStep delta, bool block)
	{
		s_Data.Scene->simulate(delta);
		s_Data.Scene->fetchResults(block);
	}

	const Math::vec3& PhysicsScene::GetGravity()
	{
		return s_Data.Gravity;
	}

	void PhysicsScene::SetGravity(const Math::vec3& gravity)
	{
		if (s_Data.Scene)
		{
			s_Data.Scene->setGravity(PhysicsUtils::ToPhysXVector(gravity));

			// we need to wake up all the actors because a number of them could be sleeping
			WakeUpActors();
		}

		s_Data.Gravity = gravity;
	}

	uint32_t PhysicsScene::GetVelocityIterations()
	{
		return s_Data.VeloctiyIterations;
	}

	void PhysicsScene::SetVelocityIterations(uint32_t iterations)
	{
		s_Data.VeloctiyIterations = iterations;
	}

	uint32_t PhysicsScene::GetPositionIterations()
	{
		return s_Data.PositionIterations;
	}

	void PhysicsScene::SetPositionIterations(uint32_t iterations)
	{
		s_Data.PositionIterations = iterations;
	}

	void PhysicsScene::WakeUpActors()
	{
		VX_PROFILE_FUNCTION();

		if (!s_Data.Scene || !Physics::GetContextScene())
		{
			return;
		}

		physx::PxActorTypeFlags flags = physx::PxActorTypeFlag::eRIGID_DYNAMIC;
		uint32_t count = s_Data.Scene->getNbActors(flags);

		physx::PxActor** buffer = new physx::PxActor*[count];
		s_Data.Scene->getActors(flags, buffer, count);

		for (uint32_t i = 0; i < count; i++)
		{
			physx::PxRigidDynamic* actor = buffer[i]->is<physx::PxRigidDynamic>();

			const bool gravityDisabled = actor->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY;
			const bool isAwake = !actor->isSleeping();

			if (gravityDisabled || isAwake)
				continue;

			actor->wakeUp();
		}

		delete[] buffer;
	}

	void* PhysicsScene::GetScene()
	{
		return s_Data.Scene;
	}

	uint32_t PhysicsScene::Raycast(const Math::vec3& origin, const Math::vec3& direction, float maxDistance, RaycastHit* outInfo)
	{
		physx::PxRaycastBuffer hitInfo;
		const bool result = s_Data.Scene->raycast(PhysicsUtils::ToPhysXVector(origin), PhysicsUtils::ToPhysXVector(Math::Normalize(direction)), maxDistance, hitInfo);

		if (result == false)
			return 0;

		const void* userData = hitInfo.block.actor->userData;

		if (userData == nullptr)
		{
			*outInfo = RaycastHit();
			return 1;
		}

		const PhysicsBodyData* physicsBodyData = (const PhysicsBodyData*)userData;
		UUID actor = physicsBodyData->ActorUUID;

		outInfo->ActorID = actor;
		outInfo->Position = PhysicsUtils::FromPhysXVector(hitInfo.block.position);
		outInfo->Normal = PhysicsUtils::FromPhysXVector(hitInfo.block.normal);
		outInfo->Distance = hitInfo.block.distance;

		return 1;
	}

}
