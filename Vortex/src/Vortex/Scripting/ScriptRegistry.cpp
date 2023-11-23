#include "vxpch.h"
#include "ScriptRegistry.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Core/Input/Input.h"
#include "Vortex/Core/Log.h"

#include "Vortex/Core/Math/Noise.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"

#include "Vortex/Scripting/ScriptUtils.h"
#include "Vortex/Scripting/ScriptEngine.h"

#include "Vortex/Audio/Audio.h"
#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Audio/AudioTypes.h"
#include "Vortex/Audio/AudioUtils.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/3D/PhysXTypes.h"
#include "Vortex/Physics/3D/PhysXAPIHelpers.h"
#include "Vortex/Physics/3D/PhysicsMaterial.h"
#include "Vortex/Physics/3D/PhysicsShapes.h"
#include "Vortex/Physics/2D/Physics2D.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/RenderCommand.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"

#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"

#include "Vortex/Utils/Random.h"
#include "Vortex/Utils/Time.h"

#include "Vortex/UI/UI.h"

#include <mono/metadata/object.h>
#include <mono/jit/jit.h>
#include <mono/metadata/reflection.h>

#include <box2d/b2_body.h>
#include <box2d/b2_world.h>
#include <box2d/b2_fixture.h>

#include <imgui.h>

#include <cstdlib>
#include <ctime>

namespace Vortex {

#define VX_REGISTER_INTERNAL_CALL(icall) mono_add_internal_call("Vortex.InternalCalls::" #icall, InternalCalls::icall)

	struct ScriptingData
	{
		std::unordered_map<MonoType*, std::function<void(Entity)>> EntityAddComponentFuncs;
		std::unordered_map<MonoType*, std::function<bool(Entity)>> EntityHasComponentFuncs;
		std::unordered_map<MonoType*, std::function<void(Entity)>> EntityRemoveComponentFuncs;

		Entity HoveredEntity = Entity{};

		float SceneStartTime = 0.0f;

		int32_t NextSceneBuildIndex = -1;

		Math::vec4 RaycastDebugLineColor = Math::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	};

	static ScriptingData s_Data;

	namespace InternalCalls {

		static Scene* GetContextScene()
		{
			Scene* contextScene = ScriptEngine::GetContextScene();
			VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

			return contextScene;
		}

		static Entity GetEntity(UUID entityUUID)
		{
			Entity entity = GetContextScene()->TryGetEntityWithUUID(entityUUID);
			VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

			return entity;
		}

#pragma region Application

		void Application_Quit()
		{
			Scene* contextScene = GetContextScene();

			Application::Get().Close();
		}

		void Application_GetSize(Math::vec2* outSize)
		{
			Scene* contextScene = GetContextScene();

			*outSize = Application::Get().GetWindow().GetSize();
		}

		void Application_GetPosition(Math::vec2* outPosition)
		{
			Scene* contextScene = GetContextScene();

			*outPosition = Application::Get().GetWindow().GetPosition();
		}

		bool Application_IsMaximized()
		{
			Scene* contextScene = GetContextScene();

			return Application::Get().GetWindow().IsMaximized();
		}

#pragma endregion

#pragma region SceneRenderer

		float SceneRenderer_GetBloomThreshold()
		{
			Scene* contextScene = GetContextScene();

			return Renderer::GetBloomSettings().x;
		}

		void SceneRenderer_SetBloomThreshold(float threshold)
		{
			Scene* contextScene = GetContextScene();

			Renderer::SetBloomThreshold(threshold);
		}

		float SceneRenderer_GetBloomSoftKnee()
		{
			Scene* contextScene = GetContextScene();

			return Renderer::GetBloomSettings().y;
		}

		void SceneRenderer_SetBloomSoftKnee(float softKnee)
		{
			Scene* contextScene = GetContextScene();

			Renderer::SetBloomKnee(softKnee);
		}

		float SceneRenderer_GetBloomUnknown()
		{
			Scene* contextScene = GetContextScene();

			return Renderer::GetBloomSettings().z;
		}

		void SceneRenderer_SetBloomUnknown(float unknown)
		{
			Scene* contextScene = GetContextScene();

			Renderer::SetBloomIntensity(unknown);
		}

		float SceneRenderer_GetExposure()
		{
			Scene* contextScene = GetContextScene();

			return Renderer::GetSceneExposure();
		}

		void SceneRenderer_SetExposure(float exposure)
		{
			Scene* contextScene = GetContextScene();

			Renderer::SetSceneExposure(exposure);
		}

		float SceneRenderer_GetGamma()
		{
			Scene* contextScene = GetContextScene();

			return Renderer::GetSceneGamma();
		}

		void SceneRenderer_SetGamma(float gamma)
		{
			Scene* contextScene = GetContextScene();

			Renderer::SetSceneGamma(gamma);
		}

#pragma endregion

#pragma region DebugRenderer

		void DebugRenderer_BeginScene()
		{
			Scene* contextScene = GetContextScene();

			Entity primaryCameraEntity = contextScene->GetPrimaryCameraEntity();

			if (!primaryCameraEntity)
			{
				VX_CORE_WARN_TAG("Scripting", "Scene must include a primary camera to call debug render functions!");
				return;
			}

			SceneCamera& camera = primaryCameraEntity.GetComponent<CameraComponent>().Camera;
			Renderer2D::BeginScene(camera, primaryCameraEntity.GetTransform().GetTransform());
		}

		void DebugRenderer_SetClearColor(Math::vec3* color)
		{
			Scene* contextScene = GetContextScene();

			RenderCommand::SetClearColor(*color);
		}

		void DebugRenderer_DrawLine(Math::vec3* p1, Math::vec3* p2, Math::vec4* color)
		{
			Scene* contextScene = GetContextScene();

			Renderer2D::DrawLine(*p1, *p2, *color);
		}

		void DebugRenderer_DrawQuadBillboard(Math::vec3* translation, Math::vec2* size, Math::vec4* color)
		{
			Scene* contextScene = GetContextScene();
			Entity primaryCameraEntity = contextScene->GetPrimaryCameraEntity();

			if (!primaryCameraEntity)
			{
				VX_CORE_WARN_TAG("Scripting", "Scene must include a primary camera to call debug render functions!");
				return;
			}

			Math::mat4 cameraView = Math::Inverse(primaryCameraEntity.GetTransform().GetTransform());

			Renderer2D::DrawQuadBillboard(cameraView, *translation, *size, *color);
		}

		void DebugRenderer_DrawCircleVec2(Math::vec2* translation, Math::vec2* size, Math::vec4* color, float thickness, float fade)
		{
			Scene* contextScene = GetContextScene();

			Renderer2D::DrawCircle(*translation, *size, 0.0f, *color, thickness, fade);
		}

		void DebugRenderer_DrawCircleVec3(Math::vec3* translation, Math::vec3* size, Math::vec4* color, float thickness, float fade)
		{
			Scene* contextScene = GetContextScene();

			Renderer2D::DrawCircle(*translation, *size, 0.0f, *color, thickness, fade);
		}

		void DebugRenderer_DrawBoundingBox(Math::vec3* worldPosition, Math::vec3* size, Math::vec4* color)
		{
			Scene* contextScene = GetContextScene();

			Math::AABB aabb{
				-Math::vec3(0.5f),
				+Math::vec3(0.5f),
			};

			Math::mat4 transform = Math::Identity() * Math::Translate(*worldPosition) * Math::Scale(*size);
			Renderer2D::DrawAABB(aabb, transform, *color);
		}

		void DebugRenderer_DrawBoundingBoxFromTransform(UUID entityUUID, Math::vec4* color)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			Math::AABB aabb{
				-Math::vec3(0.5f),
				+Math::vec3(0.5f),
			};

			Math::mat4 worldSpaceTransform = contextScene->GetWorldSpaceTransformMatrix(entity);

			Renderer2D::DrawAABB(aabb, worldSpaceTransform, *color);
		}

		void DebugRenderer_Flush()
		{
			Scene* contextScene = GetContextScene();

			Renderer2D::EndScene();
		}

#pragma endregion

#pragma region Scene

		uint64_t Scene_GetPrimaryCamera()
		{
			Scene* contextScene = GetContextScene();

			Entity primaryCamera = contextScene->GetPrimaryCameraEntity();

			if (!primaryCamera)
				return 0;

			return primaryCamera.GetUUID();
		}

		bool Scene_FindEntityByID(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return (bool)entity;
		}

		uint64_t Scene_FindEntityByName(MonoString* name)
		{
			char* managedString = mono_string_to_utf8(name);

			Scene* contextScene = GetContextScene();
			Entity entity = contextScene->FindEntityByName(managedString);
			mono_free(managedString);

			if (!entity)
				return 0;

			return entity.GetUUID();
		}

		uint64_t Scene_FindChildByName(UUID entityUUID, MonoString* childName)
		{
			char* managedString = mono_string_to_utf8(childName);

			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			const auto& children = entity.Children();

			for (const auto& child : children)
			{
				Entity childEntity = contextScene->TryGetEntityWithUUID(child);

				if (childEntity && childEntity.GetName() == managedString)
				{
					return childEntity.GetUUID();
				}
			}

			return 0;
		}

		uint64_t Scene_CreateEntity(MonoString* name)
		{
			char* managedString = mono_string_to_utf8(name);

			Scene* contextScene = GetContextScene();
			Entity entity = contextScene->CreateEntity(managedString);
			mono_free(managedString);

			return entity.GetUUID();
		}

		uint64_t Scene_Instantiate(UUID entityUUID)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			if (!entity)
			{
				VX_CORE_WARN_TAG("Scripting", "Scene.Instantiate called with Invalid Entity UUID!");
				return 0;
			}

			Entity clonedEntity = contextScene->DuplicateEntity(entity);

			return clonedEntity.GetUUID();
		}

		uint64_t Scene_InstantiateAsChild(UUID entityUUID, UUID parentUUID)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);
			Entity parent = GetEntity(parentUUID);

			if (!entity || !parent)
			{
				VX_CORE_WARN_TAG("Scripting", "Scene.Instantiate called with Invalid Entity UUID!");
				return 0;
			}

			Entity clonedEntity = contextScene->DuplicateEntity(entity);

			contextScene->ParentEntity(clonedEntity, parent);

			return clonedEntity.GetUUID();
		}

		bool Scene_IsPaused()
		{
			Scene* contextScene = GetContextScene();

			return contextScene->IsPaused();
		}

		void Scene_Pause()
		{
			Scene* contextScene = GetContextScene();

			contextScene->SetPaused(true);
		}

		void Scene_Resume()
		{
			Scene* contextScene = GetContextScene();

			contextScene->SetPaused(false);
		}

		uint64_t Scene_GetHoveredEntity()
		{
			Scene* contextScene = GetContextScene();

			if (!s_Data.HoveredEntity)
				return 0;

			return s_Data.HoveredEntity.GetUUID();
		}

		uint32_t Scene_GetCurrentBuildIndex()
		{
			Scene* contextScene = GetContextScene();

			return Scene::GetActiveSceneBuildIndex();
		}

#pragma endregion

#pragma region SceneManager

		void SceneManager_LoadScene(MonoString* sceneName)
		{
			Scene* contextScene = GetContextScene();

			char* sceneNameCStr = mono_string_to_utf8(sceneName);

			uint32_t nextBuildIndex = 0;

			const BuildIndexMap& buildIndices = Scene::GetScenesInBuild();

			for (const auto& [buildIndex, sceneFilepath] : buildIndices)
			{
				if (sceneFilepath.find(sceneNameCStr) != std::string::npos)
				{
					nextBuildIndex = buildIndex;
				}
			}

			s_Data.NextSceneBuildIndex = nextBuildIndex;

			mono_free(sceneNameCStr);
		}

		void SceneManager_LoadSceneFromBuildIndex(uint32_t buildIndex)
		{
			Scene* contextScene = GetContextScene();

			const BuildIndexMap& buildIndices = Scene::GetScenesInBuild();

			const bool invalidBuildIndex = buildIndex > buildIndices.size() - 1;

			// Wrap around to beginning
			if (invalidBuildIndex)
			{
				buildIndex = 0;
			}

			s_Data.NextSceneBuildIndex = buildIndex;
		}

		uint32_t SceneManager_GetActiveSceneBuildIndex()
		{
			Scene* contextScene = GetContextScene();

			return Scene::GetActiveSceneBuildIndex();
		}

		MonoString* SceneManager_GetActiveScene()
		{
			Scene* contextScene = GetContextScene();

			const BuildIndexMap& buildIndices = Scene::GetScenesInBuild();

			std::string sceneFilepath = buildIndices.at(Scene::GetActiveSceneBuildIndex());
			size_t lastSlashPos = sceneFilepath.find_last_of("/\\");
			std::string activeSceneName = sceneFilepath.substr(lastSlashPos + 1);

			return mono_string_new(mono_domain_get(), activeSceneName.c_str());
		}

#pragma endregion

#pragma region Entity

		void Entity_AddComponent(UUID entityUUID, MonoReflectionType* componentType)
		{
			Entity entity = GetEntity(entityUUID);

			MonoType* managedType = mono_reflection_type_get_type(componentType);
			VX_CORE_ASSERT(s_Data.EntityAddComponentFuncs.find(managedType) != s_Data.EntityAddComponentFuncs.end(), "Managed type was not found in Map!");

			s_Data.EntityAddComponentFuncs.at(managedType)(entity);
		}

		bool Entity_HasComponent(UUID entityUUID, MonoReflectionType* componentType)
		{
			Entity entity = GetEntity(entityUUID);

			MonoType* managedType = mono_reflection_type_get_type(componentType);
			VX_CORE_ASSERT(s_Data.EntityHasComponentFuncs.find(managedType) != s_Data.EntityHasComponentFuncs.end(), "Managed type was not found in Map!");

			return s_Data.EntityHasComponentFuncs.at(managedType)(entity);
		}

		void Entity_RemoveComponent(UUID entityUUID, MonoReflectionType* componentType)
		{
			Entity entity = GetEntity(entityUUID);

			MonoType* managedType = mono_reflection_type_get_type(componentType);
			VX_CORE_ASSERT(s_Data.EntityRemoveComponentFuncs.find(managedType) != s_Data.EntityRemoveComponentFuncs.end(), "Managed type was not found in Map!");

			s_Data.EntityRemoveComponentFuncs.at(managedType)(entity);
		}

		MonoArray* Entity_GetChildren(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			const std::vector<UUID>& children = entity.Children();

			MonoClass* coreEntityClass = ScriptEngine::GetCoreEntityClass()->GetMonoClass();
			VX_CORE_ASSERT(coreEntityClass, "Core Entity Class was Invalid!");

			ManagedArray managedArray(coreEntityClass, children.size());
			managedArray.FillFromVector(children);
			return managedArray.GetHandle();
		}

		uint64_t Entity_GetChild(UUID entityUUID, uint32_t index)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			const auto& children = entity.Children();
			if (index > children.size() - 1)
			{
				VX_CORE_ASSERT(false, "Index out of bounds!");
				return 0;
			}

			uint64_t childUUID = children[index];
			Entity child = contextScene->TryGetEntityWithUUID(childUUID);
			VX_CORE_ASSERT(child, "Child UUID was Invalid!");

			return child.GetUUID();
		}

		MonoString* Entity_GetTag(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return mono_string_new(mono_domain_get(), entity.GetName().c_str());
		}

		MonoString* Entity_GetMarker(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return mono_string_new(mono_domain_get(), entity.GetMarker().c_str());
		}

		void Entity_SetMarker(UUID entityUUID, MonoString* monoString)
		{
			Entity entity = GetEntity(entityUUID);

			char* managedString = mono_string_to_utf8(monoString);
			entity.GetComponent<TagComponent>().Marker = std::string(managedString);
			mono_free(managedString);
		}

		bool Entity_AddChild(UUID parentUUID, UUID childUUID)
		{
			Scene* contextScene = GetContextScene();
			Entity parent = GetEntity(parentUUID);
			Entity child = GetEntity(childUUID);

			if (parent && child)
			{
				contextScene->ParentEntity(child, parent);
				return true;
			}

			return false;
		}

		bool Entity_RemoveChild(UUID parentUUID, UUID childUUID)
		{
			Scene* contextScene = GetContextScene();
			Entity parent = GetEntity(parentUUID);
			Entity child = GetEntity(childUUID);

			if (parent && child)
			{
				contextScene->UnparentEntity(child);
				return true;
			}

			return false;
		}

		MonoObject* Entity_GetScriptInstance(UUID entityUUID)
		{
			Scene* contextScene = GetContextScene();
			return ScriptEngine::GetManagedInstance(entityUUID);
		}

		void Entity_Destroy(UUID entityUUID, bool excludeChildren)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			contextScene->SubmitToDestroyEntity(entity, excludeChildren);
		}

		void Entity_DestroyTimed(UUID entityUUID, float waitTime, bool excludeChildren)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			Scene::QueueFreeData queueFreeData;
			queueFreeData.EntityUUID = entityUUID;
			queueFreeData.ExcludeChildren = excludeChildren;
			queueFreeData.WaitTime = waitTime;

			contextScene->SubmitToDestroyEntity(queueFreeData);
		}

		void Entity_SetActive(UUID entityUUID, bool isActive)
		{
			Entity entity = GetEntity(entityUUID);

			entity.SetActive(isActive);
		}

#pragma endregion

#pragma region AssetHandle

		bool AssetHandle_IsValid(AssetHandle* assetHandle)
		{
			return AssetManager::IsHandleValid(*assetHandle);
		}

#pragma endregion

#pragma region Transform Component

		void TransformComponent_GetTranslation(UUID entityUUID, Math::vec3* outTranslation)
		{
			Entity entity = GetEntity(entityUUID);

			std::string entityName = entity.GetName();

			if (entity.HasComponent<RigidBodyComponent>() && entity.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				if (entity.HasComponent<CharacterControllerComponent>())
				{
					physx::PxController* controller = Physics::GetController(entityUUID);
					*outTranslation = FromPhysXExtendedVector(controller->getPosition());

					return;
				}

				physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
				Math::vec3 translation = FromPhysXVector(actor->getGlobalPose().p);

				*outTranslation = translation;

				return;
			}
			else if (entity.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					const auto& position = body->GetPosition();
					*outTranslation = Math::vec3(position.x, position.y, entity.GetTransform().Translation.z);

					return;
				}
			}

			*outTranslation = entity.GetTransform().Translation;
		}

		void TransformComponent_SetTranslation(UUID entityUUID, Math::vec3* translation)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<RigidBodyComponent>() && entity.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				if (entity.HasComponent<CharacterControllerComponent>())
				{
					physx::PxController* controller = Physics::GetController(entityUUID);
					controller->setPosition(ToPhysXExtendedVector(*translation));

					return;
				}

				physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();

				const auto& transformComponent = entity.GetTransform();
				physx::PxTransform physxTransform = actor->getGlobalPose();
				physxTransform.p = ToPhysXVector(*translation);

				actor->setGlobalPose(physxTransform);

				return;
			}
			else if (entity.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();
				
				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					body->SetTransform({ translation->x, translation->y }, body->GetAngle());
					entity.GetTransform().Translation.z = translation->z;
					
					return;
				}
			}

			entity.GetTransform().Translation = *translation;
		}

		void TransformComponent_GetRotation(UUID entityUUID, Math::quaternion* outRotation)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<RigidBodyComponent>() && entity.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
				physx::PxTransform physxTranform = actor->getGlobalPose();
				Math::quaternion rotation = FromPhysXQuat(physxTranform.q);

				*outRotation = rotation;

				return;
			}
			else if (entity.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					const float angleRad = body->GetAngle();
					Math::vec3 currentEulers = entity.GetTransform().GetRotationEuler();
					Math::vec3 eulers(currentEulers.x, angleRad, currentEulers.z);
					*outRotation = Math::quaternion(eulers);

					return;
				}
			}

			*outRotation = entity.GetTransform().GetRotation();
		}

		void TransformComponent_SetRotation(UUID entityUUID, Math::quaternion* rotation)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<RigidBodyComponent>() && entity.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
				physx::PxTransform physxTransform = actor->getGlobalPose();
				physxTransform.q = ToPhysXQuat(*rotation);

				actor->setGlobalPose(physxTransform);
			}
			else if (entity.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					Math::vec3 translation = entity.GetTransform().Translation;
					Math::vec3 eulerAngles = Math::EulerAngles(*rotation);
					body->SetTransform({ translation.x, translation.y }, eulerAngles.z);

					return;
				}
			}

			entity.GetTransform().SetRotation(*rotation);
		}

		void TransformComponent_GetEulerAngles(UUID entityUUID, Math::vec3* outEulerAngles)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<RigidBodyComponent>() && entity.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
				Math::quaternion orientation = FromPhysXQuat(actor->getGlobalPose().q);

				*outEulerAngles = Math::EulerAngles(orientation);

				// Since we store rotation in radians we must convert to degrees here
				*outEulerAngles = Math::Rad2Deg(*outEulerAngles);

				return;
			}
			else if (entity.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					const auto& transform = entity.GetTransform();
					*outEulerAngles = { transform.GetRotationEuler().x, body->GetAngle(), transform.GetRotationEuler().z };

					// Since we store rotation in radians we must convert to degrees here
					*outEulerAngles = Math::Rad2Deg(*outEulerAngles);

					return;
				}
			}
			
			*outEulerAngles = entity.GetTransform().GetRotationEuler();

			// Since we store rotation in radians we must convert to degrees here
			*outEulerAngles = Math::Rad2Deg(*outEulerAngles);
		}

		void TransformComponent_SetEulerAngles(UUID entityUUID, Math::vec3* eulerAngles)
		{
			Entity entity = GetEntity(entityUUID);

			// Since we store rotation in radians we must convert to radians here
			*eulerAngles = Math::Deg2Rad(*eulerAngles);

			if (entity.HasComponent<RigidBodyComponent>() && entity.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
				physx::PxTransform physxTransform = actor->getGlobalPose();
				physxTransform.q = ToPhysXQuat(Math::quaternion(*eulerAngles));

				actor->setGlobalPose(physxTransform);

				return;
			}
			else if (entity.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					body->SetTransform(body->GetPosition(), Math::Deg2Rad(eulerAngles->z));
					entity.GetTransform().SetRotationEuler(*eulerAngles);

					return;
				}
			}

			entity.GetTransform().SetRotationEuler(*eulerAngles);
		}

		void TransformComponent_Rotate(UUID entityUUID, Math::vec3* eulers, Space relativeTo)
		{
			Entity entity = GetEntity(entityUUID);

			VX_CORE_ASSERT(relativeTo == Space::Local, "World Space Rotations have not been implemented yet!");

			if (relativeTo == Space::Local)
			{
				Math::quaternion rotation;
				TransformComponent_GetRotation(entityUUID, &rotation);

				*eulers = Math::Deg2Rad(*eulers);

				rotation *= Math::AngleAxis(eulers->x, Math::vec3(1.0f, 0.0f, 0.0f));
				rotation *= Math::AngleAxis(eulers->y, Math::vec3(0.0f, 1.0f, 0.0f));
				rotation *= Math::AngleAxis(eulers->z, Math::vec3(0.0f, 0.0f, 1.0f));

				TransformComponent_SetRotation(entityUUID, &rotation);
			}
			else if (relativeTo == Space::World)
			{
				
			}
		}

		void TransformComponent_RotateAround(UUID entityUUID, Math::vec3* worldPoint, Math::vec3* axis, float angle)
		{
			Entity entity = GetEntity(entityUUID);

			const float angleRad = Math::Deg2Rad(angle);
			TransformComponent worldSpaceTransform = GetContextScene()->GetWorldSpaceTransform(entity);
			Math::mat4 worldSpaceTransformMatrix = worldSpaceTransform.GetTransform();
			const Math::vec3 point = *worldPoint;
			const Math::vec3 worldSpaceTranslation = worldSpaceTransform.Translation;
			const Math::vec3 normalizedAxis = *axis;

			Math::mat4 transform;

			transform = worldSpaceTransformMatrix
				* Math::Translate(point)
				* Math::Rotate(angleRad, normalizedAxis)
				* Math::Translate(-point);

			entity.SetTransform(transform);
		}

		void TransformComponent_SetTranslationAndRotation(UUID entityUUID, Math::vec3* translation, Math::vec3* eulers)
		{
			TransformComponent_SetTranslation(entityUUID, translation);
			Math::quaternion rotation(*eulers);
			TransformComponent_SetRotation(entityUUID, &rotation);
		}

		void TransformComponent_GetScale(UUID entityUUID, Math::vec3* outScale)
		{
			Entity entity = GetEntity(entityUUID);

			*outScale = entity.GetTransform().Scale;
		}

		void TransformComponent_SetScale(UUID entityUUID, Math::vec3* scale)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetTransform().Scale = *scale;
		}

		void TransformComponent_GetWorldSpaceTransform(UUID entityUUID, Math::vec3* outTranslation, Math::quaternion* outRotation, Math::vec3* outEulers, Math::vec3* outScale)
		{
			Entity entity = GetEntity(entityUUID);

			TransformComponent worldSpaceTransform = GetContextScene()->GetWorldSpaceTransform(entity);
			*outTranslation = worldSpaceTransform.Translation;
			*outRotation = worldSpaceTransform.GetRotation();
			*outEulers = worldSpaceTransform.GetRotationEuler();
			*outScale = worldSpaceTransform.Scale;
		}

		void TransformComponent_GetTransformMatrix(UUID entityUUID, Math::mat4* outTransform)
		{
			Entity entity = GetEntity(entityUUID);

			const TransformComponent& transform = entity.GetTransform();

			*outTransform = transform.GetTransform();
		}

		void TransformComponent_SetTransformMatrix(UUID entityUUID, Math::mat4* transform)
		{
			Entity entity = GetEntity(entityUUID);

			TransformComponent& entityTransform = entity.GetTransform();

			entityTransform.SetTransform(*transform);
		}

		void TransformComponent_GetForwardDirection(UUID entityUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

			Math::quaternion rotation = worldSpaceTransform.GetRotation();

			*outDirection = Math::Rotate(rotation, Math::vec3(0.0f, 0.0f, -1.0f));
		}

		void TransformComponent_GetUpDirection(UUID entityUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

			Math::quaternion rotation = worldSpaceTransform.GetRotation();

			*outDirection = Math::Rotate(rotation, Math::vec3(0.0f, 1.0f, 0.0f));
		}

		void TransformComponent_GetRightDirection(UUID entityUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

			Math::quaternion rotation = worldSpaceTransform.GetRotation();

			*outDirection = Math::Rotate(rotation, Math::vec3(1.0f, 0.0f, 0.0f));
		}

		void TransformComponent_LookAt(UUID entityUUID, Math::vec3* worldPoint)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<RigidBodyComponent>())
			{
				const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

				if (rigidbody.Type == RigidBodyType::Dynamic)
				{
					physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
					physx::PxTransform physxTransform = actor->getGlobalPose();

					const Math::vec3 upDirection(0.0f, 1.0f, 0.0f);
					Math::mat4 result = Math::LookAt(FromPhysXVector(physxTransform.p), *worldPoint, upDirection);
					Math::vec3 translation, scale;
					Math::quaternion rotation;
					Math::vec3 skew;
					Math::vec4 perspective;
					Math::Decompose(Math::Inverse(result), scale, rotation, translation, skew, perspective);
					physxTransform.q = ToPhysXQuat(rotation);

					actor->setGlobalPose(physxTransform);

					return;
				}
			}

			TransformComponent& transform = entity.GetTransform();
			Math::vec3 upDirection(0.0f, 1.0f, 0.0f);
			Math::mat4 result = Math::LookAt(transform.Translation, *worldPoint, upDirection);
			Math::vec3 translation, scale;
			Math::quaternion rotation;
			Math::vec3 skew;
			Math::vec4 perspective;
			Math::Decompose(Math::Inverse(result), scale, rotation, translation, skew, perspective);
			transform.SetRotation(rotation);
		}

		uint64_t TransformComponent_GetParent(UUID entityUUID)
		{
			Entity child = GetEntity(entityUUID);
			Entity parent = GetEntity(child.GetParentUUID());

			if (!parent)
			{
				VX_CONSOLE_LOG_WARN("Invalid Parent UUID!");
				return 0;
			}

			return parent.GetUUID();
		}

		void TransformComponent_SetParent(UUID childUUID, UUID parentUUID)
		{
			Scene* contextScene = GetContextScene();
			Entity child = GetEntity(childUUID);
			Entity parent = GetEntity(parentUUID);
			
			if (!parent || !child)
			{
				return;
			}

			contextScene->ParentEntity(child, parent);
		}

		void TransformComponent_Unparent(UUID entityUUID)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			contextScene->UnparentEntity(entity);
		}

		void TransformComponent_Multiply(TransformComponent* a, TransformComponent* b, TransformComponent* outTransform)
		{
			Math::mat4 transform = a->GetTransform() * b->GetTransform();
			TransformComponent& out = *outTransform;

			Math::vec3 translation, scale;
			Math::quaternion rotation;
			Math::vec3 skew;
			Math::vec4 perspective;
			Math::Decompose(transform, out.Scale, rotation, out.Translation, skew, perspective);
			outTransform->SetRotation(rotation);
		}

#pragma endregion

#pragma region Camera Component

		SceneCamera::ProjectionType CameraComponent_GetProjectionType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.ProjectionType without a Camera!");
				return SceneCamera::ProjectionType::Perspective;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetProjectionType();
		}

		void CameraComponent_SetProjectionType(UUID entityUUID, SceneCamera::ProjectionType type)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.ProjectionType without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			const bool consistentProjectionType = camera.GetProjectionType() == type;

			if (consistentProjectionType)
				return;

			camera.SetProjectionType(type);
		}

		void CameraComponent_GetPrimary(UUID entityUUID, bool* outPrimary)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.IsPrimary without a Camera!");
				return;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();

			*outPrimary = cameraComponent.Primary;
		}

		void CameraComponent_SetPrimary(UUID entityUUID, bool primary)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.IsPrimary without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();

			cameraComponent.Primary = primary;
		}

		float CameraComponent_GetPerspectiveVerticalFOV(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.FieldOfView without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();

			return Math::Rad2Deg(cameraComponent.Camera.GetPerspectiveVerticalFOVRad());
		}

		void CameraComponent_SetPerspectiveVerticalFOV(UUID entityUUID, float perspectiveVerticalFOV)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.FieldOfView without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			const float FOVRad = Math::Deg2Rad(perspectiveVerticalFOV);

			camera.SetPerspectiveVerticalFOVRad(FOVRad);
		}

		float CameraComponent_GetNearClip(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.NearClip without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetPerspectiveNearClip();
		}

		void CameraComponent_SetNearClip(UUID entityUUID, float nearClip)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.NearClip without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetPerspectiveNearClip(nearClip);
		}

		float CameraComponent_GetFarClip(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.FarClip without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetPerspectiveFarClip();
		}

		void CameraComponent_SetFarClip(UUID entityUUID, float farClip)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.FarClip without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetPerspectiveFarClip(farClip);
		}

		float CameraComponent_GetOrthographicSize(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.OrthographicSize without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetOrthographicSize();
		}

		void CameraComponent_SetOrthographicSize(UUID entityUUID, float orthographicSize)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.OrthographicSize without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetOrthographicSize(orthographicSize);
		}

		float CameraComponent_GetOrthographicNear(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.OrthographicNear without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetOrthographicNearClip();
		}

		void CameraComponent_SetOrthographicNear(UUID entityUUID, float orthographicNear)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.OrthographicNear without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetOrthographicNearClip(orthographicNear);
		}

		float CameraComponent_GetOrthographicFar(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.OrthographicFar without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetOrthographicFarClip();
		}

		void CameraComponent_SetOrthographicFar(UUID entityUUID, float orthographicFar)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.OrthographicFar without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetOrthographicFarClip(orthographicFar);
		}

		void CameraComponent_GetFixedAspectRatio(UUID entityUUID, bool* outFixedAspectRatio)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.IsFixedAspectRatio without a Camera!");
				return;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();

			*outFixedAspectRatio = cameraComponent.FixedAspectRatio;
		}

		void CameraComponent_SetFixedAspectRatio(UUID entityUUID, bool fixedAspectRatio)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.IsFixedAspectRatio without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();

			cameraComponent.FixedAspectRatio = fixedAspectRatio;
		}

		void CameraComponent_GetClearColor(UUID entityUUID, Math::vec3* outColor)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Camera.ClearColor without a Camera!");
				return;
			}

			const CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			
			*outColor = cameraComponent.ClearColor;
		}

		void CameraComponent_SetClearColor(UUID entityUUID, Math::vec3* color)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set Camera.ClearColor without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();
			
			cameraComponent.ClearColor = *color;
		}

#pragma endregion

#pragma region Light Source Component

		LightType LightSourceComponent_GetLightType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access LightSource.LightType without a Light Source!");
				return LightType::Directional;
			}

			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			return lightSourceComponent.Type;
		}

		void LightSourceComponent_SetLightType(UUID entityUUID, LightType type)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set LightSource.LightType without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			lightSourceComponent.Type = type;
		}

		void LightSourceComponent_GetRadiance(UUID entityUUID, Math::vec3* outRadiance)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access LightSource.Radiance without a Light Source!");
				return;
			}

			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			*outRadiance = lightSourceComponent.Radiance;
		}

		void LightSourceComponent_SetRadiance(UUID entityUUID, Math::vec3* radiance)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set LightSource.Radiance without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			lightSourceComponent.Radiance = *radiance;
		}

		float LightSourceComponent_GetIntensity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access LightSource.Intensity without a Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			return lightSourceComponent.Intensity;
		}

		void LightSourceComponent_SetIntensity(UUID entityUUID, float intensity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set LightSource.Intensity without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			lightSourceComponent.Intensity = intensity;
		}

		float LightSourceComponent_GetCutoff(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access LightSource.Cutoff without a Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			return lightSourceComponent.Cutoff;
		}

		void LightSourceComponent_SetCutoff(UUID entityUUID, float cutoff)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set LightSource.Cutoff without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			lightSourceComponent.Cutoff = cutoff;
		}

		float LightSourceComponent_GetOuterCutoff(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access LightSource.OuterCutoff without a Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			return lightSourceComponent.OuterCutoff;
		}

		void LightSourceComponent_SetOuterCutoff(UUID entityUUID, float outerCutoff)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set LightSource.OuterCutoff without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			lightSourceComponent.OuterCutoff = outerCutoff;
		}

		float LightSourceComponent_GetShadowBias(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access LightSource.ShadowBias without a Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			return lightSourceComponent.ShadowBias;
		}

		void LightSourceComponent_SetShadowBias(UUID entityUUID, float shadowBias)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set LightSource.ShadowBias without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			lightSourceComponent.ShadowBias = shadowBias;
		}

		bool LightSourceComponent_GetCastShadows(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access LightSource.CastShadows without a Light Source!");
				return false;
			}

			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			return lightSourceComponent.CastShadows;
		}

		void LightSourceComponent_SetCastShadows(UUID entityUUID, bool castShadows)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set LightSource.CastShadows without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			lightSourceComponent.CastShadows = castShadows;
		}

		bool LightSourceComponent_GetSoftShadows(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access LightSource.UseSoftShadows without a Light Source!");
				return false;
			}

			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			return lightSourceComponent.SoftShadows;
		}

		void LightSourceComponent_SetSoftShadows(UUID entityUUID, bool softShadows)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set LightSource.UseSoftShadows without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			lightSourceComponent.SoftShadows = softShadows;
		}

		bool LightSourceComponent_IsVisible(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access LightSource.Visible without a Light Source!");
				return false;
			}

			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			return lightSourceComponent.Visible;
		}

		void LightSourceComponent_SetVisible(UUID entityUUID, bool visible)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set LightSource.Visible without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			lightSourceComponent.Visible = visible;
		}

#pragma endregion

#pragma region TextMesh Component

		MonoString* TextMeshComponent_GetTextString(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access TextMesh.Text without a Text Mesh!");
				return mono_string_new(mono_domain_get(), "");
			}

			const TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			return mono_string_new(mono_domain_get(), textMeshComponent.TextString.c_str());
		}

		void TextMeshComponent_SetTextString(UUID entityUUID, MonoString* textString)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set TextMesh.Text without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			char* textCStr = mono_string_to_utf8(textString);

			textMeshComponent.TextString = std::string(textCStr);
			mono_free(textCStr);
		}

		void TextMeshComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access TextMesh.Color without a Text Mesh!");
				return;
			}

			const TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			*outColor = textMeshComponent.Color;
		}

		void TextMeshComponent_SetColor(UUID entityUUID, Math::vec4* color)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set TextMesh.Color without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			textMeshComponent.Color = *color;
		}

		void TextMeshComponent_GetBackgroundColor(UUID entityUUID, Math::vec4* outBackgroundColor)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access TextMesh.BackgroundColor without a Text Mesh!");
				return;
			}

			const TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			*outBackgroundColor = textMeshComponent.BgColor;
		}

		void TextMeshComponent_SetBackgroundColor(UUID entityUUID, Math::vec4* backgroundcolor)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set TextMesh.BackgroundColor without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			textMeshComponent.BgColor = *backgroundcolor;
		}

		float TextMeshComponent_GetLineSpacing(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access TextMesh.LineSpacing without a Text Mesh!");
				return 0.0f;
			}

			const TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			return textMeshComponent.LineSpacing;
		}

		void TextMeshComponent_SetLineSpacing(UUID entityUUID, float lineSpacing)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set TextMesh.LineSpacing without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			textMeshComponent.LineSpacing = lineSpacing;
		}

		float TextMeshComponent_GetKerning(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access TextMesh.Kerning without a Text Mesh!");
				return 0.0f;
			}

			const TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			return textMeshComponent.Kerning;
		}

		void TextMeshComponent_SetKerning(UUID entityUUID, float kerning)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set TextMesh.Kerning without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			textMeshComponent.Kerning = kerning;
		}

		float TextMeshComponent_GetMaxWidth(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access TextMesh.MaxWidth without a Text Mesh!");
				return 0.0f;
			}

			const TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			return textMeshComponent.MaxWidth;
		}

		void TextMeshComponent_SetMaxWidth(UUID entityUUID, float maxWidth)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set TextMesh.MaxWidth without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			textMeshComponent.MaxWidth = maxWidth;
		}

		bool TextMeshComponent_IsVisible(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access TextMesh.Visible without a Text Mesh!");
				return false;
			}

			const TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			return textMeshComponent.Visible;
		}

		void TextMeshComponent_SetVisible(UUID entityUUID, bool visible)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set TextMesh.Visible without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();
			textMeshComponent.Visible = visible;
		}

#pragma endregion

#pragma region Animator Component

		bool AnimatorComponent_IsPlaying(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AnimatorComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Animator.IsPlaying without a Animator!");
				return false;
			}

			const AnimatorComponent& animatorComponent = entity.GetComponent<AnimatorComponent>();
			SharedRef<Animator> animator = animatorComponent.Animator;
			return animator->IsPlaying();
		}

		void AnimatorComponent_Play(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AnimatorComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling Animator.Play without a Animator!");
				return;
			}

			const AnimatorComponent& animatorComponent = entity.GetComponent<AnimatorComponent>();
			SharedRef<Animator> animator = animatorComponent.Animator;

			if (!entity.HasComponent<AnimationComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling Animator.Play without an Animation!");
				return;
			}

			const AnimationComponent& animationComponent = entity.GetComponent<AnimationComponent>();
			SharedRef<Animation> animation = animationComponent.Animation;

			if (!animation)
			{
				VX_CONSOLE_LOG_ERROR("Calling Animator.Play with Invalid Animator!");
				return;
			}

			animator->PlayAnimation();
		}

		void AnimatorComponent_Stop(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AnimatorComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling Animator.Stop without a Animator!");
				return;
			}

			const AnimatorComponent& animatorComponent = entity.GetComponent<AnimatorComponent>();
			SharedRef<Animator> animator = animatorComponent.Animator;

			if (!animator)
			{
				VX_CONSOLE_LOG_ERROR("Calling Animator.Stop with Invalid Animator!");
				return;
			}

			animator->Stop();
		}

#pragma endregion

#pragma region Mesh Renderer Component

		bool MeshRendererComponent_GetMaterialHandle(uint32_t submeshIndex, UUID entityUUID, AssetHandle* outHandle)
		{
			VX_CORE_ASSERT(false, "Not implemented yet!");
			return false;
		}

		bool MeshRendererComponent_IsVisible(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<MeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access MeshRenderer.Visible without a Mesh Renderer!");
				return false;
			}

			const MeshRendererComponent& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
			return meshRendererComponent.Visible;
		}

		void MeshRendererComponent_SetVisible(UUID entityUUID, bool visible)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<MeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set MeshRenderer.Visible without a Mesh Renderer!");
				return;
			}

			MeshRendererComponent& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
			meshRendererComponent.Visible = visible;
		}

#pragma endregion

#pragma region Static Mesh Renderer Component

		MeshType StaticMeshRendererComponent_GetMeshType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access StaticMeshRenderer.MeshType without a Static Mesh Renderer!");
				return MeshType::Cube;
			}

			const StaticMeshRendererComponent& staticMeshRenderer = entity.GetComponent<StaticMeshRendererComponent>();

			return staticMeshRenderer.Type;
		}

		void StaticMeshRendererComponent_SetMeshType(UUID entityUUID, MeshType meshType)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set StaticMeshRenderer.MeshType without a Static Mesh Renderer!");
				return;
			}

			StaticMeshRendererComponent& staticMeshRenderer = entity.GetComponent<StaticMeshRendererComponent>();
			staticMeshRenderer.Type = meshType;

			staticMeshRenderer.StaticMesh = DefaultMesh::DefaultStaticMeshes[(uint32_t)meshType];

			if (AssetManager::IsHandleValid(staticMeshRenderer.StaticMesh) && staticMeshRenderer.Materials->Empty())
			{
				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshRenderer.StaticMesh);
				staticMesh->LoadMaterialTable(staticMeshRenderer.Materials);
			}
		}

		void StaticMeshRendererComponent_SetMaterialHandle(uint32_t submeshIndex, UUID entityUUID, AssetHandle* materialHandle)
		{
			if (!AssetManager::IsHandleValid(*materialHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set material handle with invalid material!");
				return;
			}

			Entity entity = GetEntity(entityUUID);
			StaticMeshRendererComponent& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();
			if (!AssetManager::IsHandleValid(staticMeshRendererComponent.StaticMesh))
			{
				VX_CONSOLE_LOG_ERROR("Cannot set material of invalid mesh asset!");
				return;
			}

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshRendererComponent.StaticMesh);
			if (!staticMesh)
			{
				VX_CONSOLE_LOG_ERROR("Cannot set material of invalid mesh asset!");
			}

			if (!staticMesh->HasSubmesh(submeshIndex))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set submesh material with out of bounds index!");
				return;
			}

			SharedReference<MaterialTable> materialTable = staticMeshRendererComponent.Materials;

			materialTable->SetMaterial(submeshIndex, *materialHandle);
		}

		bool StaticMeshRendererComponent_IsVisible(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);
			
			if (!entity.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access StaticMeshRenderer.Visible without a Static Mesh Renderer!");
				return false;
			}

			const StaticMeshRendererComponent& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();
			return staticMeshRendererComponent.Visible;
		}

		void StaticMeshRendererComponent_SetVisible(UUID entityUUID, bool visible)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set StaticMeshRenderer.Visible without a Static Mesh Renderer!");
				return;
			}

			StaticMeshRendererComponent& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();
			staticMeshRendererComponent.Visible = visible;
		}

		bool StaticMeshRendererComponent_GetMaterialHandle(uint32_t submeshIndex, UUID entityUUID, AssetHandle* outHandle)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling StaticMeshRenderer.GetMaterialHandle without a Static Mesh Renderer!");
				return false;
			}

			const StaticMeshRendererComponent& staticMeshRenderer = entity.GetComponent<StaticMeshRendererComponent>();
			SharedReference<MaterialTable> materialTable = staticMeshRenderer.Materials;
			VX_CORE_ASSERT(!materialTable->Empty(), "Material table not synchronized with component!");
			VX_CORE_ASSERT(materialTable->HasMaterial(submeshIndex), "Index out of bounds!");

			if (!materialTable->HasMaterial(submeshIndex))
				return false;

			*outHandle = materialTable->GetMaterial(submeshIndex);
			return true;
		}

#pragma endregion

#pragma region Material

		void Material_GetAlbedo(AssetHandle* assetHandle, Math::vec3* outAlbedo)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			*outAlbedo = material->GetAlbedo();
		}

		void Material_SetAlbedo(AssetHandle* assetHandle, Math::vec3* albedo)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			material->SetAlbedo(*albedo);
		}

		bool Material_GetAlbedoMap(AssetHandle* assetHandle, AssetHandle* outHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return false;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return false;
			}

			*outHandle = material->GetAlbedoMap();
			return true;
		}

		void Material_SetAlbedoMap(AssetHandle* assetHandle, AssetHandle* textureHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			if (!AssetManager::IsHandleValid(*textureHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			material->SetAlbedoMap(*textureHandle);
		}

		bool Material_GetNormalMap(AssetHandle* assetHandle, AssetHandle* outHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return false;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return false;
			}

			*outHandle = material->GetNormalMap();
			return true;
		}

		void Material_SetNormalMap(AssetHandle* assetHandle, AssetHandle* textureHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			if (!AssetManager::IsHandleValid(*textureHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			material->SetNormalMap(*textureHandle);
		}

		float Material_GetMetallic(AssetHandle* assetHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return 0.0f;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return 0.0f;
			}

			return material->GetMetallic();
		}

		void Material_SetMetallic(AssetHandle* assetHandle, float metallic)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			material->SetMetallic(metallic);
		}

		bool Material_GetMetallicMap(AssetHandle* assetHandle, AssetHandle* outHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return false;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return false;
			}

			*outHandle = material->GetMetallicMap();
			return true;
		}

		void Material_SetMetallicMap(AssetHandle* assetHandle, AssetHandle* textureHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			if (!AssetManager::IsHandleValid(*textureHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			material->SetMetallicMap(*textureHandle);
		}

		float Material_GetRoughness(AssetHandle* assetHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return 0.0f;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return 0.0f;
			}

			return material->GetRoughness();
		}

		void Material_SetRoughness(AssetHandle* assetHandle, float roughness)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			material->SetRoughness(roughness);
		}

		bool Material_GetRoughnessMap(AssetHandle* assetHandle, AssetHandle* outHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return false;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return false;
			}

			*outHandle = material->GetRoughnessMap();
			return true;
		}

		void Material_SetRoughnessMap(AssetHandle* assetHandle, AssetHandle* textureHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			if (!AssetManager::IsHandleValid(*textureHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			material->SetRoughnessMap(*textureHandle);
		}

		float Material_GetEmission(AssetHandle* assetHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return 0.0f;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return 0.0f;
			}

			return material->GetEmission();
		}

		void Material_SetEmission(AssetHandle* assetHandle, float emission)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			material->SetEmission(emission);
		}

		bool Material_GetEmissionMap(AssetHandle* assetHandle, AssetHandle* outHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return false;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return false;
			}

			*outHandle = material->GetEmissionMap();
			return true;
		}

		void Material_SetEmissionMap(AssetHandle* assetHandle, AssetHandle* textureHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			if (!AssetManager::IsHandleValid(*textureHandle))
			{
				VX_CORE_ASSERT(false, "Invlaid Asset Handle!");
				return;
			}

			material->SetEmissionMap(*textureHandle);
		}

		bool Material_GetAmbientOcclusionMap(AssetHandle* assetHandle, AssetHandle* outHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return false;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return false;
			}

			*outHandle = material->GetAmbientOcclusionMap();
			return true;
		}

		void Material_SetAmbientOcclusionMap(AssetHandle* assetHandle, AssetHandle* textureHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			if (!AssetManager::IsHandleValid(*textureHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			material->SetAmbientOcclusionMap(*textureHandle);
		}

		void Material_GetUV(AssetHandle* assetHandle, Math::vec2* outUV)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			*outUV = material->GetUV();
		}

		void Material_SetUV(AssetHandle* assetHandle, Math::vec2* uv)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			material->SetUV(*uv);
		}

		float Material_GetOpacity(AssetHandle* assetHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return 0.0f;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return 0.0f;
			}

			return material->GetOpacity();
		}

		void Material_SetOpacity(AssetHandle* assetHandle, float opacity)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			material->SetOpacity(opacity);
		}

		bool Material_IsFlagSet(AssetHandle* assetHandle, MaterialFlag flag)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return false;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return false;
			}

			return material->HasFlag(flag);
		}

		void Material_SetFlag(AssetHandle* assetHandle, MaterialFlag flag, bool value)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CORE_ASSERT(false, "Invalid Asset Handle!");
				return;
			}

			SharedReference<Material> material = AssetManager::GetAsset<Material>(*assetHandle);
			if (!material)
			{
				VX_CORE_ASSERT(false, "Invalid Material Asset!");
				return;
			}

			if (value)
				material->RemoveFlag(flag);
			else
				material->SetFlag(flag);
		}

#pragma endregion

#pragma region Sprite Renderer Component

        bool SpriteRendererComponent_GetTextureHandle(UUID entityUUID, AssetHandle* outHandle)
        {
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SpriteRenderer.Texture without a Sprite Renderer!");
				return false;
			}

			const SpriteRendererComponent& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

			if (AssetManager::IsHandleValid(spriteRenderer.Texture))
			{
				*outHandle = spriteRenderer.Texture;
			}

			return true;
        }

        void SpriteRendererComponent_SetTextureHandle(UUID entityUUID, AssetHandle* textureHandle)
        {
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SpriteRenderer.Texture without a Sprite Renderer!");
				return;
			}

			SpriteRendererComponent& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

			if (AssetManager::IsHandleValid(*textureHandle))
			{
				spriteRenderer.Texture = *textureHandle;
			}
        }

        void SpriteRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SpriteRenderer.Color without a Sprite Renderer!");
				return;
			}

			const SpriteRendererComponent& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
			*outColor = spriteRenderer.SpriteColor;
		}

		void SpriteRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SpriteRenderer.Color without a Sprite Renderer!");
				return;
			}

			SpriteRendererComponent& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
			spriteRenderer.SpriteColor = *color;
		}

		void SpriteRendererComponent_GetUV(UUID entityUUID, Math::vec2* outScale)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SpriteRenderer.Scale without a Sprite Renderer!");
				return;
			}

			const SpriteRendererComponent& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
			*outScale = spriteRenderer.TextureUV;
		}

		void SpriteRendererComponent_SetUV(UUID entityUUID, Math::vec2* scale)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SpriteRenderer.Scale without a Sprite Renderer!");
				return;
			}

			SpriteRendererComponent& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
			spriteRenderer.TextureUV = *scale;
		}

		bool SpriteRendererComponent_IsVisible(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SpriteRenderer.Visible without a Sprite Renderer!");
				return false;
			}

			const SpriteRendererComponent& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			return spriteRendererComponent.Visible;
		}

		void SpriteRendererComponent_SetVisible(UUID entityUUID, bool visible)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SpriteRenderer.Visible without a Sprite Renderer!");
				return;
			}

			SpriteRendererComponent& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			spriteRendererComponent.Visible = visible;
		}

#pragma endregion

#pragma region Circle Renderer Component

		void CircleRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleRenderer.Color without a Circle Renderer!");
				return;
			}

			const CircleRendererComponent& circleRenderer = entity.GetComponent<CircleRendererComponent>();
			*outColor = circleRenderer.Color;
		}

		void CircleRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleRenderer.Color without a Circle Renderer!");
				return;
			}

			CircleRendererComponent& circleRenderer = entity.GetComponent<CircleRendererComponent>();
			circleRenderer.Color = *color;
		}

		void CircleRendererComponent_GetThickness(UUID entityUUID, float* outThickness)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleRenderer.Thickness without a Circle Renderer!");
				return;
			}

			const CircleRendererComponent& circleRenderer = entity.GetComponent<CircleRendererComponent>();
			*outThickness = circleRenderer.Thickness;
		}

		void CircleRendererComponent_SetThickness(UUID entityUUID, float thickness)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleRenderer.Thickness without a Circle Renderer!");
				return;
			}

			CircleRendererComponent& circleRenderer = entity.GetComponent<CircleRendererComponent>();
			circleRenderer.Thickness = thickness;
		}

		void CircleRendererComponent_GetFade(UUID entityUUID, float* outFade)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleRenderer.Fade without a Circle Renderer!");
				return;
			}

			const CircleRendererComponent& circleRenderer = entity.GetComponent<CircleRendererComponent>();
			*outFade = circleRenderer.Fade;
		}

		void CircleRendererComponent_SetFade(UUID entityUUID, float fade)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleRenderer.Fade without a Circle Renderer!");
				return;
			}

			CircleRendererComponent& circleRenderer = entity.GetComponent<CircleRendererComponent>();
			circleRenderer.Fade = fade;
		}

		bool CircleRendererComponent_IsVisible(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleRenderer.Visible without a Circle Renderer!");
				return false;
			}

			const CircleRendererComponent& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
			return circleRendererComponent.Visible;
		}

		void CircleRendererComponent_SetVisible(UUID entityUUID, bool visible)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleRenderer.Visible without a Circle Renderer!");
				return;
			}

			CircleRendererComponent& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
			circleRendererComponent.Visible = visible;
		}

#pragma endregion

#pragma region Particle Emitter Component

		void ParticleEmitterComponent_GetVelocity(UUID entityUUID, Math::vec3* outVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.Velocity without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}
			
			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outVelocity = particleEmitter->GetProperties().Velocity;
		}

		void ParticleEmitterComponent_SetVelocity(UUID entityUUID, Math::vec3* velocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.Velocity without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}
			
			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().Velocity = *velocity;
		}

		void ParticleEmitterComponent_GetVelocityVariation(UUID entityUUID, Math::vec3* outVelocityVariation)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.VelocityVariation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.VelocityVariation with invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outVelocityVariation = particleEmitter->GetProperties().VelocityVariation;
		}

		void ParticleEmitterComponent_SetVelocityVariation(UUID entityUUID, Math::vec3* velocityVariation)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.VelocityVariation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().VelocityVariation = *velocityVariation;
		}

		void ParticleEmitterComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.Offset without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outOffset = particleEmitter->GetProperties().Offset;
		}

		void ParticleEmitterComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.Offset without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().Offset = *offset;
		}

		void ParticleEmitterComponent_GetSizeBegin(UUID entityUUID, Math::vec2* outSizeBegin)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.SizeBegin without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.SizeBegin with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outSizeBegin = particleEmitter->GetProperties().SizeBegin;
		}

		void ParticleEmitterComponent_SetSizeBegin(UUID entityUUID, Math::vec2* sizeBegin)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.SizeBegin without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.SizeBegin with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().SizeBegin = *sizeBegin;
		}

		void ParticleEmitterComponent_GetSizeEnd(UUID entityUUID, Math::vec2* outSizeEnd)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.SizeEnd without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.SizeEnd with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outSizeEnd = particleEmitter->GetProperties().SizeEnd;
		}

		void ParticleEmitterComponent_SetSizeEnd(UUID entityUUID, Math::vec2* sizeEnd)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.SizeEnd without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.SizeEnd with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().SizeEnd = *sizeEnd;
		}

		void ParticleEmitterComponent_GetSizeVariation(UUID entityUUID, Math::vec2* outSizeVariation)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.SizeVariation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.SizeVariation with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outSizeVariation = particleEmitter->GetProperties().SizeVariation;
		}

		void ParticleEmitterComponent_SetSizeVariation(UUID entityUUID, Math::vec2* sizeVariation)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.SizeVariation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.SizeVariation with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().SizeVariation = *sizeVariation;
		}

		void ParticleEmitterComponent_GetColorBegin(UUID entityUUID, Math::vec4* outColorBegin)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.ColorBegin without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.ColorBegin with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outColorBegin = particleEmitter->GetProperties().ColorBegin;
		}

		void ParticleEmitterComponent_SetColorBegin(UUID entityUUID, Math::vec4* colorBegin)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.ColorBegin without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.ColorBegin with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().ColorBegin = *colorBegin;
		}

		void ParticleEmitterComponent_GetColorEnd(UUID entityUUID, Math::vec4* outColorEnd)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.ColorEnd without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.ColorEnd with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outColorEnd = particleEmitter->GetProperties().ColorEnd;
		}

		void ParticleEmitterComponent_SetColorEnd(UUID entityUUID, Math::vec4* colorEnd)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.ColorEnd without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.ColorEnd with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().ColorEnd = *colorEnd;
		}

		void ParticleEmitterComponent_GetRotation(UUID entityUUID, float* outRotation)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.Rotation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.Rotation with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outRotation = particleEmitter->GetProperties().Rotation;
		}

		void ParticleEmitterComponent_SetRotation(UUID entityUUID, float colorEnd)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.Rotation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.Rotation with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().Rotation = colorEnd;
		}

		void ParticleEmitterComponent_GetLifeTime(UUID entityUUID, float* outLifeTime)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.LifeTime without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access ParticleEmitter.LifeTime with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			*outLifeTime = particleEmitter->GetProperties().LifeTime;
		}

		void ParticleEmitterComponent_SetLifeTime(UUID entityUUID, float lifetime)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.LifeTime without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set ParticleEmitter.LifeTime with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().LifeTime = lifetime;
		}

		void ParticleEmitterComponent_Start(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling ParticleEmitter.Start without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling ParticleEmitter.Start with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->Start();
		}

		void ParticleEmitterComponent_Stop(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling ParticleEmitter.Stop without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(particleEmitterComponent.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling ParticleEmitter.Stop with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(particleEmitterComponent.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->Stop();
		}

#pragma endregion

#pragma region AudioSource Component

		void AudioSourceComponent_GetPosition(UUID entityUUID, Math::vec3* outPosition)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Position without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Position with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			*outPosition = Utils::FromWaveVector(audioSource->GetPlaybackDevice().GetSound().GetPosition());
		}

		void AudioSourceComponent_SetPosition(UUID entityUUID, Math::vec3* position)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Position without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Position with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetPosition(Utils::ToWaveVector(*position));
		}

		void AudioSourceComponent_GetDirection(UUID entityUUID, Math::vec3* outDirection)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Direction without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Direction with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			*outDirection = Utils::FromWaveVector(audioSource->GetPlaybackDevice().GetSound().GetDirection());
		}

		void AudioSourceComponent_SetDirection(UUID entityUUID, Math::vec3* direction)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Direction without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Direction with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetDirection(Utils::ToWaveVector(*direction));
		}

		void AudioSourceComponent_GetVelocity(UUID entityUUID, Math::vec3* outVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Velocity without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			*outVelocity = Utils::FromWaveVector(audioSource->GetPlaybackDevice().GetSound().GetVelocity());
		}

		void AudioSourceComponent_SetVelocity(UUID entityUUID, Math::vec3* velocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Velocity without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetVelocity(Utils::ToWaveVector(*velocity));
		}

        float AudioSourceComponent_GetMinGain(UUID entityUUID)
        {
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MinGain without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MinGain with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetMinGain();
        }

        void AudioSourceComponent_SetMinGain(UUID entityUUID, float minGain)
        {
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MinGain without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MinGain with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetMinGain(minGain);
        }

        float AudioSourceComponent_GetMaxGain(UUID entityUUID)
        {
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MaxGain without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MaxGain with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetMaxGain();
        }

        void AudioSourceComponent_SetMaxGain(UUID entityUUID, float maxGain)
        {
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MaxGain without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MaxGain with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetMaxGain(maxGain);
        }

        AttenuationModel AudioSourceComponent_GetAttenuationModel(UUID entityUUID)
        {
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Attenuation without a Audio Source!");
				return AttenuationModel::None;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Attenuation with an invalid asset handle!");
				return AttenuationModel::None;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return AttenuationModel::None;

			return Utils::FromWaveAttenuationModel(audioSource->GetPlaybackDevice().GetSound().GetAttenuationModel());
		}

		void AudioSourceComponent_SetAttenuationModel(UUID entityUUID, AttenuationModel model)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Attenuation without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Attenuation with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetAttenuationModel(Utils::ToWaveAttenuationModel(model));
        }

        float AudioSourceComponent_GetFalloff(UUID entityUUID)
        {
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Falloff without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Falloff with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetFalloff();
		}

		void AudioSourceComponent_SetFalloff(UUID entityUUID, float falloff)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Falloff without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Falloff with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetFalloff(falloff);
        }

		float AudioSourceComponent_GetMinDistance(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MinDistance without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MinDistance with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetMinDistance();
		}

		void AudioSourceComponent_SetMinDistance(UUID entityUUID, float minDistance)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MinDistance without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MinDistance with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetMinDistance(minDistance);
		}

		float AudioSourceComponent_GetMaxDistance(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MaxDistance without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MaxDistance with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetMaxDistance();
		}

		void AudioSourceComponent_SetMaxDistance(UUID entityUUID, float maxDistance)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MaxDistance without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MaxDistance with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetMaxDistance(maxDistance);
		}

		float AudioSourceComponent_GetPitch(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Pitch without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Pitch with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetPitch();
		}

		void AudioSourceComponent_SetPitch(UUID entityUUID, float pitch)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Pitch without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Pitch with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetPitch(pitch);
		}

		float AudioSourceComponent_GetDopplerFactor(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.DopplerFactor without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.DopplerFactor with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetDopplerFactor();
		}

		void AudioSourceComponent_SetDopplerFactor(UUID entityUUID, float dopplerFactor)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.DopplerFactor without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.DopplerFactor with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetDopplerFactor(dopplerFactor);
		}

		float AudioSourceComponent_GetVolume(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Volume without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Volume with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetVolume();
		}

		void AudioSourceComponent_SetVolume(UUID entityUUID, float volume)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Volume without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Volume with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetVolume(volume);
		}

		bool AudioSourceComponent_GetPlayOnStart(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PlayOnStart without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PlayOnStart with an invalid asset handle!");
				return false;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return false;

			// TODO fix this once we have PlayOnStart
			//return audioSource->GetPlaybackDevice().GetSound().PlayOnStart;
			return false;
		}

		void AudioSourceComponent_SetPlayOnStart(UUID entityUUID, bool playOnStart)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.PlayOnStart without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.PlayOnStart with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			// TODO: Ditto
			// audioSource->SetPlayOnStart(playOnStart);
		}

		bool AudioSourceComponent_GetIsSpacialized(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsSpacialized without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsSpacialized with an invalid asset handle!");
				return false;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return false;

			return audioSource->GetPlaybackDevice().GetSound().IsSpacialized();
		}

		void AudioSourceComponent_SetIsSpacialized(UUID entityUUID, bool spacialized)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.IsSpacialized without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.IsSpacialized with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetSpacialized(spacialized);
		}

		bool AudioSourceComponent_GetIsLooping(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsLooping without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsLooping with an invalid asset handle!");
				return false;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return false;

			return audioSource->GetPlaybackDevice().GetSound().IsLooping();
		}

		void AudioSourceComponent_SetIsLooping(UUID entityUUID, bool loop)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.IsLooping without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.IsLooping with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetLooping(loop);
		}

		bool AudioSourceComponent_GetIsPlaying(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsPlaying without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsPlaying with an invalid asset handle!");
				return false;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return false;

			return audioSource->GetPlaybackDevice().GetSound().IsPlaying();
		}

		void AudioSourceComponent_Play(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Play without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Play with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().Play();
		}

		void AudioSourceComponent_PlayOneShot(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.PlayOneShot without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.PlayOneShot with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			// TODO: fix once Wave has PlayOneShot
			//audioSource->PlayOneShot();
		}

		void AudioSourceComponent_Restart(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Restart without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Restart with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().Restart();
		}

		void AudioSourceComponent_Stop(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Stop without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Stop with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().Stop();
		}

#pragma endregion

#pragma region Audio Clip

		MonoString* AudioClip_GetName(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioClip.Name without a Audio Source!");
				return mono_string_new(mono_domain_get(), "");
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioClip.Name with an invalid asset handle!");
				return mono_string_new(mono_domain_get(), "");
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return mono_string_new(mono_domain_get(), "");

			/*const AudioClip& audioClip = audioSource->GetAudioClip();
			std::string clipName = audioClip.Name;

			return mono_string_new(mono_domain_get(), clipName.c_str());*/
		}

		float AudioClip_GetLength(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioClip.Length without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioClip.Length with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			audioSource->GetPlaybackDevice().GetSound().GetLengthInSeconds();
		}

#pragma endregion

#pragma region Audio Cone

		float AudioCone_GetInnerAngle(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioCone.InnerAngle without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioCone.InnerAngle with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetAudioCone().InnerAngle;
		}

		void AudioCone_SetInnerAngle(UUID entityUUID, float innerAngle)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioCone.InnerAngle without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioCone.InnerAngle with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			Wave::AudioCone cone = audioSource->GetPlaybackDevice().GetSound().GetAudioCone();
			cone.InnerAngle = innerAngle;
			audioSource->GetPlaybackDevice().GetSound().SetAudioCone(cone);
		}

		float AudioCone_GetOuterAngle(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioCone.OuterAngle without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioCone.OuterAngle with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetAudioCone().OuterAngle;
		}

		void AudioCone_SetOuterAngle(UUID entityUUID, float outerAngle)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioCone.OuterAngle without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioCone.OuterAngle with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			Wave::AudioCone cone = audioSource->GetPlaybackDevice().GetSound().GetAudioCone();
			cone.OuterAngle = outerAngle;
			audioSource->GetPlaybackDevice().GetSound().SetAudioCone(cone);
		}

		float AudioCone_GetOuterGain(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioCone.OuterGain without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioCone.OuterGain with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetAudioCone().OuterGain;
		}

		void AudioCone_SetOuterGain(UUID entityUUID, float outerGain)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioCone.OuterGain without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioCone.OuterGain with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			Wave::AudioCone cone = audioSource->GetPlaybackDevice().GetSound().GetAudioCone();
			cone.OuterGain = outerGain;
			audioSource->GetPlaybackDevice().GetSound().SetAudioCone(cone);
		}

#pragma endregion

#pragma region RigidBody Component

		RigidBodyType RigidBodyComponent_GetBodyType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return RigidBodyType::Static;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			return rigidbody.Type;
		}

		void RigidBodyComponent_SetBodyType(UUID entityUUID, RigidBodyType bodyType)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			const bool consistentBodyType = bodyType == rigidbody.Type;

			if (consistentBodyType)
				return;

			rigidbody.Type = bodyType;
			Physics::ReCreateActor(entity);
		}

		CollisionDetectionType RigidBodyComponent_GetCollisionDetectionType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return CollisionDetectionType::Discrete;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.CollisionDetection with a non-dynamic Rigidbody!");
				return CollisionDetectionType::Discrete;
			}

			return rigidbody.CollisionDetection;
		}

		void RigidBodyComponent_SetCollisionDetectionType(UUID entityUUID, CollisionDetectionType collisionDetectionType)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.CollisionDetectiom with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.CollisionDetection = collisionDetectionType;
		}

		float RigidBodyComponent_GetMass(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.Mass with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.Mass;
		}

		void RigidBodyComponent_SetMass(UUID entityUUID, float mass)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.Mass with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.Mass = mass;
		}

		void RigidBodyComponent_GetLinearVelocity(UUID entityUUID, Math::vec3* outVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.LinearVelocity with a non-dynamic Rigidbody!");
				return;
			}

			*outVelocity = rigidbody.LinearVelocity;
		}

		void RigidBodyComponent_SetLinearVelocity(UUID entityUUID, Math::vec3* velocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.LinearVelocity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.LinearVelocity = *velocity;
		}

		float RigidBodyComponent_GetMaxLinearVelocity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.MaxLinearVelocity with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.MaxLinearVelocity;
		}

		void RigidBodyComponent_SetMaxLinearVelocity(UUID entityUUID, float maxLinearVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.MaxLinearVelocity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.MaxLinearVelocity = maxLinearVelocity;
		}

		float RigidBodyComponent_GetLinearDrag(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.LinearDrag with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.LinearDrag;
		}

		void RigidBodyComponent_SetLinearDrag(UUID entityUUID, float drag)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.LinearDrag with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.LinearDrag = drag;
		}

		void RigidBodyComponent_GetAngularVelocity(UUID entityUUID, Math::vec3* outVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.AngularVelocity with a non-dynamic Rigidbody!");
				return;
			}

			*outVelocity = rigidbody.AngularVelocity;
		}

		void RigidBodyComponent_SetAngularVelocity(UUID entityUUID, Math::vec3* velocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.AngularVelocity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.AngularVelocity = *velocity;
		}

		float RigidBodyComponent_GetMaxAngularVelocity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.MaxAngularVelocity with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.MaxAngularVelocity;
		}

		void RigidBodyComponent_SetMaxAngularVelocity(UUID entityUUID, float maxAngularVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.MaxAngularVelocity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.MaxAngularVelocity = maxAngularVelocity;
		}

		float RigidBodyComponent_GetAngularDrag(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.AngularDrag with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.AngularDrag;
		}

		void RigidBodyComponent_SetAngularDrag(UUID entityUUID, float drag)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.AngluarDrag with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.AngularDrag = drag;
		}

		bool RigidBodyComponent_GetDisableGravity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return false;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.DisableGravity with a non-dynamic Rigidbody!");
				return false;
			}

			return rigidbody.DisableGravity;
		}

		void RigidBodyComponent_SetDisableGravity(UUID entityUUID, bool disabled)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.DisableGravity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.DisableGravity = disabled;
		}

		bool RigidBodyComponent_GetIsKinematic(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return false;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.IsKinematic with a non-dynamic Rigidbody!");
				return false;
			}

			return entity.GetComponent<RigidBodyComponent>().IsKinematic;
		}

		void RigidBodyComponent_SetIsKinematic(UUID entityUUID, bool isKinematic)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.IsKinematic with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.IsKinematic = isKinematic;
		}

		void RigidBodyComponent_GetKinematicTargetTranslation(UUID entityUUID, Math::vec3* outTranslation)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody.KinematicTarget without a Kinematic RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (!rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget with a non-kinematic RigidBody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			physx::PxTransform target;

			if (actor->getKinematicTarget(target))
			{
				*outTranslation = FromPhysXVector(target.p);
			}

			*outTranslation = Math::vec3(0.0f);
		}

		void RigidBodyComponent_SetKinematicTargetTranslation(UUID entityUUID, Math::vec3* translation)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget without a Kinematic RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (!rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget with a non-kinematic RigidBody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			physx::PxTransform targetTransform;
			targetTransform.p = ToPhysXVector(*translation);

			physx::PxTransform t;

			if (actor->getKinematicTarget(t))
			{
				targetTransform.q = t.q;
			}

			actor->setKinematicTarget(targetTransform);
		}

		void RigidBodyComponent_GetKinematicTargetRotation(UUID entityUUID, Math::quaternion* outRotation)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody.KinematicTarget without a Kinematic RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (!rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget with a non-kinematic RigidBody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			physx::PxTransform target;

			if (actor->getKinematicTarget(target))
			{
				*outRotation = FromPhysXQuat(target.q);
			}

			*outRotation = Math::quaternion(1, 0, 0, 0);
		}

		void RigidBodyComponent_SetKinematicTargetRotation(UUID entityUUID, Math::quaternion* rotation)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget without a Kinematic RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (!rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget with a non-kinematic RigidBody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			physx::PxTransform targetTransform;
			targetTransform.q = ToPhysXQuat(*rotation);

			physx::PxTransform t;

			if (actor->getKinematicTarget(t))
			{
				targetTransform.p = t.p;
			}

			actor->setKinematicTarget(targetTransform);
		}

		uint32_t RigidBodyComponent_GetLockFlags(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return 0;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.LockFlags with a non-dynamic Rigidbody!");
				return 0;
			}

			return (uint32_t)rigidbody.LockFlags;
		}

		void RigidBodyComponent_SetLockFlag(UUID entityUUID, ActorLockFlag flag, bool value, bool forceWake)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.LockFlags with a non-dynamic Rigidbody!");
				return;
			}

			if (value)
			{
				rigidbody.LockFlags |= (uint8_t)flag;
			}
			else
			{
				rigidbody.LockFlags &= ~(uint8_t)flag;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();

			actor->setRigidDynamicLockFlag((physx::PxRigidDynamicLockFlag::Enum)flag, value);

			if (forceWake)
			{
				actor->wakeUp();
			}
		}

		bool RigidBodyComponent_IsLockFlagSet(UUID entityUUID, ActorLockFlag flag)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return false;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.LockFlag with a non-dynamic Rigidbody!");
				return false;
			}

			return rigidbody.LockFlags & (uint8_t)flag;
		}

		bool RigidBodyComponent_IsSleeping(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return false;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.IsSleeping with a non-dynamic Rigidbody!");
				return false;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			return actor->isSleeping();
		}

		void RigidBodyComponent_WakeUp(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling RigidBody.Wakeup with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			actor->wakeUp();
		}

		void RigidBodyComponent_AddForce(UUID entityUUID, Math::vec3* force, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling Rigidbody.AddForce with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			actor->addForce(ToPhysXVector(*force), (physx::PxForceMode::Enum)mode);
		}

		void RigidBodyComponent_AddForceAtPosition(UUID entityUUID, Math::vec3* force, Math::vec3* position, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity)
			{
				return;
			}

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling Rigidbody.AddForceAtPosition with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			physx::PxRigidBodyExt::addForceAtPos(*actor, ToPhysXVector(*force), ToPhysXVector(*position), (physx::PxForceMode::Enum)mode);
		}

		void RigidBodyComponent_AddTorque(UUID entityUUID, Math::vec3* torque, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling Rigidbody.AddTorque with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			actor->addTorque(ToPhysXVector(*torque), (physx::PxForceMode::Enum)mode);
		}

		void RigidBodyComponent_ClearTorque(UUID entityUUID, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CORE_ASSERT(false, "Calling RigidBody.ClearTorque with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			actor->clearTorque((physx::PxForceMode::Enum)mode);
		}

		void RigidBodyComponent_ClearForce(UUID entityUUID, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling RigidBody.ClearForce with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			actor->clearForce((physx::PxForceMode::Enum)mode);
		}

#pragma endregion

#pragma region Physics

		bool Physics_Raycast(Math::vec3* origin, Math::vec3* direction, float maxDistance, RaycastHit* outHit)
		{
			Scene* contextScene = GetContextScene();

			return Physics::Raycast(*origin, *direction, maxDistance, outHit);
		}

		void Physics_GetSceneGravity(Math::vec3* outGravity)
		{
			Scene* contextScene = GetContextScene();

			*outGravity = Physics::GetPhysicsSceneGravity();
		}

		void Physics_SetSceneGravity(Math::vec3* gravity)
		{
			Scene* contextScene = GetContextScene();

			Physics::SetPhysicsSceneGravity(*gravity);
			Physics::WakeUpActors();
		}

		uint32_t Physics_GetScenePositionIterations()
		{
			Scene* contextScene = GetContextScene();

			return Physics::GetPhysicsScenePositionIterations();
		}

		void Physics_SetScenePositionIterations(uint32_t positionIterations)
		{
			Scene* contextScene = GetContextScene();

			Physics::SetPhysicsScenePositionIterations(positionIterations);
		}

		uint32_t Physics_GetSceneVelocityIterations()
		{
			Scene* contextScene = GetContextScene();

			return Physics::GetPhysicsSceneVelocityIterations();
		}

		void Physics_SetSceneVelocityIterations(uint32_t velocityIterations)
		{
			Scene* contextScene = GetContextScene();

			Physics::SetPhysicsSceneVelocityIterations(velocityIterations);
		}

#pragma endregion

#pragma region PhysicsMaterial

		float PhysicsMaterial_GetStaticFriction(AssetHandle* assetHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.StaticFriction with invalid asset handle!");
				return 0.0f;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);
			
			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.StaticFriction with invalid asset handle!");
				return 0.0f;
			}

			return physicsMaterial->StaticFriction;
		}

		void PhysicsMaterial_SetStaticFriction(AssetHandle* assetHandle, float staticFriction)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.StaticFriction with invalid asset handle!");
				return;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);

			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.StaticFriction with invalid asset handle!");
				return;
			}

			physicsMaterial->StaticFriction = staticFriction;
		}

		float PhysicsMaterial_GetDynamicFriction(AssetHandle* assetHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.DynamicFriction with invalid asset handle!");
				return 0.0f;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);

			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.DynamicFriction with invalid asset handle!");
				return 0.0f;
			}

			return physicsMaterial->DynamicFriction;
		}

		void PhysicsMaterial_SetDynamicFriction(AssetHandle* assetHandle, float dynamicFriction)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.DynamicFriction with invalid asset handle!");
				return;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);

			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.DynamicFriction with invalid asset handle!");
				return;
			}

			physicsMaterial->DynamicFriction = dynamicFriction;
		}

		float PhysicsMaterial_GetBounciness(AssetHandle* assetHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.BouncinessDynamicFriction with invalid asset handle!");
				return 0.0f;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);

			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.BouncinessDynamicFriction with invalid asset handle!");
				return 0.0f;
			}

			return physicsMaterial->Bounciness;
		}

		void PhysicsMaterial_SetBounciness(AssetHandle* assetHandle, float bounciness)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.BouncinessDynamicFriction with invalid asset handle!");
				return;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);

			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.BouncinessDynamicFriction with invalid asset handle!");
				return;
			}

			physicsMaterial->Bounciness = bounciness;
		}

		CombineMode PhysicsMaterial_GetFrictionCombineMode(AssetHandle* assetHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.FrictionCombine with invalid asset handle!");
				return CombineMode::Average;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);

			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.FrictionCombine with invalid asset handle!");
				return CombineMode::Average;
			}

			return physicsMaterial->FrictionCombineMode;
		}

		void PhysicsMaterial_SetFrictionCombineMode(AssetHandle* assetHandle, CombineMode frictionCombineMode)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.FrictionCombine with invalid asset handle!");
				return;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);

			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.FrictionCombine with invalid asset handle!");
				return;
			}

			physicsMaterial->FrictionCombineMode = frictionCombineMode;
		}

		CombineMode PhysicsMaterial_GetBouncinessCombineMode(AssetHandle* assetHandle)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.BouncinessCombineFrictionCombine with invalid asset handle!");
				return CombineMode::Average;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);

			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to access PhysicsMaterial.BouncinessCombineFrictionCombine with invalid asset handle!");
				return CombineMode::Average;
			}

			return physicsMaterial->BouncinessCombineMode;
		}

		void PhysicsMaterial_SetBouncinessCombineMode(AssetHandle* assetHandle, CombineMode bouncinessCombineMode)
		{
			if (!AssetManager::IsHandleValid(*assetHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.BouncinessCombineFrictionCombine with invalid asset handle!");
				return;
			}

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(*assetHandle);

			if (!physicsMaterial)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set PhysicsMaterial.BouncinessCombineFrictionCombine with invalid asset handle!");
				return;
			}

			physicsMaterial->BouncinessCombineMode = bouncinessCombineMode;
		}

#pragma endregion

#pragma region Character Controller Component

		void CharacterControllerComponent_Move(UUID entityUUID, Math::vec3* displacement)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Calling CharacterController.Move without a Character Controller!");
				return;
			}

			Physics::OnCharacterControllerUpdateRuntime(entityUUID, *displacement);
		}

		void CharacterControllerComponent_Jump(UUID entityUUID, float jumpForce)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Calling CharacterController.Jump without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.SpeedDown = -1.0f * jumpForce;
		}

		bool CharacterControllerComponent_IsGrounded(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.IsGrounded without a Character Controller!");
				return false;
			}

			physx::PxController* controller = Physics::GetController(entityUUID);
			physx::PxControllerState state;
			controller->getState(state);

			return state.collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN;
		}

		void CharacterControllerComponent_GetFootPosition(UUID entityUUID, Math::vec3* outFootPos)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.FootPosition without a Character Controller!");
				return;
			}

			physx::PxExtendedVec3 footPosition = Physics::GetController(entityUUID)->getFootPosition();
			*outFootPos = FromPhysXExtendedVector(footPosition);
		}

		float CharacterControllerComponent_GetSpeedDown(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.SpeedDown without a Character Controller!");
				return 0.0f;
			}

			const CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			return characterController.SpeedDown;
		}

		float CharacterControllerComponent_GetSlopeLimit(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.SlopeLimit without a Character Controller!");
				return 0.0f;
			}

			const CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			return characterController.SlopeLimitDegrees;
		}

		void CharacterControllerComponent_SetSlopeLimit(UUID entityUUID, float slopeLimit)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.SlopeLimit without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.SlopeLimitDegrees = slopeLimit;

			Physics::GetController(entityUUID)->setSlopeLimit(Math::Max(0.0f, cosf(Math::Deg2Rad(slopeLimit))));
		}

		float CharacterControllerComponent_GetStepOffset(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.StepOffset without a Character Controller!");
				return 0.0f;
			}

			const CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			return characterController.StepOffset;
		}

		void CharacterControllerComponent_SetStepOffset(UUID entityUUID, float stepOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.StepOffset without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.StepOffset = stepOffset;

			Physics::GetController(entityUUID)->setStepOffset(stepOffset);
		}

		float CharacterControllerComponent_GetContactOffset(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.ContactOffset without a Character Controller!");
				return 0.0f;
			}

			const CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			return characterController.ContactOffset;
		}

		void CharacterControllerComponent_SetContactOffset(UUID entityUUID, float contactOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.ContactOffset without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.ContactOffset = contactOffset;

			Physics::GetController(entityUUID)->setContactOffset(contactOffset);
		}

		NonWalkableMode CharacterControllerComponent_GetNonWalkableMode(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.NonWalkMode without a Character Controller!");
				return NonWalkableMode::PreventClimbing;
			}

			const CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			return characterController.NonWalkMode;
		}

		void CharacterControllerComponent_SetNonWalkableMode(UUID entityUUID, NonWalkableMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.NonWalkMode without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.NonWalkMode = mode;

			Physics::GetController(entityUUID)->setNonWalkableMode((physx::PxControllerNonWalkableMode::Enum)mode);
		}

		CapsuleClimbMode CharacterControllerComponent_GetClimbMode(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.ClimbMode without a Character Controller!");
				return CapsuleClimbMode::Easy;
			}

			const CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			return characterController.ClimbMode;
		}

		void CharacterControllerComponent_SetClimbMode(UUID entityUUID, CapsuleClimbMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.ClimbMode without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.ClimbMode = mode;
			// TODO any way to set capsule climbing mode during runtime?
		}

		bool CharacterControllerComponent_GetDisableGravity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.DisableGravity without a Character Controller!");
				return false;
			}

			const CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			return characterController.DisableGravity;
		}

		void CharacterControllerComponent_SetDisableGravity(UUID entityUUID, bool disableGravity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.DisableGravity without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.DisableGravity = disableGravity;
		}

#pragma endregion

#pragma region FixedJoint Component

		uint64_t FixedJointComponent_GetConnectedEntity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.ConnectedEntity without a Fixed Joint!");
				return 0;
			}

			const FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			return fixedJointComponent.ConnectedEntity;
		}

		void FixedJointComponent_SetConnectedEntity(UUID entityUUID, UUID connectedEntityUUID)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.ConnectedEntity without a Fixed Joint!");
				return;
			}

			if (!contextScene->TryGetEntityWithUUID(connectedEntityUUID))
				return;

			physx::PxRigidActor* actor0 = Physics::GetActor(entityUUID);
			physx::PxRigidActor* actor1 = Physics::GetActor(connectedEntityUUID);

			if (!actor0 || !actor1)
			{
				return;
			}

			FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			fixedJointComponent.ConnectedEntity = connectedEntityUUID;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(entityUUID);

			fixedJoint->setActors(actor0, actor1);
		}

		float FixedJointComponent_GetBreakForce(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.BreakForce without a Fixed Joint!");
				return 0.0f;
			}

			const FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			return fixedJointComponent.BreakForce;
		}

		void FixedJointComponent_SetBreakForce(UUID entityUUID, float breakForce)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.BreakForce without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			fixedJointComponent.BreakForce = breakForce;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(entityUUID);
			fixedJoint->setBreakForce(breakForce, fixedJointComponent.BreakTorque);
		}

		float FixedJointComponent_GetBreakTorque(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.BreakTorque without a Fixed Joint!");
				return 0.0f;
			}

			const FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			return fixedJointComponent.BreakTorque;
		}

		void FixedJointComponent_SetBreakTorque(UUID entityUUID, float breakTorque)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.BreakTorque without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			fixedJointComponent.BreakTorque = breakTorque;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(entityUUID);
			fixedJoint->setBreakForce(fixedJointComponent.BreakForce, breakTorque);
		}

		void FixedJointComponent_SetBreakForceAndTorque(UUID entityUUID, float breakForce, float breakTorque)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Calling FixedJoint.SetBreakForceAndTorque without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			fixedJointComponent.BreakForce = breakForce;
			fixedJointComponent.BreakTorque = breakTorque;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(entityUUID);
			fixedJoint->setBreakForce(breakForce, breakTorque);
		}

		bool FixedJointComponent_GetEnableCollision(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.CollisionEnabled without a Fixed Joint!");
				return false;
			}

			const FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			return fixedJointComponent.EnableCollision;
		}

		void FixedJointComponent_SetCollisionEnabled(UUID entityUUID, bool enableCollision)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.CollisionEnabled without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			fixedJointComponent.EnableCollision = enableCollision;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(entityUUID);
			fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, enableCollision);
		}

		bool FixedJointComponent_GetPreProcessingEnabled(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.PreProcessingEnabled without a Fixed Joint!");
				return false;
			}

			const FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			return fixedJointComponent.EnablePreProcessing;
		}

		void FixedJointComponent_SetPreProcessingEnabled(UUID entityUUID, bool enablePreProcessing)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.PreProcessingEnabled without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			fixedJointComponent.EnableCollision = enablePreProcessing;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(entityUUID);
			fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eDISABLE_PREPROCESSING, !enablePreProcessing);
		}

		bool FixedJointComponent_IsBroken(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.IsBroken without a Fixed Joint!");
				return false;
			}

			return Physics::IsConstraintBroken(entityUUID);
		}

		bool FixedJointComponent_GetIsBreakable(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.IsBreakable without a Fixed Joint!");
				return false;
			}

			const FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			return fixedJointComponent.IsBreakable;
		}

		void FixedJointComponent_SetIsBreakable(UUID entityUUID, bool isBreakable)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.IsBreakable without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			fixedJointComponent.IsBreakable = isBreakable;
		}

		void FixedJointComponent_Break(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Calling FixedJoint.Break without a Fixed Joint!");
				return;
			}

			Physics::BreakJoint(entityUUID);
		}

#pragma endregion

#pragma region BoxCollider Component

		void BoxColliderComponent_GetHalfSize(UUID entityUUID, Math::vec3* outHalfSize)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider.HalfSize without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				*outHalfSize = boxCollider->GetHalfSize();
			}
		}

		void BoxColliderComponent_SetHalfSize(UUID entityUUID, Math::vec3* halfSize)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider.HalfSize without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				boxCollider->SetHalfSize(*halfSize);
			}
		}

		void BoxColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider.Offset without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				*outOffset = boxCollider->GetOffset();
			}
		}

		void BoxColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider.Offset without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				boxCollider->SetOffset(*offset);
			}
		}

		bool BoxColliderComponent_GetIsTrigger(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider.IsTrigger without a Box Collider!");
				return false;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				return boxCollider->IsTrigger();
			}

			return false;
		}

		void BoxColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider.IsTrigger without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				boxCollider->SetTrigger(isTrigger);
			}
		}

		bool BoxColliderComponent_GetMaterialHandle(UUID entityUUID, AssetHandle* outHandle)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider.Material without a Box Collider!");
				return false;
			}

			const BoxColliderComponent& boxCollider = entity.GetComponent<BoxColliderComponent>();
			*outHandle = boxCollider.Material;
			return AssetManager::IsHandleValid(boxCollider.Material);
		}

#pragma endregion

#pragma region SphereCollider Component

		float SphereColliderComponent_GetRadius(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SphereCollider.Radius without a Sphere Collider!");
				return 0.0f;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				return sphereCollider->GetRadius();
			}

			return 0.0f;
		}

		void SphereColliderComponent_SetRadius(UUID entityUUID, float radius)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SphereCollider.Radius without a Sphere Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				sphereCollider->SetRadius(radius);
			}
		}

		void SphereColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SphereCollider.Offset without a Sphere Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				*outOffset = sphereCollider->GetOffset();
			}
		}

		void SphereColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SphereCollider.Offset without a Sphere Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				sphereCollider->SetOffset(*offset);
			}
		}

		bool SphereColliderComponent_GetIsTrigger(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SphereCollider.IsTrigger without a Sphere Collider!");
				return false;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				return sphereCollider->IsTrigger();
			}

			return false;
		}

		void SphereColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SphereCollider.Offset without a Sphere Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				sphereCollider->SetTrigger(isTrigger);
			}
		}

		bool SphereColliderComponent_GetMaterialHandle(UUID entityUUID, AssetHandle* outHandle)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SphereCollider.Material without a Sphere Collider!");
				return false;
			}

			const SphereColliderComponent& sphereCollider = entity.GetComponent<SphereColliderComponent>();
			*outHandle = sphereCollider.Material;
			return AssetManager::IsHandleValid(sphereCollider.Material);
		}

#pragma endregion

#pragma region CapsuleCollider Component

		float CapsuleColliderComponent_GetRadius(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.Radius without a Capsule Collider!");
				return 0.0f;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				return capsuleCollider->GetRadius();
			}

			return 0.0f;
		}

		void CapsuleColliderComponent_SetRadius(UUID entityUUID, float radius)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CapsuleCollider.Radius without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				capsuleCollider->SetRadius(radius);
			}
		}

		float CapsuleColliderComponent_GetHeight(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.Height without a Capsule Collider!");
				return 0.0f;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				return capsuleCollider->GetHeight();
			}

			return 0.0f;
		}

		void CapsuleColliderComponent_SetHeight(UUID entityUUID, float height)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CapsuleCollider.Height without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				capsuleCollider->SetHeight(height);
			}
		}

		void CapsuleColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.Offset without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				*outOffset = capsuleCollider->GetOffset();
			}
		}

		void CapsuleColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CapsuleCollider.Offset without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				capsuleCollider->SetOffset(*offset);
			}
		}

		bool CapsuleColliderComponent_GetIsTrigger(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.IsTrigger without a Capsule Collider!");
				return false;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			const auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				return capsuleCollider->IsTrigger();
			}

			return false;
		}

		void CapsuleColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CapsuleCollider.IsTrigger without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				capsuleCollider->SetTrigger(isTrigger);
			}
		}

		bool CapsuleColliderComponent_GetMaterialHandle(UUID entityUUID, AssetHandle* outHandle)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.Material without a Capsule Collider!");
				return false;
			}

			const CapsuleColliderComponent& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();
			*outHandle = capsuleCollider.Material;
			return AssetManager::IsHandleValid(capsuleCollider.Material);
		}

#pragma endregion

#pragma region MeshCollider Component

		// Finish these once we have collider assets

		bool MeshColliderComponent_IsStaticMesh(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);
			
			if (!entity.HasComponent<MeshColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access MeshCollider.IsStaticMesh without a Mesh Collider!");
				return false;
			}

			const MeshColliderComponent& meshCollider = entity.GetComponent<MeshColliderComponent>();
			const auto& colliders = Physics::GetEntityColliders(entityUUID);
			SharedReference<ColliderShape> colliderShape = colliders.back();

			if (SharedReference<ConvexMeshShape> convexMeshShape = colliderShape.Is<ConvexMeshShape>())
			{
				//const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata();
				//return metadata.Type == AssetType::StaticMeshAsset;
			}
			else if (SharedReference<TriangleMeshShape> triangleMeshShape = colliderShape.Is<TriangleMeshShape>())
			{
				//const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata();
				//return metadata.Type == AssetType::StaticMeshAsset;
			}

			return false;
		}

		bool MeshColliderComponent_IsColliderMeshValid(UUID entityUUID, AssetHandle* assetHandle)
		{
			return false;
		}

		bool MeshColliderComponent_GetColliderMesh(UUID entityUUID, AssetHandle* outHandle)
		{
			return false;
		}

		void MeshColliderComponent_SetColliderMesh(UUID entityUUID, AssetHandle assetHandle)
		{
		}

		bool MeshColliderComponent_GetIsTrigger(UUID entityUUID)
		{
			return false;
		}

		void MeshColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
		{
		}

		bool MeshColliderComponent_GetMaterialHandle(UUID entityUUID, AssetHandle* outHandle)
		{
			return false;
		}

#pragma endregion

#pragma region RigidBody2D Component

		RigidBody2DType RigidBody2DComponent_GetBodyType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.BodyType without a RigidBody 2D!");
				return RigidBody2DType::Static;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			return rigidbody.Type;
		}

		void RigidBody2DComponent_SetBodyType(UUID entityUUID, RigidBody2DType bodyType)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.BodyType without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			const bool consistentBodyType = bodyType == rigidbody.Type;

			if (consistentBodyType)
				return;

			Physics2D::DestroyPhysicsBody(entity);
			rigidbody.Type = bodyType;
			rigidbody.RuntimeBody = nullptr;
			Physics2D::CreatePhysicsBody(entity, entity.GetTransform(), rigidbody);
		}

		void RigidBody2DComponent_GetVelocity(UUID entityUUID, Math::vec2* outVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.Velocity without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			*outVelocity = rigidbody.Velocity;
		}

		void RigidBody2DComponent_SetVelocity(UUID entityUUID, Math::vec2* velocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.Velocity without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			rigidbody.Velocity = *velocity;
		}

		float RigidBody2DComponent_GetDrag(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.Drag without a RigidBody 2D!");
				return 0.0f;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			return rigidbody.Drag;
		}

		void RigidBody2DComponent_SetDrag(UUID entityUUID, float drag)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.Drag without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			rigidbody.Drag = drag;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetLinearDamping(drag);
		}

		float RigidBody2DComponent_GetAngularVelocity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.AngularVelocity without a RigidBody 2D!");
				return 0.0f;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			return rigidbody.AngularVelocity;
		}
		
		void RigidBody2DComponent_SetAngularVelocity(UUID entityUUID, float angularVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.AngularVelocity without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			rigidbody.AngularVelocity = angularVelocity;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetAngularVelocity(angularVelocity);
		}
		
		float RigidBody2DComponent_GetAngularDrag(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.AngularDrag without a RigidBody 2D!");
				return 0.0f;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			return rigidbody.AngularDrag;
		}

		void RigidBody2DComponent_SetAngularDrag(UUID entityUUID, float angularDrag)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.AngularDrag without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			rigidbody.AngularDrag = angularDrag;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetAngularDamping(angularDrag);
		}

		bool RigidBody2DComponent_GetFixedRotation(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.FxiedRotation without a RigidBody 2D!");
				return false;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			return rigidbody.FixedRotation;
		}

		void RigidBody2DComponent_SetFixedRotation(UUID entityUUID, bool freeze)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.FxiedRotation without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			rigidbody.FixedRotation = freeze;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetFixedRotation(freeze);
		}

		float RigidBody2DComponent_GetGravityScale(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.GravityScale without a RigidBody 2D!");
				return 0.0f;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			return rigidbody.GravityScale;
		}

		void RigidBody2DComponent_SetGravityScale(UUID entityUUID, float gravityScale)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.GravityScale without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			rigidbody.GravityScale = gravityScale;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetGravityScale(gravityScale);
		}

		void RigidBody2DComponent_ApplyForce(UUID entityUUID, Math::vec2* force, Math::vec2* point, bool wake)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling RigidBody2D.ApplyForce without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->ApplyForce(b2Vec2(force->x, force->y), b2Vec2(point->x, point->y), wake);
		}

		void RigidBody2DComponent_ApplyForceToCenter(UUID entityUUID, Math::vec2* force, bool wake)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling RigidBody2D.ApplyForceToCenter without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
		}

		void RigidBody2DComponent_ApplyLinearImpulse(UUID entityUUID, Math::vec2* impulse, Math::vec2* point, bool wake)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling RigidBody2D.ApplyLinearImpulse without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
		}

		void RigidBody2DComponent_ApplyLinearImpulseToCenter(UUID entityUUID, Math::vec2* impulse, bool wake)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling RigidBody2D.ApplyLinearImpulseToCenter without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
		}

#pragma endregion

#pragma region Physics2D

		uint64_t Physics2D_Raycast(Math::vec2* start, Math::vec2* end, RaycastHit2D* outResult, bool drawDebugLine)
		{
			Scene* contextScene = GetContextScene();

			return Physics2D::Raycast(*start, *end, outResult, drawDebugLine);
		}

		void Physics2D_GetWorldGravity(Math::vec2* outGravity)
		{
			Scene* contextScene = GetContextScene();

			*outGravity = Physics2D::GetPhysicsWorldGravity();
		}

		void Physics2D_SetWorldGravity(Math::vec2* gravity)
		{
			Scene* contextScene = GetContextScene();

			Physics2D::SetPhysicsWorldGravitty(*gravity);
		}

		uint32_t Physics2D_GetWorldPositionIterations()
		{
			Scene* contextScene = GetContextScene();

			return Physics2D::GetPhysicsWorldPositionIterations();
		}

		void Physics2D_SetWorldPositionIterations(uint32_t positionIterations)
		{
			Scene* contextScene = GetContextScene();

			Physics2D::SetPhysicsWorldPositionIterations(positionIterations);
		}

		uint32_t Physics2D_GetWorldVelocityIterations()
		{
			Scene* contextScene = GetContextScene();

			return Physics2D::GetPhysicsWorldVelocityIterations();
		}

		void Physics2D_SetWorldVelocityIterations(uint32_t velocityIterations)
		{
			Scene* contextScene = GetContextScene();

			Physics2D::SetPhysicsWorldVelocityIterations(velocityIterations);
		}

#pragma endregion

#pragma region Box Collider2D Component

		void BoxCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Offset without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			*outOffset = boxCollider.Offset;
		}

		void BoxCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Offset without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			boxCollider.Offset = *offset;

			Physics2D::DestroyPhysicsBody(entity);
			Physics2D::CreatePhysicsBody(entity, GetContextScene()->GetWorldSpaceTransform(entity), entity.GetComponent<RigidBody2DComponent>());
		}

		void BoxCollider2DComponent_GetSize(UUID entityUUID, Math::vec2* outSize)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Size without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			*outSize = boxCollider.Size;
		}

		void BoxCollider2DComponent_SetSize(UUID entityUUID, Math::vec2* size)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Size without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			boxCollider.Size = *size;

			Physics2D::DestroyPhysicsBody(entity);
			Physics2D::CreatePhysicsBody(entity, GetContextScene()->GetWorldSpaceTransform(entity), entity.GetComponent<RigidBody2DComponent>());
		}

		void BoxCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Density without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			*outDensity = boxCollider.Density;
		}

		void BoxCollider2DComponent_SetDensity(UUID entityUUID, float density)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Density without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			boxCollider.Density = density;

			b2Fixture* fixture = (b2Fixture*)boxCollider.RuntimeFixture;
			
			fixture->SetDensity(density);

			// Since we changed the density we must recalculate the mass data according to box2d
			fixture->GetBody()->ResetMassData();
		}

		void BoxCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Friction without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			*outFriction = boxCollider.Friction;
		}

		void BoxCollider2DComponent_SetFriction(UUID entityUUID, float friction)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Friction without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			boxCollider.Friction = friction;

			b2Fixture* fixture = (b2Fixture*)boxCollider.RuntimeFixture;

			fixture->SetFriction(friction);
		}

		void BoxCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Restitution without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			*outRestitution = boxCollider.Restitution;
		}

		void BoxCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Restitution without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			boxCollider.Restitution = restitution;

			b2Fixture* fixture = (b2Fixture*)boxCollider.RuntimeFixture;
			
			fixture->SetRestitution(restitution);
		}

		void BoxCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.RestitutionThreshold without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			*outRestitutionThreshold = boxCollider.RestitutionThreshold;
		}

		void BoxCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.RestitutionThreshold without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

			boxCollider.RestitutionThreshold = restitutionThreshold;

			b2Fixture* fixture = (b2Fixture*)boxCollider.RuntimeFixture;

			fixture->SetRestitutionThreshold(restitutionThreshold);
		}

#pragma endregion

#pragma region Circle Collider2D Component

		void CircleCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Offset without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			*outOffset = circleCollider.Offset;
		}

		void CircleCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Offset without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			circleCollider.Offset = *offset;

			Physics2D::DestroyPhysicsBody(entity);
			Physics2D::CreatePhysicsBody(entity, GetContextScene()->GetWorldSpaceTransform(entity), entity.GetComponent<RigidBody2DComponent>());
		}

		void CircleCollider2DComponent_GetRadius(UUID entityUUID, float* outRadius)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Radius without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			*outRadius = circleCollider.Radius;
		}

		void CircleCollider2DComponent_SetRadius(UUID entityUUID, float radius)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Radius without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			circleCollider.Radius = radius;

			Physics2D::DestroyPhysicsBody(entity);
			Physics2D::CreatePhysicsBody(entity, GetContextScene()->GetWorldSpaceTransform(entity), entity.GetComponent<RigidBody2DComponent>());
		}

		void CircleCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Density without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			*outDensity = circleCollider.Density;
		}

		void CircleCollider2DComponent_SetDensity(UUID entityUUID, float density)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Density without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			circleCollider.Density = density;

			b2Fixture* fixture = (b2Fixture*)circleCollider.RuntimeFixture;

			fixture->SetDensity(density);

			// Since we changed the density we must recalculate the mass data according to box2d
			fixture->GetBody()->ResetMassData();
		}

		void CircleCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Friction without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			*outFriction = circleCollider.Friction;
		}

		void CircleCollider2DComponent_SetFriction(UUID entityUUID, float friction)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Friction without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			circleCollider.Friction = friction;

			b2Fixture* fixture = (b2Fixture*)circleCollider.RuntimeFixture;

			fixture->SetFriction(friction);
		}

		void CircleCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Restitution without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			*outRestitution = circleCollider.Restitution;
		}

		void CircleCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Restitution without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			circleCollider.Restitution = restitution;

			b2Fixture* fixture = (b2Fixture*)circleCollider.RuntimeFixture;

			fixture->SetRestitution(restitution);
		}

		void CircleCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.RestitutionThreshold without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			*outRestitutionThreshold = circleCollider.RestitutionThreshold;
		}

		void CircleCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.RestitutionThreshold without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

			circleCollider.RestitutionThreshold = restitutionThreshold;

			b2Fixture* fixture = (b2Fixture*)circleCollider.RuntimeFixture;

			fixture->SetRestitutionThreshold(restitutionThreshold);
		}

#pragma endregion

#pragma region Texture2D

		bool Texture2D_LoadFromPath(MonoString* filepath, AssetHandle* outHandle)
		{
			char* filepathCStr = mono_string_to_utf8(filepath);

			*outHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepathCStr);

			mono_free(filepathCStr);

			return AssetManager::IsHandleValid(*outHandle);
		}

		void Texture2D_Constructor(uint32_t width, uint32_t height, AssetHandle* outHandle)
		{
			TextureProperties imageProps;
			imageProps.Width = width;
			imageProps.Height = height;
			imageProps.WrapMode = ImageWrap::Repeat;

			SharedReference<Texture2D> texture = Texture2D::Create(imageProps);
			texture->Handle = AssetHandle();

			Project::GetEditorAssetManager()->AddMemoryOnlyAsset(texture);
			*outHandle = texture->Handle;
		}

		uint32_t Texture2D_GetWidth(AssetHandle* textureHandle)
		{
			if (!AssetManager::IsHandleValid(*textureHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Texture2D.Width with invalid asset handle!");
				return 0;
			}

			SharedReference<Texture2D> texture = AssetManager::GetAsset<Texture2D>(*textureHandle);

			return texture->GetWidth();
		}

		uint32_t Texture2D_GetHeight(AssetHandle* textureHandle)
		{
			if (!AssetManager::IsHandleValid(*textureHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access Texture2D.Height with invalid asset handle!");
				return 0;
			}

			SharedReference<Texture2D> texture = AssetManager::GetAsset<Texture2D>(*textureHandle);

			return texture->GetHeight();
		}

		void Texture2D_SetPixel(AssetHandle* textureHandle, uint32_t x, uint32_t y, Math::vec4* color)
		{
			if (!AssetManager::IsHandleValid(*textureHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling Texture2D.SetPixel with invalid asset handle!");
				return;
			}

			SharedReference<Texture2D> texture = AssetManager::GetAsset<Texture2D>(*textureHandle);

			texture->SetPixel(x, y, (void*)color);
		}

#pragma endregion

#pragma region Random

		int Random_RangedInt32(int min, int max)
		{
			std::random_device randomDevice;
			std::mt19937 engine(randomDevice());
			std::uniform_int_distribution<int> uniformDistribution(min, max);

			return uniformDistribution(engine);
		}

		float Random_RangedFloat(float min, float max)
		{
			std::random_device randomDevice;
			std::mt19937 engine(randomDevice());
			std::uniform_real_distribution<float> uniformDistribution(min, max);

			return uniformDistribution(engine);
		}

		float Random_Float()
		{
			return Random::Float();
		}

#pragma endregion

#pragma region Quaternion

		void Quaternion_Inverse(Math::quaternion* quat, Math::quaternion* result)
		{
			*result = Math::Inverse(*quat);
		}

#pragma endregion

#pragma region Matrix4

		void Matrix4_Rotate(float angleDeg, Math::vec3* axis, Math::mat4* outResult)
		{
			*outResult = Math::Rotate(Math::Deg2Rad(angleDeg), *axis);
		}

		void Matrix4_LookAt(Math::vec3* eyePos, Math::vec3* worldPoint, Math::vec3* up, Math::mat4* outResult)
		{
			*outResult = Math::LookAt(*eyePos, *worldPoint, *up);
		}

		void Matrix4_Multiply(Math::mat4* matrix, Math::mat4* other, Math::mat4* outResult)
		{
			*outResult = (*matrix) * (*other);
		}

#pragma endregion

#pragma region Mathf

		float Mathf_GetPI()
		{
			return Math::PI;
		}

		double Mathf_GetPI_D()
		{
			return Math::PI_D;
		}

		float Mathf_Round(float value)
		{
			return Math::Round(value);
		}

		float Mathf_Abs(float in)
		{
			return Math::Abs(in);
		}

		float Mathf_Sqrt(float in)
		{
			return Math::Sqrt(in);
		}

		float Mathf_Pow(float base, float power)
		{
			return Math::Pow(base, power);
		}

		float Mathf_Sin(float in)
		{
			return Math::Sin(in);
		}

		float Mathf_Cos(float in)
		{
			return Math::Cos(in);
		}

		float Mathf_Acos(float in)
		{
			return Math::Acos(in);
		}

		float Mathf_Tan(float in)
		{
			return Math::Tan(in);
		}

		float Mathf_Max(float x, float y)
		{
			return Math::Max(x, y);
		}

		float Mathf_Min(float x, float y)
		{
			return Math::Min(x, y);
		}

		float Mathf_Deg2Rad(float degrees)
		{
			return Math::Deg2Rad(degrees);
		}

		float Mathf_Rad2Deg(float radians)
		{
			return Math::Rad2Deg(radians);
		}

		void Mathf_Deg2RadVector3(Math::vec3* value, Math::vec3* outResult)
		{
			*outResult = Math::Deg2Rad(*value);
		}

		void Mathf_Rad2DegVector3(Math::vec3* value, Math::vec3* outResult)
		{
			*outResult = Math::Rad2Deg(*value);
		}

		void Mathf_LookAt(Math::vec3* eyePos, Math::vec3* worldPoint, Math::vec3* outRotation)
		{
			Math::vec3 up{ 0.0f, 1.0f, 0.0f };
			Math::mat4 transform = Math::LookAt(*eyePos, *worldPoint, up);

			Math::vec3 translation, scale;
			Math::quaternion rotation;
			Math::vec3 skew;
			Math::vec4 perspective;
			Math::Decompose(transform, scale, rotation, translation, skew, perspective);
			*outRotation = Math::EulerAngles(rotation);
		}

		void Mathf_InverseQuat(Math::quaternion* rotation, Math::quaternion* result)
		{
			// TODO fix this
			*result = Math::Inverse(*rotation);
		}

#pragma endregion

#pragma region Noise

		Noise* Noise_Constructor(int seed, NoiseType type)
		{
			return new Noise(seed, type);
		}

		void Noise_Destructor(Noise* _this)
		{
			delete _this;
		}

		float Noise_GetFrequency(Noise* _this)
		{
			return _this->GetFrequency();
		}

		void Noise_SetFrequency(Noise* _this, float frequency)
		{
			_this->SetFrequency(frequency);
		}

		int Noise_GetFractalOctaves(Noise* _this)
		{
			return _this->GetFractalOctaves();
		}

		void Noise_SetFractalOctaves(Noise* _this, int octaves)
		{
			_this->SetFractalOctaves(octaves);
		}

		float Noise_GetFractalLacunarity(Noise* _this)
		{
			return _this->GetFractalLacunarity();
		}

		void Noise_SetFractalLacunarity(Noise* _this, float lacunarity)
		{
			_this->SetFractalLacunarity(lacunarity);
		}

		float Noise_GetFractalGain(Noise* _this)
		{
			return _this->GetFractalGain();
		}

		void Noise_SetFractalGain(Noise* _this, float gain)
		{
			_this->SetFractalGain(gain);
		}

		float Noise_GetVec2(Noise* _this, float x, float y)
		{
			return _this->Get(x, y);
		}

		float Noise_GetVec3(Noise* _this, float x, float y, float z)
		{
			return _this->Get(x, y, z);
		}

		void Noise_SetSeed(int seed)
		{
			Noise::SetSeed(seed);
		}

		float Noise_PerlinNoiseVec2(float x, float y)
		{
			return Noise::PerlinNoise(x, y);
		}

		float Noise_PerlinNoiseVec3(float x, float y, float z)
		{
			return Noise::PerlinNoise(x, y, z);
		}

#pragma endregion

#pragma region Time

		float Time_GetElapsed()
		{
			return Time::GetTime() - s_Data.SceneStartTime;
		}

		float Time_GetDeltaTime()
		{
			return Time::GetDeltaTime();
		}

#pragma endregion

#pragma region Input

		bool Input_IsKeyPressed(KeyCode key)
		{
			return Input::IsKeyPressed(key);
		}

		bool Input_IsKeyReleased(KeyCode key)
		{
			return Input::IsKeyReleased(key);
		}

		bool Input_IsKeyDown(KeyCode key)
		{
			return Input::IsKeyDown(key);
		}

		bool Input_IsKeyUp(KeyCode key)
		{
			return Input::IsKeyUp(key);
		}

		bool Input_IsMouseButtonPressed(MouseButton mouseButton)
		{
			return Input::IsMouseButtonPressed(mouseButton);
		}

		bool Input_IsMouseButtonReleased(MouseButton mouseButton)
		{
			return Input::IsMouseButtonReleased(mouseButton);
		}

		bool Input_IsMouseButtonDown(MouseButton mouseButton)
		{
			return Input::IsMouseButtonDown(mouseButton);
		}

		bool Input_IsMouseButtonUp(MouseButton mouseButton)
		{
			return Input::IsMouseButtonUp(mouseButton);
		}

		void Input_GetMousePosition(Math::vec2* outPosition)
		{
			Math::vec2 mousePos = Input::GetMousePosition();
			mousePos.y *= -1.0f; // This makes more sense
			*outPosition = mousePos;
		}

		void Input_SetMousePosition(Math::vec2* position)
		{
			Input::SetMousePosition(*position);
		}

		void Input_GetMouseWheelMovement(Math::vec2* outMouseScrollOffset)
		{
			*outMouseScrollOffset = Input::GetMouseScrollOffset();
		}

		bool Input_IsGamepadButtonDown(GamepadButton button)
		{
			return Input::IsGamepadButtonDown(button);
		}

		bool Input_IsGamepadButtonUp(GamepadButton button)
		{
			return Input::IsGamepadButtonUp(button);
		}

		float Input_GetGamepadAxis(GamepadAxis axis)
		{
			return Input::GetGamepadAxis(axis);
		}

		CursorMode Input_GetCursorMode()
		{
			return Input::GetCursorMode();
		}

		void Input_SetCursorMode(CursorMode cursorMode)
		{
			Input::SetCursorMode(cursorMode);
		}

#pragma endregion

#pragma region Gui

		void BeginWindow(char* text, uint32_t flags)
		{
			Gui::Begin(text, nullptr, flags);
			UI::BeginPropertyGrid();
		}

		void Gui_Begin(MonoString* text)
		{
			char* textCStr = mono_string_to_utf8(text);

			BeginWindow(textCStr);

			mono_free(textCStr);
		}

		void Gui_BeginWithPosition(MonoString* text, Math::vec2* position)
		{
			char* textCStr = mono_string_to_utf8(text);

			Gui::SetNextWindowPos({ position->x, position->y });
			BeginWindow(textCStr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

			mono_free(textCStr);
		}

		void Gui_BeginWithSize(MonoString* text, float width, float height)
		{
			char* textCStr = mono_string_to_utf8(text);

			Gui::SetNextWindowSize({ width, height });
			BeginWindow(textCStr, ImGuiWindowFlags_NoResize);

			mono_free(textCStr);
		}

		void Gui_BeginWithPositionAndSize(MonoString* text, Math::vec2* position, Math::vec2* size)
		{
			char* textCStr = mono_string_to_utf8(text);

			Gui::SetNextWindowPos({ position->x, position->y });
			Gui::SetNextWindowSize({ size->x, size->y });
			BeginWindow(textCStr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

			mono_free(textCStr);
		}

		void Gui_End()
		{
			UI::EndPropertyGrid();
			Gui::End();
		}

		void Gui_Underline()
		{
			UI::Draw::Underline();
		}

		void Gui_Spacing(unsigned int count)
		{
			for (uint32_t i = 0; i < count; i++)
			{
				Gui::Spacing();
			}
		}

		void Gui_Text(MonoString* text)
		{
			char* textCStr = mono_string_to_utf8(text);

			Gui::Text(textCStr);

			mono_free(textCStr);
		}

		bool Gui_Button(MonoString* text)
		{
			char* textCStr = mono_string_to_utf8(text);

			bool result = Gui::Button(textCStr);

			mono_free(textCStr);

			return result;
		}

		bool Gui_PropertyBool(MonoString* label, bool* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, *value);

			mono_free(textCStr);

			return modified;
		}

		bool Gui_PropertyInt(MonoString* label, int* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, *value);

			mono_free(textCStr);

			return modified;
		}

		bool Gui_PropertyULong(MonoString* label, unsigned int* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, *value);

			mono_free(textCStr);

			return modified;
		}

		bool Gui_PropertyFloat(MonoString* label, float* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, *value);

			mono_free(textCStr);

			return modified;
		}

		bool Gui_PropertyDouble(MonoString* label, double* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, *value);

			mono_free(textCStr);

			return modified;
		}

		bool Gui_PropertyVec2(MonoString* label, Math::vec2* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, *value);

			mono_free(textCStr);

			return modified;
		}

		bool Gui_PropertyVec3(MonoString* label, Math::vec3* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, *value);

			mono_free(textCStr);

			return modified;
		}

		bool Gui_PropertyVec4(MonoString* label, Math::vec4* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, *value);

			mono_free(textCStr);

			return modified;
		}

		bool Gui_PropertyColor3(MonoString* label, Math::vec3* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, value);

			mono_free(textCStr);

			return modified;
		}

		bool Gui_PropertyColor4(MonoString* label, Math::vec4* value)
		{
			char* textCStr = mono_string_to_utf8(label);

			bool modified = UI::Property(textCStr, value);

			mono_free(textCStr);

			return modified;
		}

#pragma endregion

#pragma region Log

		void Log_Message(MonoString* message, Log::LogLevel type)
		{
			char* managedString = mono_string_to_utf8(message);

			switch (type)
			{
				case Log::LogLevel::Trace:
					VX_CONSOLE_LOG_TRACE("{}", managedString);
					break;
				case Log::LogLevel::Info:
					VX_CONSOLE_LOG_INFO("{}", managedString);
					break;
				case Log::LogLevel::Warn:
					VX_CONSOLE_LOG_WARN("{}", managedString);
					break;
				case Log::LogLevel::Error:
					VX_CONSOLE_LOG_ERROR("{}", managedString);
					break;
				case Log::LogLevel::Fatal:
					VX_CONSOLE_LOG_FATAL("{}", managedString);
					break;
			}

			mono_free(managedString);
		}

#pragma endregion

	}

	template <typename... TComponent>
	static void RegisterComponent()
	{
		([]()
		{
			std::string_view typeName = typeid(TComponent).name();
			size_t pos = typeName.find_last_of(':');
			std::string_view structName = typeName.substr(pos + 1);
			pos = structName.find("Component"); // In C# the api doesn't include 'Component' in the names of Components
			std::string managedTypename = fmt::format("Vortex.{}", structName.substr(0, pos));

			MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());

			if (!managedType)
			{
				VX_CORE_ERROR_TAG("Scripting", "Could not find Component type {}", managedTypename);
				return;
			}

			s_Data.EntityAddComponentFuncs[managedType] = [](Entity entity) { entity.AddComponent<TComponent>(); };
			s_Data.EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<TComponent>(); };
			s_Data.EntityRemoveComponentFuncs[managedType] = [](Entity entity) { entity.RemoveComponent<TComponent>(); };
		}(), ...);
	}

	template <typename... TComponent>
	static void RegisterComponent(ComponentGroup<TComponent...>)
	{
		RegisterComponent<TComponent...>();
	}

	void ScriptRegistry::RegisterComponents()
	{
		s_Data.EntityHasComponentFuncs.clear();
		s_Data.EntityAddComponentFuncs.clear();
		s_Data.EntityRemoveComponentFuncs.clear();

		RegisterComponent(AllComponents{});
	}

	void ScriptRegistry::SetHoveredEntity(Entity entity)
	{
		s_Data.HoveredEntity = entity;
	}

	void ScriptRegistry::SetSceneStartTime(float startTime)
	{
		s_Data.SceneStartTime = startTime;
	}

	bool ScriptRegistry::HasPendingTransitionQueued()
	{
		return s_Data.NextSceneBuildIndex != -1;
	}

	uint32_t ScriptRegistry::GetNextBuildIndex()
	{
		return s_Data.NextSceneBuildIndex;
	}

	void ScriptRegistry::ResetBuildIndex()
	{
		s_Data.NextSceneBuildIndex = -1;
	}

	void ScriptRegistry::RegisterMethods()
	{
		VX_REGISTER_INTERNAL_CALL(Application_Quit);
		VX_REGISTER_INTERNAL_CALL(Application_GetSize);
		VX_REGISTER_INTERNAL_CALL(Application_GetPosition);
		VX_REGISTER_INTERNAL_CALL(Application_IsMaximized);

		VX_REGISTER_INTERNAL_CALL(SceneRenderer_GetBloomThreshold);
		VX_REGISTER_INTERNAL_CALL(SceneRenderer_SetBloomThreshold);
		VX_REGISTER_INTERNAL_CALL(SceneRenderer_GetBloomSoftKnee);
		VX_REGISTER_INTERNAL_CALL(SceneRenderer_SetBloomSoftKnee);
		VX_REGISTER_INTERNAL_CALL(SceneRenderer_GetBloomUnknown);
		VX_REGISTER_INTERNAL_CALL(SceneRenderer_SetBloomUnknown);
		VX_REGISTER_INTERNAL_CALL(SceneRenderer_GetExposure);
		VX_REGISTER_INTERNAL_CALL(SceneRenderer_SetExposure);
		VX_REGISTER_INTERNAL_CALL(SceneRenderer_GetGamma);
		VX_REGISTER_INTERNAL_CALL(SceneRenderer_SetGamma);

		VX_REGISTER_INTERNAL_CALL(DebugRenderer_BeginScene);
		VX_REGISTER_INTERNAL_CALL(DebugRenderer_SetClearColor);
		VX_REGISTER_INTERNAL_CALL(DebugRenderer_DrawLine);
		VX_REGISTER_INTERNAL_CALL(DebugRenderer_DrawQuadBillboard);
		VX_REGISTER_INTERNAL_CALL(DebugRenderer_DrawCircleVec2);
		VX_REGISTER_INTERNAL_CALL(DebugRenderer_DrawCircleVec3);
		VX_REGISTER_INTERNAL_CALL(DebugRenderer_DrawBoundingBox);
		VX_REGISTER_INTERNAL_CALL(DebugRenderer_DrawBoundingBoxFromTransform);
		VX_REGISTER_INTERNAL_CALL(DebugRenderer_Flush);

		VX_REGISTER_INTERNAL_CALL(Scene_GetPrimaryCamera);
		VX_REGISTER_INTERNAL_CALL(Scene_FindEntityByID);
		VX_REGISTER_INTERNAL_CALL(Scene_FindEntityByName);
		VX_REGISTER_INTERNAL_CALL(Scene_FindChildByName);
		VX_REGISTER_INTERNAL_CALL(Scene_CreateEntity);
		VX_REGISTER_INTERNAL_CALL(Scene_Instantiate);
		VX_REGISTER_INTERNAL_CALL(Scene_InstantiateAsChild);
		VX_REGISTER_INTERNAL_CALL(Scene_IsPaused);
		VX_REGISTER_INTERNAL_CALL(Scene_Pause);
		VX_REGISTER_INTERNAL_CALL(Scene_Resume);
		VX_REGISTER_INTERNAL_CALL(Scene_GetHoveredEntity);
		VX_REGISTER_INTERNAL_CALL(Scene_GetCurrentBuildIndex);

		VX_REGISTER_INTERNAL_CALL(SceneManager_LoadScene);
		VX_REGISTER_INTERNAL_CALL(SceneManager_LoadSceneFromBuildIndex);
		VX_REGISTER_INTERNAL_CALL(SceneManager_GetActiveScene);

		VX_REGISTER_INTERNAL_CALL(Entity_AddComponent);
		VX_REGISTER_INTERNAL_CALL(Entity_HasComponent);
		VX_REGISTER_INTERNAL_CALL(Entity_RemoveComponent);
		VX_REGISTER_INTERNAL_CALL(Entity_GetChildren);
		VX_REGISTER_INTERNAL_CALL(Entity_GetChild);
		VX_REGISTER_INTERNAL_CALL(Entity_GetTag);
		VX_REGISTER_INTERNAL_CALL(Entity_GetMarker);
		VX_REGISTER_INTERNAL_CALL(Entity_SetMarker);
		VX_REGISTER_INTERNAL_CALL(Entity_AddChild);
		VX_REGISTER_INTERNAL_CALL(Entity_RemoveChild);
		VX_REGISTER_INTERNAL_CALL(Entity_GetScriptInstance);
		VX_REGISTER_INTERNAL_CALL(Entity_Destroy);
		VX_REGISTER_INTERNAL_CALL(Entity_DestroyTimed);
		VX_REGISTER_INTERNAL_CALL(Entity_SetActive);

		VX_REGISTER_INTERNAL_CALL(AssetHandle_IsValid);

		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetTranslation);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_SetTranslation);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetRotation);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_SetRotation);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetEulerAngles);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_SetEulerAngles);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_Rotate);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_RotateAround);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_SetTranslationAndRotation);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetScale);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_SetScale);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetWorldSpaceTransform);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetTransformMatrix);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_SetTransformMatrix);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetForwardDirection);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetUpDirection);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetRightDirection);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_LookAt);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_GetParent);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_SetParent);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_Unparent);
		VX_REGISTER_INTERNAL_CALL(TransformComponent_Multiply);

		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetProjectionType);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetProjectionType);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetPrimary);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetPrimary);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetPerspectiveVerticalFOV);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetPerspectiveVerticalFOV);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetNearClip);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetNearClip);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetFarClip);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetFarClip);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetOrthographicSize);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetOrthographicSize);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetOrthographicNear);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetOrthographicNear);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetOrthographicFar);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetOrthographicFar);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetFixedAspectRatio);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetFixedAspectRatio);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_GetClearColor);
		VX_REGISTER_INTERNAL_CALL(CameraComponent_SetClearColor);

		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_GetLightType);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_SetLightType);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_GetRadiance);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_SetRadiance);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_GetIntensity);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_SetIntensity);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_GetCutoff);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_SetCutoff);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_GetOuterCutoff);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_SetOuterCutoff);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_GetShadowBias);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_SetShadowBias);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_GetCastShadows);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_SetCastShadows);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_GetSoftShadows);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_SetSoftShadows);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_IsVisible);
		VX_REGISTER_INTERNAL_CALL(LightSourceComponent_SetVisible);

		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_GetTextString);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_SetTextString);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_GetColor);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_SetColor);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_GetBackgroundColor);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_SetBackgroundColor);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_GetLineSpacing);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_SetLineSpacing);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_GetKerning);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_SetKerning);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_GetMaxWidth);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_SetMaxWidth);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_IsVisible);
		VX_REGISTER_INTERNAL_CALL(TextMeshComponent_SetVisible);

		VX_REGISTER_INTERNAL_CALL(AnimatorComponent_IsPlaying);
		VX_REGISTER_INTERNAL_CALL(AnimatorComponent_Play);
		VX_REGISTER_INTERNAL_CALL(AnimatorComponent_Stop);

		VX_REGISTER_INTERNAL_CALL(MeshRendererComponent_GetMaterialHandle);
		VX_REGISTER_INTERNAL_CALL(MeshRendererComponent_IsVisible);
		VX_REGISTER_INTERNAL_CALL(MeshRendererComponent_SetVisible);

		VX_REGISTER_INTERNAL_CALL(StaticMeshRendererComponent_GetMeshType);
		VX_REGISTER_INTERNAL_CALL(StaticMeshRendererComponent_SetMeshType);
		VX_REGISTER_INTERNAL_CALL(StaticMeshRendererComponent_SetMaterialHandle);
		VX_REGISTER_INTERNAL_CALL(StaticMeshRendererComponent_IsVisible);
		VX_REGISTER_INTERNAL_CALL(StaticMeshRendererComponent_SetVisible);
		VX_REGISTER_INTERNAL_CALL(StaticMeshRendererComponent_GetMaterialHandle);

		VX_REGISTER_INTERNAL_CALL(Material_GetAlbedo);
		VX_REGISTER_INTERNAL_CALL(Material_SetAlbedo);
		VX_REGISTER_INTERNAL_CALL(Material_GetAlbedoMap);
		VX_REGISTER_INTERNAL_CALL(Material_SetAlbedoMap);
		VX_REGISTER_INTERNAL_CALL(Material_GetNormalMap);
		VX_REGISTER_INTERNAL_CALL(Material_SetNormalMap);
		VX_REGISTER_INTERNAL_CALL(Material_GetMetallic);
		VX_REGISTER_INTERNAL_CALL(Material_SetMetallic);
		VX_REGISTER_INTERNAL_CALL(Material_GetMetallicMap);
		VX_REGISTER_INTERNAL_CALL(Material_SetMetallicMap);
		VX_REGISTER_INTERNAL_CALL(Material_GetRoughness);
		VX_REGISTER_INTERNAL_CALL(Material_SetRoughness);
		VX_REGISTER_INTERNAL_CALL(Material_GetRoughnessMap);
		VX_REGISTER_INTERNAL_CALL(Material_SetRoughnessMap);
		VX_REGISTER_INTERNAL_CALL(Material_GetEmission);
		VX_REGISTER_INTERNAL_CALL(Material_SetEmission);
		VX_REGISTER_INTERNAL_CALL(Material_GetEmissionMap);
		VX_REGISTER_INTERNAL_CALL(Material_SetEmissionMap);
		VX_REGISTER_INTERNAL_CALL(Material_GetAmbientOcclusionMap);
		VX_REGISTER_INTERNAL_CALL(Material_SetAmbientOcclusionMap);
		VX_REGISTER_INTERNAL_CALL(Material_GetUV);
		VX_REGISTER_INTERNAL_CALL(Material_SetUV);
		VX_REGISTER_INTERNAL_CALL(Material_GetOpacity);
		VX_REGISTER_INTERNAL_CALL(Material_SetOpacity);
		VX_REGISTER_INTERNAL_CALL(Material_IsFlagSet);
		VX_REGISTER_INTERNAL_CALL(Material_SetFlag);

		VX_REGISTER_INTERNAL_CALL(SpriteRendererComponent_GetTextureHandle);
		VX_REGISTER_INTERNAL_CALL(SpriteRendererComponent_SetTextureHandle);
		VX_REGISTER_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		VX_REGISTER_INTERNAL_CALL(SpriteRendererComponent_SetColor);
		VX_REGISTER_INTERNAL_CALL(SpriteRendererComponent_GetUV);
		VX_REGISTER_INTERNAL_CALL(SpriteRendererComponent_SetUV);
		VX_REGISTER_INTERNAL_CALL(SpriteRendererComponent_IsVisible);
		VX_REGISTER_INTERNAL_CALL(SpriteRendererComponent_SetVisible);

		VX_REGISTER_INTERNAL_CALL(CircleRendererComponent_GetColor);
		VX_REGISTER_INTERNAL_CALL(CircleRendererComponent_SetColor);
		VX_REGISTER_INTERNAL_CALL(CircleRendererComponent_GetThickness);
		VX_REGISTER_INTERNAL_CALL(CircleRendererComponent_SetThickness);
		VX_REGISTER_INTERNAL_CALL(CircleRendererComponent_GetFade);
		VX_REGISTER_INTERNAL_CALL(CircleRendererComponent_SetFade);
		VX_REGISTER_INTERNAL_CALL(CircleRendererComponent_IsVisible);
		VX_REGISTER_INTERNAL_CALL(CircleRendererComponent_SetVisible);

		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetPosition);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetPosition);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetDirection);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetDirection);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetVelocity);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetVelocity);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetMinGain);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetMinGain);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetMaxGain);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetMaxGain);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetAttenuationModel);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetAttenuationModel);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetFalloff);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetFalloff);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetMinDistance);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetMinDistance);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetMaxDistance);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetMaxDistance);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetPitch);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetPitch);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetDopplerFactor);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetDopplerFactor);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetVolume);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetVolume);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetPlayOnStart);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetPlayOnStart);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetIsSpacialized);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetIsSpacialized);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetIsLooping);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_SetIsLooping);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_GetIsPlaying);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_Play);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_PlayOneShot);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_Restart);
		VX_REGISTER_INTERNAL_CALL(AudioSourceComponent_Stop);

		VX_REGISTER_INTERNAL_CALL(AudioClip_GetName);
		VX_REGISTER_INTERNAL_CALL(AudioClip_GetLength);

		VX_REGISTER_INTERNAL_CALL(AudioCone_GetInnerAngle);
		VX_REGISTER_INTERNAL_CALL(AudioCone_SetInnerAngle);
		VX_REGISTER_INTERNAL_CALL(AudioCone_GetOuterAngle);
		VX_REGISTER_INTERNAL_CALL(AudioCone_SetOuterAngle);
		VX_REGISTER_INTERNAL_CALL(AudioCone_GetOuterGain);
		VX_REGISTER_INTERNAL_CALL(AudioCone_SetOuterGain);
		
		VX_REGISTER_INTERNAL_CALL(Physics_Raycast);
		VX_REGISTER_INTERNAL_CALL(Physics_GetSceneGravity);
		VX_REGISTER_INTERNAL_CALL(Physics_SetSceneGravity);
		VX_REGISTER_INTERNAL_CALL(Physics_GetScenePositionIterations);
		VX_REGISTER_INTERNAL_CALL(Physics_SetScenePositionIterations);
		VX_REGISTER_INTERNAL_CALL(Physics_GetSceneVelocityIterations);
		VX_REGISTER_INTERNAL_CALL(Physics_SetSceneVelocityIterations);

		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_GetStaticFriction);
		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_SetStaticFriction);
		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_GetDynamicFriction);
		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_SetDynamicFriction);
		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_GetBounciness);
		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_SetBounciness);
		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_GetFrictionCombineMode);
		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_SetFrictionCombineMode);
		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_GetBouncinessCombineMode);
		VX_REGISTER_INTERNAL_CALL(PhysicsMaterial_SetBouncinessCombineMode);

		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetBodyType);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetBodyType);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetCollisionDetectionType);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetCollisionDetectionType);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetMass);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetMass);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetLinearVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetLinearVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetMaxLinearVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetMaxLinearVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetLinearDrag);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetLinearDrag);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetAngularVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetAngularVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetMaxAngularVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetMaxAngularVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetAngularDrag);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetAngularDrag);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetDisableGravity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetDisableGravity);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetIsKinematic);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetIsKinematic);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetKinematicTargetTranslation);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetKinematicTargetTranslation);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetKinematicTargetRotation);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetKinematicTargetRotation);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_GetLockFlags);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_SetLockFlag);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_IsLockFlagSet);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_IsSleeping);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_WakeUp);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_AddForce);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_AddForceAtPosition);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_AddTorque);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_ClearTorque);
		VX_REGISTER_INTERNAL_CALL(RigidBodyComponent_ClearForce);

		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_Move);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_Jump); 
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_IsGrounded);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_GetFootPosition);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_GetSpeedDown);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_GetSlopeLimit);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_SetSlopeLimit);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_GetStepOffset);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_SetStepOffset);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_GetContactOffset);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_SetContactOffset);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_GetNonWalkableMode);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_SetNonWalkableMode);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_GetClimbMode);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_SetClimbMode);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_GetDisableGravity);
		VX_REGISTER_INTERNAL_CALL(CharacterControllerComponent_SetDisableGravity);

		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_GetConnectedEntity);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_SetConnectedEntity);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_GetBreakForce);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_SetBreakForce);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_GetBreakTorque);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_SetBreakTorque);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_SetBreakForceAndTorque);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_GetEnableCollision);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_SetCollisionEnabled);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_GetPreProcessingEnabled);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_SetPreProcessingEnabled);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_IsBroken);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_GetIsBreakable);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_SetIsBreakable);
		VX_REGISTER_INTERNAL_CALL(FixedJointComponent_Break);

		VX_REGISTER_INTERNAL_CALL(BoxColliderComponent_GetHalfSize);
		VX_REGISTER_INTERNAL_CALL(BoxColliderComponent_SetHalfSize);
		VX_REGISTER_INTERNAL_CALL(BoxColliderComponent_GetOffset);
		VX_REGISTER_INTERNAL_CALL(BoxColliderComponent_SetOffset);
		VX_REGISTER_INTERNAL_CALL(BoxColliderComponent_GetIsTrigger);
		VX_REGISTER_INTERNAL_CALL(BoxColliderComponent_SetIsTrigger);
		VX_REGISTER_INTERNAL_CALL(BoxColliderComponent_GetMaterialHandle);

		VX_REGISTER_INTERNAL_CALL(SphereColliderComponent_GetRadius);
		VX_REGISTER_INTERNAL_CALL(SphereColliderComponent_SetRadius);
		VX_REGISTER_INTERNAL_CALL(SphereColliderComponent_GetOffset);
		VX_REGISTER_INTERNAL_CALL(SphereColliderComponent_SetOffset);
		VX_REGISTER_INTERNAL_CALL(SphereColliderComponent_GetIsTrigger);
		VX_REGISTER_INTERNAL_CALL(SphereColliderComponent_SetIsTrigger);
		VX_REGISTER_INTERNAL_CALL(SphereColliderComponent_GetMaterialHandle);

		VX_REGISTER_INTERNAL_CALL(CapsuleColliderComponent_GetRadius);
		VX_REGISTER_INTERNAL_CALL(CapsuleColliderComponent_SetRadius);
		VX_REGISTER_INTERNAL_CALL(CapsuleColliderComponent_GetHeight);
		VX_REGISTER_INTERNAL_CALL(CapsuleColliderComponent_SetHeight);
		VX_REGISTER_INTERNAL_CALL(CapsuleColliderComponent_GetOffset);
		VX_REGISTER_INTERNAL_CALL(CapsuleColliderComponent_SetOffset);
		VX_REGISTER_INTERNAL_CALL(CapsuleColliderComponent_GetIsTrigger);
		VX_REGISTER_INTERNAL_CALL(CapsuleColliderComponent_SetIsTrigger);
		VX_REGISTER_INTERNAL_CALL(CapsuleColliderComponent_GetMaterialHandle);

		VX_REGISTER_INTERNAL_CALL(MeshColliderComponent_IsStaticMesh);
		VX_REGISTER_INTERNAL_CALL(MeshColliderComponent_IsColliderMeshValid);
		VX_REGISTER_INTERNAL_CALL(MeshColliderComponent_GetColliderMesh);
		VX_REGISTER_INTERNAL_CALL(MeshColliderComponent_SetColliderMesh);
		VX_REGISTER_INTERNAL_CALL(MeshColliderComponent_GetIsTrigger);
		VX_REGISTER_INTERNAL_CALL(MeshColliderComponent_SetIsTrigger);
		VX_REGISTER_INTERNAL_CALL(MeshColliderComponent_GetMaterialHandle);

		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetBodyType);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_SetBodyType);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_SetVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetDrag);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_SetDrag);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetAngularVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_SetAngularVelocity);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetAngularDrag);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_SetAngularDrag);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetFixedRotation);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_SetFixedRotation);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetGravityScale);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_SetGravityScale);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_ApplyForce);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_ApplyForceToCenter);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		VX_REGISTER_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulseToCenter);

		VX_REGISTER_INTERNAL_CALL(Physics2D_Raycast);
		VX_REGISTER_INTERNAL_CALL(Physics2D_GetWorldGravity);
		VX_REGISTER_INTERNAL_CALL(Physics2D_SetWorldGravity);
		VX_REGISTER_INTERNAL_CALL(Physics2D_GetWorldPositionIterations);
		VX_REGISTER_INTERNAL_CALL(Physics2D_SetWorldPositionIterations);
		VX_REGISTER_INTERNAL_CALL(Physics2D_GetWorldVelocityIterations);
		VX_REGISTER_INTERNAL_CALL(Physics2D_SetWorldVelocityIterations);

		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_GetOffset);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_SetOffset);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_GetSize);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_SetSize);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_GetDensity);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_SetDensity);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_GetFriction);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_SetFriction);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_GetRestitution);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_SetRestitution);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_GetRestitutionThreshold);
		VX_REGISTER_INTERNAL_CALL(BoxCollider2DComponent_SetRestitutionThreshold);

		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_GetOffset);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_SetOffset);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_GetRadius);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_SetRadius);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_GetDensity);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_SetDensity);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_GetFriction);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_SetFriction);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_GetRestitution);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_SetRestitution);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_GetRestitutionThreshold);
		VX_REGISTER_INTERNAL_CALL(CircleCollider2DComponent_SetRestitutionThreshold);

		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetVelocity);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetVelocity);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetVelocityVariation);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetVelocityVariation);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetOffset);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetOffset);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetSizeBegin);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetSizeBegin);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetSizeEnd);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetSizeEnd);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetSizeVariation);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetSizeVariation);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetColorBegin);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetColorBegin);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetColorEnd);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetColorEnd);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetRotation);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetRotation);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_GetLifeTime);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_SetLifeTime);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_Start);
		VX_REGISTER_INTERNAL_CALL(ParticleEmitterComponent_Stop);

		VX_REGISTER_INTERNAL_CALL(Texture2D_LoadFromPath);
		VX_REGISTER_INTERNAL_CALL(Texture2D_Constructor);
		VX_REGISTER_INTERNAL_CALL(Texture2D_GetWidth);
		VX_REGISTER_INTERNAL_CALL(Texture2D_GetHeight);
		VX_REGISTER_INTERNAL_CALL(Texture2D_SetPixel);

		VX_REGISTER_INTERNAL_CALL(Random_RangedInt32);
		VX_REGISTER_INTERNAL_CALL(Random_RangedFloat);
		VX_REGISTER_INTERNAL_CALL(Random_Float);

		VX_REGISTER_INTERNAL_CALL(Quaternion_Inverse);

		VX_REGISTER_INTERNAL_CALL(Matrix4_Rotate);
		VX_REGISTER_INTERNAL_CALL(Matrix4_LookAt);
		VX_REGISTER_INTERNAL_CALL(Matrix4_Multiply);

		VX_REGISTER_INTERNAL_CALL(Mathf_GetPI);
		VX_REGISTER_INTERNAL_CALL(Mathf_GetPI_D);
		VX_REGISTER_INTERNAL_CALL(Mathf_Round);
		VX_REGISTER_INTERNAL_CALL(Mathf_Abs);
		VX_REGISTER_INTERNAL_CALL(Mathf_Sqrt);
		VX_REGISTER_INTERNAL_CALL(Mathf_Pow);
		VX_REGISTER_INTERNAL_CALL(Mathf_Sin);
		VX_REGISTER_INTERNAL_CALL(Mathf_Cos);
		VX_REGISTER_INTERNAL_CALL(Mathf_Acos);
		VX_REGISTER_INTERNAL_CALL(Mathf_Tan);
		VX_REGISTER_INTERNAL_CALL(Mathf_Max);
		VX_REGISTER_INTERNAL_CALL(Mathf_Max);
		VX_REGISTER_INTERNAL_CALL(Mathf_Deg2Rad);
		VX_REGISTER_INTERNAL_CALL(Mathf_Rad2Deg);
		VX_REGISTER_INTERNAL_CALL(Mathf_Deg2RadVector3);
		VX_REGISTER_INTERNAL_CALL(Mathf_Rad2DegVector3);
		VX_REGISTER_INTERNAL_CALL(Mathf_LookAt);
		VX_REGISTER_INTERNAL_CALL(Mathf_InverseQuat);

		VX_REGISTER_INTERNAL_CALL(Noise_Constructor);
		VX_REGISTER_INTERNAL_CALL(Noise_Destructor);
		VX_REGISTER_INTERNAL_CALL(Noise_GetFrequency);
		VX_REGISTER_INTERNAL_CALL(Noise_SetFrequency);
		VX_REGISTER_INTERNAL_CALL(Noise_GetFractalOctaves);
		VX_REGISTER_INTERNAL_CALL(Noise_SetFractalOctaves);
		VX_REGISTER_INTERNAL_CALL(Noise_GetFractalLacunarity);
		VX_REGISTER_INTERNAL_CALL(Noise_SetFractalLacunarity);
		VX_REGISTER_INTERNAL_CALL(Noise_GetFractalGain);
		VX_REGISTER_INTERNAL_CALL(Noise_SetFractalGain);
		VX_REGISTER_INTERNAL_CALL(Noise_GetVec2);
		VX_REGISTER_INTERNAL_CALL(Noise_GetVec3);
		VX_REGISTER_INTERNAL_CALL(Noise_SetSeed);
		VX_REGISTER_INTERNAL_CALL(Noise_PerlinNoiseVec2);
		VX_REGISTER_INTERNAL_CALL(Noise_PerlinNoiseVec3);

		VX_REGISTER_INTERNAL_CALL(Time_GetElapsed);
		VX_REGISTER_INTERNAL_CALL(Time_GetDeltaTime);

		VX_REGISTER_INTERNAL_CALL(Input_IsKeyPressed);
		VX_REGISTER_INTERNAL_CALL(Input_IsKeyReleased);
		VX_REGISTER_INTERNAL_CALL(Input_IsKeyDown);
		VX_REGISTER_INTERNAL_CALL(Input_IsKeyUp);
		VX_REGISTER_INTERNAL_CALL(Input_IsMouseButtonPressed);
		VX_REGISTER_INTERNAL_CALL(Input_IsMouseButtonReleased);
		VX_REGISTER_INTERNAL_CALL(Input_IsMouseButtonDown);
		VX_REGISTER_INTERNAL_CALL(Input_IsMouseButtonUp);
		VX_REGISTER_INTERNAL_CALL(Input_GetMousePosition);
		VX_REGISTER_INTERNAL_CALL(Input_SetMousePosition);
		VX_REGISTER_INTERNAL_CALL(Input_GetMouseWheelMovement);
		VX_REGISTER_INTERNAL_CALL(Input_IsGamepadButtonDown);
		VX_REGISTER_INTERNAL_CALL(Input_IsGamepadButtonUp);
		VX_REGISTER_INTERNAL_CALL(Input_GetGamepadAxis);
		VX_REGISTER_INTERNAL_CALL(Input_GetCursorMode);
		VX_REGISTER_INTERNAL_CALL(Input_SetCursorMode);

		VX_REGISTER_INTERNAL_CALL(Gui_Begin);
		VX_REGISTER_INTERNAL_CALL(Gui_BeginWithPosition);
		VX_REGISTER_INTERNAL_CALL(Gui_BeginWithSize);
		VX_REGISTER_INTERNAL_CALL(Gui_BeginWithPositionAndSize);
		VX_REGISTER_INTERNAL_CALL(Gui_End);
		VX_REGISTER_INTERNAL_CALL(Gui_Underline);
		VX_REGISTER_INTERNAL_CALL(Gui_Spacing);
		VX_REGISTER_INTERNAL_CALL(Gui_Text);
		VX_REGISTER_INTERNAL_CALL(Gui_Button);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyBool);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyInt);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyULong);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyFloat);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyDouble);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyVec2);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyVec3);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyVec4);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyColor3);
		VX_REGISTER_INTERNAL_CALL(Gui_PropertyColor4);

		VX_REGISTER_INTERNAL_CALL(Log_Message);
	}

}
