#include "vxpch.h"
#include "ScriptRegistry.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Core/Log.h"

#include "Vortex/Input/Input.h"

#include "Vortex/Math/Noise.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/ScriptClass.h"
#include "Vortex/Scripting/ScriptInstance.h"
#include "Vortex/Scripting/ScriptUtils.h"
#include "Vortex/Scripting/ManagedArray.h"
#include "Vortex/Scripting/ManagedString.h"

#include "Vortex/Audio/Audio.h"
#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Audio/AudioTypes.h"
#include "Vortex/Audio/AudioUtils.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/3D/PhysicsScene.h"
#include "Vortex/Physics/3D/PhysicsMaterial.h"
#include "Vortex/Physics/3D/PhysicsShapes.h"
#include "Vortex/Physics/3D/PhysicsTypes.h"
#include "Vortex/Physics/3D/PhysicsUtils.h"

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

#include "Vortex/Serialization/PlayerPrefsSerializer.h"

#include "Vortex/Utils/Random.h"
#include "Vortex/Utils/Time.h"

#include "Vortex/Editor/UI/UI.h"

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

#define VX_SCRIPT_CORE_NAMESPACE "Vortex"
#define VX_DEFAULT_INTERNAL_CALL_CLASSNAME "InternalCalls"
#define VX_REGISTER_INTERNAL_CALL(coreNamespace, icClassname, bindFnName, bindFn) mono_add_internal_call(fmt::format("{}.{}::{}", (coreNamespace), (icClassname), (bindFnName)).c_str(), bindFn)
#define VX_REGISTER_DEFAULT_INTERNAL_CALL(icall) VX_REGISTER_INTERNAL_CALL(VX_SCRIPT_CORE_NAMESPACE, VX_DEFAULT_INTERNAL_CALL_CLASSNAME, VX_STRINGIFY(icall), InternalCalls::icall)

	struct ScriptRegistryInternalData
	{
		std::unordered_map<MonoType*, std::function<void(Actor)>> ActorAddComponentFuncs;
		std::unordered_map<MonoType*, std::function<bool(Actor)>> ActorHasComponentFuncs;
		std::unordered_map<MonoType*, std::function<void(Actor)>> ActorRemoveComponentFuncs;

		float SceneStartTime = 0.0f;

		PlayerPrefsSerializer Serializer;
		const Fs::Path PlayerPrefsFilename = "PlayerPrefs.prefs";
	};

	static ScriptRegistryInternalData s_Data;

	namespace InternalCalls {

		static Scene* GetContextScene()
		{
			Scene* contextScene = ScriptEngine::GetContextScene();
			VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

			return contextScene;
		}

		static Actor GetActor(UUID actorUUID)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = contextScene->TryGetActorWithUUID(actorUUID);
			VX_CORE_ASSERT(actor, "Invalid Actor UUID!");

			return actor;
		}

#pragma region Application

		void Application_Quit()
		{
			Scene* contextScene = GetContextScene();

			Application::Get().Close();
		}

#pragma endregion

#pragma region Window

		void Window_GetSize(Math::vec2* outSize)
		{
			Scene* contextScene = GetContextScene();

			const Window& window = Application::Get().GetWindow();
			*outSize = window.GetSize();
		}

		void Window_GetPosition(Math::vec2* outPosition)
		{
			Scene* contextScene = GetContextScene();

			const Window& window = Application::Get().GetWindow();
			*outPosition = window.GetPosition();
		}

		bool Window_IsMaximized()
		{
			Scene* contextScene = GetContextScene();

			const Window& window = Application::Get().GetWindow();
			return window.IsMaximized();
		}

		void Window_SetMaximized(bool maximized)
		{
			Scene* contextScene = GetContextScene();

			Window& window = Application::Get().GetWindow();
			window.SetMaximized(maximized);
		}

		bool Window_IsResizeable()
		{
			Scene* contextScene = GetContextScene();

			const Window& window = Application::Get().GetWindow();
			return window.IsResizeable();
		}

		void Window_SetResizeable(bool resizeable)
		{
			Scene* contextScene = GetContextScene();

			Window& window = Application::Get().GetWindow();
			window.SetResizeable(resizeable);
		}

		bool Window_IsDecorated()
		{
			Scene* contextScene = GetContextScene();

			const Window& window = Application::Get().GetWindow();
			return window.IsDecorated();
		}

		void Window_SetDecorated(bool decorated)
		{
			Scene* contextScene = GetContextScene();

			Window& window = Application::Get().GetWindow();
			window.SetDecorated(decorated);
		}

		bool Window_IsVSyncEnabled()
		{
			Scene* contextScene = GetContextScene();

			const Window& window = Application::Get().GetWindow();
			return window.IsVSyncEnabled();
		}

		void Window_SetVSync(bool use)
		{
			Scene* contextScene = GetContextScene();

			Window& window = Application::Get().GetWindow();
			window.SetVSync(use);
		}

#pragma endregion

#pragma region SceneRenderer

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

		void DebugRenderer_DrawLine(const Math::vec3* startPoint, const Math::vec3* endPoint, const Math::vec4* color)
		{
			Scene* contextScene = GetContextScene();
			Actor primaryCameraActor = contextScene->GetPrimaryCameraActor();

			if (!primaryCameraActor)
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Calling DebugRenderer.DrawLine without a primary camera! Attach a camera component to an actor and enable 'Primary'");
				return;
			}

			Renderer2D::DrawLine(*startPoint, *endPoint, *color);
		}

		void DebugRenderer_SetLineWidth(float width)
		{
			Scene* contextScene = GetContextScene();
			Actor primaryCameraActor = contextScene->GetPrimaryCameraActor();

			if (!primaryCameraActor)
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Calling DebugRenderer.SetLineWidth without a primary camera! Attach a camera component to an actor and enable 'Primary'");
				return;
			}

			Renderer2D::SetLineWidth(width);
		}

		void DebugRenderer_DrawQuadBillboard(const Math::vec3* translation, const Math::vec2* size, const Math::vec4* color)
		{
			Scene* contextScene = GetContextScene();
			Actor primaryCameraActor = contextScene->GetPrimaryCameraActor();

			if (!primaryCameraActor)
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Calling DebugRenderer.DrawQuadBillboard without a primary camera! Attach a camera component to an actor and enable 'Primary'");
				return;
			}

			const Math::mat4 transform = contextScene->GetWorldSpaceTransformMatrix(primaryCameraActor);
			const Math::mat4 view = Math::Inverse(transform);

			Renderer2D::DrawQuadBillboard(view, *translation, *size, *color);
		}

		void DebugRenderer_DrawCircleVec2(const Math::vec2* translation, const Math::vec2* size, const Math::vec4* color, float thickness, float fade)
		{
			Scene* contextScene = GetContextScene();
			Actor primaryCameraActor = contextScene->GetPrimaryCameraActor();

			if (!primaryCameraActor)
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Calling DebugRenderer.DrawCircle without a primary camera! Attach a camera component to an actor and enable 'Primary'");
				return;
			}

			Renderer2D::DrawCircle(*translation, *size, 0.0f, *color, thickness, fade);
		}

		void DebugRenderer_DrawCircleVec3(const Math::vec3* translation, const Math::vec3* size, const Math::vec4* color, float thickness, float fade)
		{
			Scene* contextScene = GetContextScene();
			Actor primaryCameraActor = contextScene->GetPrimaryCameraActor();

			if (!primaryCameraActor)
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Calling DebugRenderer.DrawCircle without a primary camera! Attach a camera component to an actor and enable 'Primary'");
				return;
			}

			Renderer2D::DrawCircle(*translation, *size, 0.0f, *color, thickness, fade);
		}

		void DebugRenderer_DrawBoundingBox(const Math::vec3* worldPosition, const Math::vec3* size, const Math::vec4* color)
		{
			Scene* contextScene = GetContextScene();
			Actor primaryCameraActor = contextScene->GetPrimaryCameraActor();

			if (!primaryCameraActor)
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Calling DebugRenderer.DrawBoundingBox without a primary camera! Attach a camera component to an actor and enable 'Primary'");
				return;
			}

			const Math::AABB aabb{
				-Math::vec3(0.5f),
				+Math::vec3(0.5f),
			};

			const Math::mat4 transform = Math::Identity() * Math::Translate(*worldPosition) * Math::Scale(*size);
			Renderer2D::DrawAABB(aabb, transform, *color);
		}

		void DebugRenderer_DrawBoundingBoxFromTransform(UUID actorUUID, const Math::vec4* color)
		{
			Scene* contextScene = GetContextScene();
			Actor primaryCameraActor = contextScene->GetPrimaryCameraActor();

			if (!primaryCameraActor)
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Calling DebugRenderer.DrawBoundingBox without a primary camera! Attach a camera component to an actor and enable 'Primary'");
				return;
			}

			Actor actor = GetActor(actorUUID);

			const Math::AABB aabb{
				-Math::vec3(0.5f),
				+Math::vec3(0.5f),
			};

			const Math::mat4 worldSpaceTransform = contextScene->GetWorldSpaceTransformMatrix(actor);

			Renderer2D::DrawAABB(aabb, worldSpaceTransform, *color);
		}

		void DebugRenderer_Flush()
		{
			Scene* contextScene = GetContextScene();
			Actor primaryCameraActor = contextScene->GetPrimaryCameraActor();

			if (!primaryCameraActor)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling DebugRenderer.Flush without a primary camera! Attach a camera component to an actor and enable 'Primary'");
				return;
			}

			const CameraComponent& cameraComponent = primaryCameraActor.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			const Math::mat4 transform = contextScene->GetWorldSpaceTransformMatrix(primaryCameraActor);
			const Math::mat4 view = Math::Inverse(transform);

			Renderer2D::EndScene();
			Renderer2D::BeginScene(camera, view);
		}

#pragma endregion

#pragma region Scene

		uint64_t Scene_GetPrimaryCamera()
		{
			Scene* contextScene = GetContextScene();

			Actor primaryCamera = contextScene->GetPrimaryCameraActor();

			if (!primaryCamera)
				return 0;

			return primaryCamera.GetUUID();
		}

		bool Scene_FindActorByID(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			return (bool)actor;
		}

		uint64_t Scene_FindActorByName(MonoString* name)
		{
			ManagedString mstring(name);

			Scene* contextScene = GetContextScene();
			Actor actor = contextScene->FindActorByName(mstring.String());

			if (!actor)
				return 0;

			return actor.GetUUID();
		}

		uint64_t Scene_FindChildByName(UUID actorUUID, MonoString* childName)
		{
			ManagedString mstring(childName);

			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			const auto& children = actor.Children();

			for (const auto& child : children)
			{
				Actor childActor = contextScene->TryGetActorWithUUID(child);

				if (childActor && childActor.GetName() == mstring.String())
				{
					return childActor.GetUUID();
				}
			}

			return 0;
		}

		uint64_t Scene_CreateActor(MonoString* name)
		{
			ManagedString mstring(name);

			Scene* contextScene = GetContextScene();
			Actor actor = contextScene->CreateActor(mstring.String());

			return actor.GetUUID();
		}

		uint64_t Scene_Instantiate(UUID actorUUID)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor)
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Scene.Instantiate called with Invalid Actor UUID!");
				return 0;
			}

			Actor duplicate = contextScene->DuplicateActor(actor);

			return duplicate.GetUUID();
		}

		uint64_t Scene_InstantiateAsChild(UUID actorUUID, UUID parentUUID)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);
			Actor parent = GetActor(parentUUID);

			if (!actor || !parent)
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Scene.Instantiate called with Invalid Actor UUID!");
				return 0;
			}

			Actor duplicate = contextScene->DuplicateActor(actor);

			contextScene->ParentActor(duplicate, parent);

			return duplicate.GetUUID();
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

#pragma endregion

#pragma region SceneManager

		void SceneManager_LoadScene(MonoString* sceneName)
		{
			Scene* contextScene = GetContextScene();

			ManagedString mstring(sceneName);

			// TODO
		}

#pragma endregion

#pragma region Actor

		void Actor_AddComponent(UUID actorUUID, MonoReflectionType* componentType)
		{
			Actor actor = GetActor(actorUUID);

			MonoType* managedType = mono_reflection_type_get_type(componentType);
			VX_CORE_ASSERT(s_Data.ActorAddComponentFuncs.find(managedType) != s_Data.ActorAddComponentFuncs.end(), "Managed type was not found in Map!");

			s_Data.ActorAddComponentFuncs.at(managedType)(actor);
		}

		bool Actor_HasComponent(UUID actorUUID, MonoReflectionType* componentType)
		{
			Actor actor = GetActor(actorUUID);

			MonoType* managedType = mono_reflection_type_get_type(componentType);
			VX_CORE_ASSERT(s_Data.ActorHasComponentFuncs.find(managedType) != s_Data.ActorHasComponentFuncs.end(), "Managed type was not found in Map!");

			return s_Data.ActorHasComponentFuncs.at(managedType)(actor);
		}

		void Actor_RemoveComponent(UUID actorUUID, MonoReflectionType* componentType)
		{
			Actor actor = GetActor(actorUUID);

			MonoType* managedType = mono_reflection_type_get_type(componentType);
			VX_CORE_ASSERT(s_Data.ActorRemoveComponentFuncs.find(managedType) != s_Data.ActorRemoveComponentFuncs.end(), "Managed type was not found in Map!");

			s_Data.ActorRemoveComponentFuncs.at(managedType)(actor);
		}

		MonoArray* Actor_GetChildren(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			const std::vector<UUID>& children = actor.Children();

			MonoClass* coreActorClass = ScriptEngine::GetCoreActorClass()->GetMonoClass();
			VX_CORE_ASSERT(coreActorClass, "Core Actor Class was Invalid!");

			ManagedArray managedArray(coreActorClass, children.size());
			managedArray.FillFromVector(children);
			return managedArray.GetHandle();
		}

		uint64_t Actor_GetChild(UUID actorUUID, uint32_t index)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			const std::vector<UUID>& children = actor.Children();
			if (index > (children.size() - 1))
			{
				VX_CORE_ASSERT(false, "Index out of bounds!");
				return 0;
			}

			uint64_t childUUID = children[index];
			Actor child = contextScene->TryGetActorWithUUID(childUUID);
			VX_CORE_ASSERT(child, "Child UUID was Invalid!");

			return child.GetUUID();
		}

		MonoString* Actor_GetTag(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			ManagedString mstring(actor.GetName());

			return mstring.GetAddressOf();
		}

		void Actor_SetTag(UUID actorUUID, MonoString* tag)
		{
			Actor actor = GetActor(actorUUID);

			TagComponent& tagComponent = actor.GetComponent<TagComponent>();

			ManagedString mstring(tag);
			tagComponent.Tag = std::string(mstring.String());
		}

		MonoString* Actor_GetMarker(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			ManagedString mstring(actor.GetMarker());

			return mstring.GetAddressOf();
		}

		void Actor_SetMarker(UUID actorUUID, MonoString* marker)
		{
			Actor actor = GetActor(actorUUID);

			TagComponent& tagComponent = actor.GetComponent<TagComponent>();

			ManagedString mstring(marker);
			tagComponent.Marker = mstring.String();
		}

		bool Actor_AddChild(UUID parentUUID, UUID childUUID)
		{
			Scene* contextScene = GetContextScene();
			Actor parent = GetActor(parentUUID);
			Actor child = GetActor(childUUID);

			if (parent && child)
			{
				contextScene->ParentActor(child, parent);
				return true;
			}

			return false;
		}

		bool Actor_RemoveChild(UUID parentUUID, UUID childUUID)
		{
			Scene* contextScene = GetContextScene();
			Actor parent = GetActor(parentUUID);
			Actor child = GetActor(childUUID);

			if (parent && child)
			{
				contextScene->UnparentActor(child);
				return true;
			}

			return false;
		}

		MonoObject* Actor_GetScriptInstance(UUID actorUUID)
		{
			Scene* contextScene = GetContextScene();
			return ScriptEngine::TryGetManagedInstance(actorUUID);
		}

		void Actor_Destroy(UUID actorUUID, bool excludeChildren)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Actor.Destroy with invalid actor!");
				return;
			}

			contextScene->SubmitToDestroyActor(actor, excludeChildren);
		}

		void Actor_DestroyWithDelay(UUID actorUUID, float delay, bool excludeChildren)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			auto onFinishedFn = [=]() { contextScene->SubmitToDestroyActor(actor, excludeChildren); };
			Timer timer(actor.GetName() + std::to_string(actorUUID), delay, onFinishedFn);
			timer.Start();

			contextScene->EmplaceOrReplaceTimer(actor, std::move(timer));
		}

		void Actor_Invoke(UUID actorUUID, MonoString* methodName)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			VX_CORE_VERIFY(contextScene);

			if (!actor.HasComponent<ScriptComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Actor.Invoke without a script component!");
				return;
			}

			const ScriptComponent& scriptComponent = actor.GetComponent<ScriptComponent>();
			const std::string& className = scriptComponent.ClassName;

			if (!ScriptEngine::ScriptClassExists(className))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Actor.Invoke with an invalid script class!");
				return;
			}

			SharedReference<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(className);

			ManagedString mstring(methodName);

			const int paramCount = 0;
			MonoMethod* method = scriptClass->GetMethod(mstring.String(), paramCount);

			if (method == nullptr)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Actor.Invoke with an invalid method name '{}'", mstring.String());
				return;
			}

			SharedReference<ScriptInstance> instance = ScriptEngine::GetScriptInstance(actor.GetUUID());
			if (instance == nullptr)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Actor.Invoke with invalid script instance!");
				return;
			}

			MonoObject* scriptInstance = instance->GetManagedObject();
			if (scriptInstance == nullptr)
			{
				return;
			}

			ScriptUtils::InvokeMethod(scriptInstance, method, nullptr);
		}

		void Actor_InvokeWithDelay(UUID actorUUID, MonoString* methodName, float delay)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			auto onTimerFinishedFn = [=]() { Actor_Invoke(actorUUID, methodName); };
			Timer timer("InvokeWithDelay", delay, onTimerFinishedFn);
			timer.Start();

			scene->EmplaceOrReplaceTimer(actor, std::move(timer));
		}

		bool Actor_IsActive(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Actor.ActiveInHierarchy with invalid actor!");
				return false;
			}

			return actor.IsActive();
		}

		void Actor_SetActive(UUID actorUUID, bool isActive)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Actor.SetActive with invalid actor!");
				return;
			}

			actor.SetActive(isActive);
		}

		void Actor_AddTimer(UUID actorUUID, MonoString* name, float delay)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			ManagedString mstring(name);

			Timer timer(mstring.String(), delay, nullptr);
			scene->EmplaceOrReplaceTimer(actor, std::move(timer));
		}

		bool Actor_IsValid(UUID actorUUID)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			return bool(actor);
		}

#pragma endregion

#pragma region AssetHandle

		bool AssetHandle_IsValid(AssetHandle* assetHandle)
		{
			return AssetManager::IsHandleValid(*assetHandle);
		}

#pragma endregion

#pragma region Timer
		
		float Timer_GetTimeLeft(UUID actorUUID, MonoString* name)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			ManagedString mstring(name);

			Timer& timer = scene->TryGetMutableTimerByName(actor, mstring.String());

			if (timer.GetName().empty())
			{
				// invalid timer
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access invalid timer '{}' - '{}'", actor.GetName(), mstring.String());
				return 0.0f;
			}

			return timer.GetTimeLeft();
		}

		bool Timer_IsStarted(UUID actorUUID, MonoString* name)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			ManagedString mstring(name);

			Timer& timer = scene->TryGetMutableTimerByName(actor, mstring.String());

			if (timer.GetName().empty())
			{
				// invalid timer
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access invalid timer '{}' - '{}'", actor.GetName(), mstring.String());
				return false;
			}

			return timer.IsStarted();
		}

		bool Timer_IsFinished(UUID actorUUID, MonoString* name)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			ManagedString mstring(name);

			Timer& timer = scene->TryGetMutableTimerByName(actor, mstring.String());

			if (timer.GetName().empty())
			{
				// invalid timer
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access invalid timer '{}' - '{}'", actor.GetName(), mstring.String());
				return false;
			}

			return timer.IsFinished();
		}

		void Timer_Start(UUID actorUUID, MonoString* name)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			ManagedString mstring(name);

			Timer& timer = scene->TryGetMutableTimerByName(actor, mstring.String());

			if (timer.GetName().empty())
			{
				// invalid timer
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access invalid timer '{}' - '{}'", actor.GetName(), mstring.String());
			}

			timer.Start();
		}

#pragma endregion

#pragma region Transform Component

		void TransformComponent_GetTranslation(UUID actorUUID, Math::vec3* outTranslation)
		{
			Actor actor = GetActor(actorUUID);

			std::string actorName = actor.GetName();

			if (actor.HasComponent<RigidBodyComponent>() && actor.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				if (actor.HasComponent<CharacterControllerComponent>())
				{
					physx::PxController* controller = Physics::GetController(actorUUID);
					*outTranslation = PhysicsUtils::FromPhysXExtendedVector(controller->getPosition());

					return;
				}

				physx::PxRigidDynamic* actor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
				Math::vec3 translation = PhysicsUtils::FromPhysXVector(actor->getGlobalPose().p);

				*outTranslation = translation;

				return;
			}
			else if (actor.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					const auto& position = body->GetPosition();
					*outTranslation = Math::vec3(position.x, position.y, actor.GetTransform().Translation.z);

					return;
				}
			}

			*outTranslation = actor.GetTransform().Translation;
		}

		void TransformComponent_SetTranslation(UUID actorUUID, Math::vec3* translation)
		{
			Actor actor = GetActor(actorUUID);

			if (actor.HasComponent<RigidBodyComponent>() && actor.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				if (actor.HasComponent<CharacterControllerComponent>())
				{
					physx::PxController* controller = Physics::GetController(actorUUID);
					controller->setPosition(PhysicsUtils::ToPhysXExtendedVector(*translation));

					return;
				}

				physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();

				const auto& transformComponent = actor.GetTransform();
				physx::PxTransform physxTransform = pxActor->getGlobalPose();
				physxTransform.p = PhysicsUtils::ToPhysXVector(*translation);

				pxActor->setGlobalPose(physxTransform);

				return;
			}
			else if (actor.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();
				
				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					body->SetTransform({ translation->x, translation->y }, body->GetAngle());
					actor.GetTransform().Translation.z = translation->z;
					
					return;
				}
			}

			actor.GetTransform().Translation = *translation;
		}

		void TransformComponent_GetRotation(UUID actorUUID, Math::quaternion* outRotation)
		{
			Actor actor = GetActor(actorUUID);

			if (actor.HasComponent<RigidBodyComponent>() && actor.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* actor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
				physx::PxTransform physxTranform = actor->getGlobalPose();
				Math::quaternion rotation = PhysicsUtils::FromPhysXQuat(physxTranform.q);

				*outRotation = rotation;

				return;
			}
			else if (actor.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					const float angleRad = body->GetAngle();
					Math::vec3 currentEulers = actor.GetTransform().GetRotationEuler();
					Math::vec3 eulers(currentEulers.x, angleRad, currentEulers.z);
					*outRotation = Math::quaternion(eulers);

					return;
				}
			}

			*outRotation = actor.GetTransform().GetRotation();
		}

		void TransformComponent_SetRotation(UUID actorUUID, Math::quaternion* rotation)
		{
			Actor actor = GetActor(actorUUID);

			if (actor.HasComponent<RigidBodyComponent>() && actor.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* actor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
				physx::PxTransform physxTransform = actor->getGlobalPose();
				physxTransform.q = PhysicsUtils::ToPhysXQuat(*rotation);

				actor->setGlobalPose(physxTransform);
			}
			else if (actor.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					Math::vec3 translation = actor.GetTransform().Translation;
					Math::vec3 eulerAngles = Math::EulerAngles(*rotation);
					body->SetTransform({ translation.x, translation.y }, eulerAngles.z);

					return;
				}
			}

			actor.GetTransform().SetRotation(*rotation);
		}

		void TransformComponent_GetEulerAngles(UUID actorUUID, Math::vec3* outEulerAngles)
		{
			Actor actor = GetActor(actorUUID);

			if (actor.HasComponent<RigidBodyComponent>() && actor.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* actor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
				Math::quaternion orientation = PhysicsUtils::FromPhysXQuat(actor->getGlobalPose().q);

				*outEulerAngles = Math::EulerAngles(orientation);

				// Since we store rotation in radians we must convert to degrees here
				*outEulerAngles = Math::Rad2Deg(*outEulerAngles);

				return;
			}
			else if (actor.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					const auto& transform = actor.GetTransform();
					*outEulerAngles = { transform.GetRotationEuler().x, body->GetAngle(), transform.GetRotationEuler().z };

					// Since we store rotation in radians we must convert to degrees here
					*outEulerAngles = Math::Rad2Deg(*outEulerAngles);

					return;
				}
			}
			
			*outEulerAngles = actor.GetTransform().GetRotationEuler();

			// Since we store rotation in radians we must convert to degrees here
			*outEulerAngles = Math::Rad2Deg(*outEulerAngles);
		}

		void TransformComponent_SetEulerAngles(UUID actorUUID, Math::vec3* eulerAngles)
		{
			Actor actor = GetActor(actorUUID);

			// Since we store rotation in radians we must convert to radians here
			*eulerAngles = Math::Deg2Rad(*eulerAngles);

			if (actor.HasComponent<RigidBodyComponent>() && actor.GetComponent<RigidBodyComponent>().Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* actor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
				physx::PxTransform physxTransform = actor->getGlobalPose();
				physxTransform.q = PhysicsUtils::ToPhysXQuat(Math::quaternion(*eulerAngles));

				actor->setGlobalPose(physxTransform);

				return;
			}
			else if (actor.HasComponent<RigidBody2DComponent>())
			{
				const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

				if (rigidbody.Type == RigidBody2DType::Dynamic)
				{
					b2Body* body = (b2Body*)rigidbody.RuntimeBody;

					body->SetTransform(body->GetPosition(), Math::Deg2Rad(eulerAngles->z));
					actor.GetTransform().SetRotationEuler(*eulerAngles);

					return;
				}
			}

			actor.GetTransform().SetRotationEuler(*eulerAngles);
		}

		void TransformComponent_Rotate(UUID actorUUID, Math::vec3* eulers, Space relativeTo)
		{
			Actor actor = GetActor(actorUUID);

			VX_CORE_ASSERT(relativeTo == Space::Local, "World Space Rotations have not been implemented yet!");

			if (relativeTo == Space::Local)
			{
				Math::quaternion rotation;
				TransformComponent_GetRotation(actorUUID, &rotation);

				*eulers = Math::Deg2Rad(*eulers);

				rotation *= Math::AngleAxis(eulers->x, Math::vec3(1.0f, 0.0f, 0.0f));
				rotation *= Math::AngleAxis(eulers->y, Math::vec3(0.0f, 1.0f, 0.0f));
				rotation *= Math::AngleAxis(eulers->z, Math::vec3(0.0f, 0.0f, 1.0f));

				TransformComponent_SetRotation(actorUUID, &rotation);
			}
			else if (relativeTo == Space::World)
			{
				
			}
		}

		void TransformComponent_RotateAround(UUID actorUUID, Math::vec3* worldPoint, Math::vec3* axis, float angle)
		{
			Actor actor = GetActor(actorUUID);

			const float angleRad = Math::Deg2Rad(angle);
			TransformComponent worldSpaceTransform = GetContextScene()->GetWorldSpaceTransform(actor);
			Math::mat4 worldSpaceTransformMatrix = worldSpaceTransform.GetTransform();
			const Math::vec3 point = *worldPoint;
			const Math::vec3 worldSpaceTranslation = worldSpaceTransform.Translation;
			const Math::vec3 normalizedAxis = *axis;

			Math::mat4 transform;

			transform = worldSpaceTransformMatrix
				* Math::Translate(point)
				* Math::Rotate(angleRad, normalizedAxis)
				* Math::Translate(-point);

			actor.SetTransform(transform);
		}

		void TransformComponent_SetTranslationAndRotation(UUID actorUUID, Math::vec3* translation, Math::vec3* eulers)
		{
			TransformComponent_SetTranslation(actorUUID, translation);
			Math::quaternion rotation(*eulers);
			TransformComponent_SetRotation(actorUUID, &rotation);
		}

		void TransformComponent_GetScale(UUID actorUUID, Math::vec3* outScale)
		{
			Actor actor = GetActor(actorUUID);

			*outScale = actor.GetTransform().Scale;
		}

		void TransformComponent_SetScale(UUID actorUUID, Math::vec3* scale)
		{
			Actor actor = GetActor(actorUUID);

			actor.GetTransform().Scale = *scale;
		}

		void TransformComponent_GetWorldSpaceTransform(UUID actorUUID, Math::vec3* outTranslation, Math::quaternion* outRotation, Math::vec3* outEulers, Math::vec3* outScale)
		{
			Actor actor = GetActor(actorUUID);

			TransformComponent worldSpaceTransform = GetContextScene()->GetWorldSpaceTransform(actor);
			*outTranslation = worldSpaceTransform.Translation;
			*outRotation = worldSpaceTransform.GetRotation();
			*outEulers = worldSpaceTransform.GetRotationEuler();
			*outScale = worldSpaceTransform.Scale;
		}

		void TransformComponent_GetTransformMatrix(UUID actorUUID, Math::mat4* outTransform)
		{
			Actor actor = GetActor(actorUUID);

			const TransformComponent& transform = actor.GetTransform();

			*outTransform = transform.GetTransform();
		}

		void TransformComponent_SetTransformMatrix(UUID actorUUID, Math::mat4* transform)
		{
			Actor actor = GetActor(actorUUID);

			TransformComponent& actorTransform = actor.GetTransform();

			actorTransform.SetTransform(*transform);
		}

		void TransformComponent_GetForwardDirection(UUID actorUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			TransformComponent worldSpaceTransform = contextScene->GetWorldSpaceTransform(actor);

			*outDirection = worldSpaceTransform.CalculateForward();
		}

		void TransformComponent_GetBackwardDirection(UUID actorUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			TransformComponent worldSpaceTransform = contextScene->GetWorldSpaceTransform(actor);

			*outDirection = worldSpaceTransform.CalculateBackward();
		}

		void TransformComponent_GetUpDirection(UUID actorUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			TransformComponent worldSpaceTransform = contextScene->GetWorldSpaceTransform(actor);

			*outDirection = worldSpaceTransform.CalculateUp();
		}

		void TransformComponent_GetDownDirection(UUID actorUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			TransformComponent worldSpaceTransform = contextScene->GetWorldSpaceTransform(actor);

			*outDirection = worldSpaceTransform.CalculateDown();
		}

		void TransformComponent_GetRightDirection(UUID actorUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			TransformComponent worldSpaceTransform = contextScene->GetWorldSpaceTransform(actor);

			*outDirection = worldSpaceTransform.CalculateRight();
		}

		void TransformComponent_GetLeftDirection(UUID actorUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			TransformComponent worldSpaceTransform = contextScene->GetWorldSpaceTransform(actor);

			*outDirection = worldSpaceTransform.CalculateLeft();
		}

		void TransformComponent_LookAt(UUID actorUUID, Math::vec3* worldPoint)
		{
			Actor actor = GetActor(actorUUID);

			if (actor.HasComponent<RigidBodyComponent>())
			{
				const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

				if (rigidbody.Type == RigidBodyType::Dynamic)
				{
					physx::PxRigidDynamic* actor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
					physx::PxTransform physxTransform = actor->getGlobalPose();

					const Math::vec3 upDirection(0.0f, 1.0f, 0.0f);
					Math::mat4 result = Math::LookAt(PhysicsUtils::FromPhysXVector(physxTransform.p), *worldPoint, upDirection);
					Math::vec3 translation, scale;
					Math::quaternion rotation;
					Math::vec3 skew;
					Math::vec4 perspective;
					Math::Decompose(Math::Inverse(result), scale, rotation, translation, skew, perspective);
					physxTransform.q = PhysicsUtils::ToPhysXQuat(rotation);

					actor->setGlobalPose(physxTransform);

					return;
				}
			}

			TransformComponent& transform = actor.GetTransform();
			Math::vec3 upDirection(0.0f, 1.0f, 0.0f);
			Math::mat4 result = Math::LookAt(transform.Translation, *worldPoint, upDirection);
			Math::vec3 translation, scale;
			Math::quaternion rotation;
			Math::vec3 skew;
			Math::vec4 perspective;
			Math::Decompose(Math::Inverse(result), scale, rotation, translation, skew, perspective);
			transform.SetRotation(rotation);
		}

		uint64_t TransformComponent_GetParent(UUID actorUUID)
		{
			Actor child = GetActor(actorUUID);

			if (!child.HasParent())
			{
				VX_CONSOLE_LOG_WARN("[Script Engine] Trying to access Transform.Parent with invalid actor!");
				return 0;
			}

			Actor parent = GetActor(child.GetParentUUID());

			return parent.GetUUID();
		}

		void TransformComponent_SetParent(UUID childUUID, UUID parentUUID)
		{
			Scene* contextScene = GetContextScene();
			Actor child = GetActor(childUUID);
			Actor parent = GetActor(parentUUID);
			
			if (!parent || !child)
			{
				return;
			}

			contextScene->ParentActor(child, parent);
		}

		void TransformComponent_Unparent(UUID actorUUID)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			contextScene->UnparentActor(actor);
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

		SceneCamera::ProjectionType CameraComponent_GetProjectionType(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.ProjectionType without a Camera!");
				return SceneCamera::ProjectionType::Perspective;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetProjectionType();
		}

		void CameraComponent_SetProjectionType(UUID actorUUID, SceneCamera::ProjectionType type)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.ProjectionType without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			const bool consistentProjectionType = camera.GetProjectionType() == type;

			if (consistentProjectionType)
				return;

			camera.SetProjectionType(type);
			const Math::uvec2& viewportSize = scene->GetViewportSize();
			camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}

		void CameraComponent_GetPrimary(UUID actorUUID, bool* outPrimary)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.IsPrimary without a Camera!");
				return;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();

			*outPrimary = cameraComponent.Primary;
		}

		void CameraComponent_SetPrimary(UUID actorUUID, bool primary)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.IsPrimary without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();

			cameraComponent.Primary = primary;
		}

		float CameraComponent_GetPerspectiveVerticalFOV(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.FieldOfView without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();

			return Math::Rad2Deg(cameraComponent.Camera.GetPerspectiveVerticalFOVRad());
		}

		void CameraComponent_SetPerspectiveVerticalFOV(UUID actorUUID, float perspectiveVerticalFOV)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.FieldOfView without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			const float FOVRad = Math::Deg2Rad(perspectiveVerticalFOV);

			camera.SetPerspectiveVerticalFOVRad(FOVRad);
			const Math::uvec2& viewportSize = scene->GetViewportSize();
			camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}

		float CameraComponent_GetNearClip(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.NearClip without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetPerspectiveNearClip();
		}

		void CameraComponent_SetNearClip(UUID actorUUID, float nearClip)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.NearClip without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetPerspectiveNearClip(nearClip);
			const Math::uvec2& viewportSize = scene->GetViewportSize();
			camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}

		float CameraComponent_GetFarClip(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.FarClip without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetPerspectiveFarClip();
		}

		void CameraComponent_SetFarClip(UUID actorUUID, float farClip)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.FarClip without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetPerspectiveFarClip(farClip);
			const Math::uvec2& viewportSize = scene->GetViewportSize();
			camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}

		float CameraComponent_GetOrthographicSize(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.OrthographicSize without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetOrthographicSize();
		}

		void CameraComponent_SetOrthographicSize(UUID actorUUID, float orthographicSize)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.OrthographicSize without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetOrthographicSize(orthographicSize);
			const Math::uvec2& viewportSize = scene->GetViewportSize();
			camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}

		float CameraComponent_GetOrthographicNear(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.OrthographicNear without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetOrthographicNearClip();
		}

		void CameraComponent_SetOrthographicNear(UUID actorUUID, float orthographicNear)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.OrthographicNear without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetOrthographicNearClip(orthographicNear);
			const Math::uvec2& viewportSize = scene->GetViewportSize();
			camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}

		float CameraComponent_GetOrthographicFar(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.OrthographicFar without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			return camera.GetOrthographicFarClip();
		}

		void CameraComponent_SetOrthographicFar(UUID actorUUID, float orthographicFar)
		{
			Scene* scene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.OrthographicFar without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			SceneCamera& camera = cameraComponent.Camera;

			camera.SetOrthographicFarClip(orthographicFar);
			const Math::uvec2& viewportSize = scene->GetViewportSize();
			camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}

		void CameraComponent_GetFixedAspectRatio(UUID actorUUID, bool* outFixedAspectRatio)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.IsFixedAspectRatio without a Camera!");
				return;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();

			*outFixedAspectRatio = cameraComponent.FixedAspectRatio;
		}

		void CameraComponent_SetFixedAspectRatio(UUID actorUUID, bool fixedAspectRatio)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.IsFixedAspectRatio without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();

			cameraComponent.FixedAspectRatio = fixedAspectRatio;
		}

		void CameraComponent_GetClearColor(UUID actorUUID, Math::vec3* outColor)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Camera.ClearColor without a Camera!");
				return;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			
			*outColor = cameraComponent.ClearColor;
		}

		void CameraComponent_SetClearColor(UUID actorUUID, Math::vec3* color)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set Camera.ClearColor without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			
			cameraComponent.ClearColor = *color;
		}

		void CameraComponent_Raycast(UUID actorUUID, Math::vec3* position, float maxDistance, Math::Ray* outRay)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Camera.CastRay without a Camera!");
				return;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& sceneCamera = cameraComponent.Camera;

			const Math::mat4 transform = contextScene->GetWorldSpaceTransformMatrix(actor);
			const Math::mat4 view = Math::Inverse(transform);

			*outRay = sceneCamera.Raycast(*position, actor.GetTransform().Translation, maxDistance, view);
		}

		void CameraComponent_ScreenToWorldPoint(UUID actorUUID, Math::vec2* position, float maxDistance, Math::vec3* outWorldPoint)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Camera.ScreenToWorldPoint without a Camera!");
				return;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& sceneCamera = cameraComponent.Camera;

			const Math::mat4 transform = contextScene->GetWorldSpaceTransformMatrix(actor);
			const Math::mat4 view = Math::Inverse(transform);

			const ViewportBounds& viewportBounds = contextScene->GetViewportBounds();
			*outWorldPoint = sceneCamera.ScreenPointToWorldPoint(*position, viewportBounds.MinBound, actor.GetTransform().Translation, maxDistance, view);
		}

		void CameraComponent_ScreenToViewportPoint(UUID actorUUID, Math::vec2* position, Math::vec2* outViewportPoint)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Camera.ScreenToViewportPoint without a Camera!");
				return;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& sceneCamera = cameraComponent.Camera;

			*outViewportPoint = sceneCamera.ScreenPointToViewportPoint(*position);
		}

#pragma endregion

#pragma region PostProcessInfo

		bool PostProcessInfo_GetEnabled(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access PostProcessInfo.Enabled without a Camera!");
				return false;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			return cameraComponent.PostProcessing.Enabled;
		}

		void PostProcessInfo_SetEnabled(UUID actorUUID, bool enabled)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set PostProcessInfo.Enabled without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			cameraComponent.PostProcessing.Enabled = enabled;
        }

#pragma endregion

#pragma region BloomInfo

		float BloomInfo_GetThreshold(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access BloomInfo.Threshold without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			return cameraComponent.PostProcessing.Bloom.Threshold;
		}

		void BloomInfo_SetThreshold(UUID actorUUID, float threshold)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set BloomInfo.Threshold without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			cameraComponent.PostProcessing.Bloom.Threshold = threshold;
		}

		float BloomInfo_GetKnee(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access BloomInfo.Knee without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			return cameraComponent.PostProcessing.Bloom.Knee;
		}

		void BloomInfo_SetKnee(UUID actorUUID, float knee)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set BloomInfo.Knee without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			cameraComponent.PostProcessing.Bloom.Knee = knee;
		}

		float BloomInfo_GetIntensity(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access BloomInfo.Intensity without a Camera!");
				return 0.0f;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			return cameraComponent.PostProcessing.Bloom.Intensity;
		}

		void BloomInfo_SetIntensity(UUID actorUUID, float intensity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set BloomInfo.Intensity without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			cameraComponent.PostProcessing.Bloom.Intensity = intensity;
		}

		bool BloomInfo_GetEnabled(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access BloomInfo.Enabled without a Camera!");
				return false;
			}

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			return cameraComponent.PostProcessing.Bloom.Enabled;
		}

		void BloomInfo_SetEnabled(UUID actorUUID, bool enabled)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CameraComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set BloomInfo.Enabled without a Camera!");
				return;
			}

			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			cameraComponent.PostProcessing.Bloom.Enabled = enabled;
		}

#pragma endregion

#pragma region Light Source Component

		LightType LightSourceComponent_GetLightType(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access LightSource.LightType without a Light Source!");
				return LightType::Directional;
			}

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			return lightSourceComponent.Type;
		}

		void LightSourceComponent_SetLightType(UUID actorUUID, LightType type)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set LightSource.LightType without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			lightSourceComponent.Type = type;
		}

		void LightSourceComponent_GetRadiance(UUID actorUUID, Math::vec3* outRadiance)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access LightSource.Radiance without a Light Source!");
				return;
			}

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			*outRadiance = lightSourceComponent.Radiance;
		}

		void LightSourceComponent_SetRadiance(UUID actorUUID, Math::vec3* radiance)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set LightSource.Radiance without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			lightSourceComponent.Radiance = *radiance;
		}

		float LightSourceComponent_GetIntensity(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access LightSource.Intensity without a Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			return lightSourceComponent.Intensity;
		}

		void LightSourceComponent_SetIntensity(UUID actorUUID, float intensity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set LightSource.Intensity without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			lightSourceComponent.Intensity = intensity;
		}

		float LightSourceComponent_GetCutoff(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access LightSource.Cutoff without a Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			return lightSourceComponent.Cutoff;
		}

		void LightSourceComponent_SetCutoff(UUID actorUUID, float cutoff)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set LightSource.Cutoff without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			lightSourceComponent.Cutoff = cutoff;
		}

		float LightSourceComponent_GetOuterCutoff(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access LightSource.OuterCutoff without a Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			return lightSourceComponent.OuterCutoff;
		}

		void LightSourceComponent_SetOuterCutoff(UUID actorUUID, float outerCutoff)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set LightSource.OuterCutoff without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			lightSourceComponent.OuterCutoff = outerCutoff;
		}

		float LightSourceComponent_GetShadowBias(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access LightSource.ShadowBias without a Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			return lightSourceComponent.ShadowBias;
		}

		void LightSourceComponent_SetShadowBias(UUID actorUUID, float shadowBias)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set LightSource.ShadowBias without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			lightSourceComponent.ShadowBias = shadowBias;
		}

		bool LightSourceComponent_GetCastShadows(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access LightSource.CastShadows without a Light Source!");
				return false;
			}

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			return lightSourceComponent.CastShadows;
		}

		void LightSourceComponent_SetCastShadows(UUID actorUUID, bool castShadows)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set LightSource.CastShadows without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			lightSourceComponent.CastShadows = castShadows;
		}

		bool LightSourceComponent_GetSoftShadows(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access LightSource.UseSoftShadows without a Light Source!");
				return false;
			}

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			return lightSourceComponent.SoftShadows;
		}

		void LightSourceComponent_SetSoftShadows(UUID actorUUID, bool softShadows)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set LightSource.UseSoftShadows without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			lightSourceComponent.SoftShadows = softShadows;
		}

		bool LightSourceComponent_IsVisible(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access LightSource.Visible without a Light Source!");
				return false;
			}

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			return lightSourceComponent.Visible;
		}

		void LightSourceComponent_SetVisible(UUID actorUUID, bool visible)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set LightSource.Visible without a Light Source!");
				return;
			}

			LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();
			lightSourceComponent.Visible = visible;
		}

#pragma endregion

#pragma region TextMesh Component

		MonoString* TextMeshComponent_GetTextString(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access TextMesh.Text without a Text Mesh!");
				ManagedString mstring("");
				return mstring.GetAddressOf();
			}

			const TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();

			ManagedString mstring(textMeshComponent.TextString);

			return mstring.GetAddressOf();
		}

		void TextMeshComponent_SetTextString(UUID actorUUID, MonoString* textString)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set TextMesh.Text without a Text Mesh!");
				return;
			}

			ManagedString mstring(textString);

			TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			textMeshComponent.TextString = mstring.String();
			textMeshComponent.TextHash = std::hash<std::string>()(mstring.String());
		}

		void TextMeshComponent_GetColor(UUID actorUUID, Math::vec4* outColor)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access TextMesh.Color without a Text Mesh!");
				return;
			}

			const TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			*outColor = textMeshComponent.Color;
		}

		void TextMeshComponent_SetColor(UUID actorUUID, Math::vec4* color)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set TextMesh.Color without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			textMeshComponent.Color = *color;
		}

		void TextMeshComponent_GetOutlineColor(UUID actorUUID, Math::vec4* outOutlineColor)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access TextMesh.OutlineColor without a Text Mesh!");
				return;
			}

			const TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			*outOutlineColor = textMeshComponent.BackgroundColor;
		}

		void TextMeshComponent_SetOutlineColor(UUID actorUUID, Math::vec4* outlineColor)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set TextMesh.OutlineColor without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			textMeshComponent.BackgroundColor = *outlineColor;
		}

		float TextMeshComponent_GetLineSpacing(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access TextMesh.LineSpacing without a Text Mesh!");
				return 0.0f;
			}

			const TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			return textMeshComponent.LineSpacing;
		}

		void TextMeshComponent_SetLineSpacing(UUID actorUUID, float lineSpacing)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set TextMesh.LineSpacing without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			textMeshComponent.LineSpacing = lineSpacing;
		}

		float TextMeshComponent_GetKerning(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access TextMesh.Kerning without a Text Mesh!");
				return 0.0f;
			}

			const TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			return textMeshComponent.Kerning;
		}

		void TextMeshComponent_SetKerning(UUID actorUUID, float kerning)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set TextMesh.Kerning without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			textMeshComponent.Kerning = kerning;
		}

		float TextMeshComponent_GetMaxWidth(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access TextMesh.MaxWidth without a Text Mesh!");
				return 0.0f;
			}

			const TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			return textMeshComponent.MaxWidth;
		}

		void TextMeshComponent_SetMaxWidth(UUID actorUUID, float maxWidth)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set TextMesh.MaxWidth without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			textMeshComponent.MaxWidth = maxWidth;
		}

		bool TextMeshComponent_IsVisible(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access TextMesh.Visible without a Text Mesh!");
				return false;
			}

			const TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			return textMeshComponent.Visible;
		}

		void TextMeshComponent_SetVisible(UUID actorUUID, bool visible)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<TextMeshComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set TextMesh.Visible without a Text Mesh!");
				return;
			}

			TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			textMeshComponent.Visible = visible;
		}

#pragma endregion

#pragma region Animator Component

		bool AnimatorComponent_IsPlaying(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AnimatorComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access Animator.IsPlaying without a Animator!");
				return false;
			}

			const AnimatorComponent& animatorComponent = actor.GetComponent<AnimatorComponent>();
			SharedRef<Animator> animator = animatorComponent.Animator;
			return animator->IsPlaying();
		}

		void AnimatorComponent_Play(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AnimatorComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Animator.Play without a Animator!");
				return;
			}

			const AnimatorComponent& animatorComponent = actor.GetComponent<AnimatorComponent>();
			SharedRef<Animator> animator = animatorComponent.Animator;

			if (!actor.HasComponent<AnimationComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Animator.Play without an Animation!");
				return;
			}

			const AnimationComponent& animationComponent = actor.GetComponent<AnimationComponent>();
			SharedRef<Animation> animation = animationComponent.Animation;

			if (!animation)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Animator.Play with Invalid Animator!");
				return;
			}

			animator->PlayAnimation();
		}

		void AnimatorComponent_Stop(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AnimatorComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Animator.Stop without a Animator!");
				return;
			}

			const AnimatorComponent& animatorComponent = actor.GetComponent<AnimatorComponent>();
			SharedRef<Animator> animator = animatorComponent.Animator;

			if (!animator)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Animator.Stop with Invalid Animator!");
				return;
			}

			animator->Stop();
		}

#pragma endregion

#pragma region Mesh Renderer Component

		bool MeshRendererComponent_GetMaterialHandle(uint32_t submeshIndex, UUID actorUUID, AssetHandle* outHandle)
		{
			VX_CORE_ASSERT(false, "Not implemented yet!");
			return false;
		}

		bool MeshRendererComponent_IsVisible(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<MeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access MeshRenderer.Visible without a Mesh Renderer!");
				return false;
			}

			const MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
			return meshRendererComponent.Visible;
		}

		void MeshRendererComponent_SetVisible(UUID actorUUID, bool visible)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<MeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set MeshRenderer.Visible without a Mesh Renderer!");
				return;
			}

			MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
			meshRendererComponent.Visible = visible;
		}

		bool MeshRendererComponent_GetCastShadows(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<MeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access MeshRenderer.CastShadows without a Mesh Renderer!");
				return false;
			}

			const MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
			return meshRendererComponent.CastShadows;
		}

		void MeshRendererComponent_SetCastShadows(UUID actorUUID, bool castShadows)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<MeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set MeshRenderer.CastShadows without a Mesh Renderer!");
				return;
			}

			MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
			meshRendererComponent.CastShadows = castShadows;
		}

#pragma endregion

#pragma region Static Mesh Renderer Component

		MeshType StaticMeshRendererComponent_GetMeshType(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access StaticMeshRenderer.MeshType without a Static Mesh Renderer!");
				return MeshType::Cube;
			}

			const StaticMeshRendererComponent& staticMeshRenderer = actor.GetComponent<StaticMeshRendererComponent>();

			return staticMeshRenderer.Type;
		}

		void StaticMeshRendererComponent_SetMeshType(UUID actorUUID, MeshType meshType)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set StaticMeshRenderer.MeshType without a Static Mesh Renderer!");
				return;
			}

			StaticMeshRendererComponent& staticMeshRenderer = actor.GetComponent<StaticMeshRendererComponent>();
			staticMeshRenderer.Type = meshType;

			staticMeshRenderer.StaticMesh = DefaultMesh::DefaultStaticMeshes[(uint32_t)meshType];

			if (AssetManager::IsHandleValid(staticMeshRenderer.StaticMesh) && staticMeshRenderer.Materials->Empty())
			{
				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshRenderer.StaticMesh);
				staticMesh->LoadMaterialTable(staticMeshRenderer.Materials);
			}
		}

		void StaticMeshRendererComponent_SetMaterialHandle(uint32_t submeshIndex, UUID actorUUID, AssetHandle* materialHandle)
		{
			if (!AssetManager::IsHandleValid(*materialHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set material handle with invalid material!");
				return;
			}

			Actor actor = GetActor(actorUUID);
			StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();
			if (!AssetManager::IsHandleValid(staticMeshRendererComponent.StaticMesh))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Cannot set material of invalid mesh asset!");
				return;
			}

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshRendererComponent.StaticMesh);
			if (!staticMesh)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Cannot set material of invalid mesh asset!");
			}

			if (!staticMesh->HasSubmesh(submeshIndex))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set submesh material with out of bounds index!");
				return;
			}

			SharedReference<MaterialTable> materialTable = staticMeshRendererComponent.Materials;

			materialTable->SetMaterial(submeshIndex, *materialHandle);
		}

		bool StaticMeshRendererComponent_IsVisible(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);
			
			if (!actor.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access StaticMeshRenderer.Visible without a Static Mesh Renderer!");
				return false;
			}

			const StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();
			return staticMeshRendererComponent.Visible;
		}

		void StaticMeshRendererComponent_SetVisible(UUID actorUUID, bool visible)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set StaticMeshRenderer.Visible without a Static Mesh Renderer!");
				return;
			}

			StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();
			staticMeshRendererComponent.Visible = visible;
		}

		bool StaticMeshRendererComponent_GetCastShadows(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access StaticMeshRenderer.CastShadows without a Static Mesh Renderer!");
				return false;
			}

			const StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();
			return staticMeshRendererComponent.CastShadows;
		}

		void StaticMeshRendererComponent_SetCastShadows(UUID actorUUID, bool castShadows)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set StaticMeshRenderer.CastShadows without a Static Mesh Renderer!");
				return;
			}

			StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();
			staticMeshRendererComponent.CastShadows = castShadows;
		}

		bool StaticMeshRendererComponent_GetMaterialHandle(uint32_t submeshIndex, UUID actorUUID, AssetHandle* outHandle)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<StaticMeshRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling StaticMeshRenderer.GetMaterialHandle without a Static Mesh Renderer!");
				return false;
			}

			const StaticMeshRendererComponent& staticMeshRenderer = actor.GetComponent<StaticMeshRendererComponent>();
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

        bool SpriteRendererComponent_GetTextureHandle(UUID actorUUID, AssetHandle* outHandle)
        {
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access SpriteRenderer.Texture without a Sprite Renderer!");
				return false;
			}

			const SpriteRendererComponent& spriteRenderer = actor.GetComponent<SpriteRendererComponent>();

			if (AssetManager::IsHandleValid(spriteRenderer.Texture))
			{
				*outHandle = spriteRenderer.Texture;
			}

			return true;
        }

        void SpriteRendererComponent_SetTextureHandle(UUID actorUUID, AssetHandle* textureHandle)
        {
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set SpriteRenderer.Texture without a Sprite Renderer!");
				return;
			}

			SpriteRendererComponent& spriteRenderer = actor.GetComponent<SpriteRendererComponent>();

			if (AssetManager::IsHandleValid(*textureHandle))
			{
				spriteRenderer.Texture = *textureHandle;
			}
        }

        void SpriteRendererComponent_GetColor(UUID actorUUID, Math::vec4* outColor)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access SpriteRenderer.Color without a Sprite Renderer!");
				return;
			}

			const SpriteRendererComponent& spriteRenderer = actor.GetComponent<SpriteRendererComponent>();
			*outColor = spriteRenderer.SpriteColor;
		}

		void SpriteRendererComponent_SetColor(UUID actorUUID, Math::vec4* color)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set SpriteRenderer.Color without a Sprite Renderer!");
				return;
			}

			SpriteRendererComponent& spriteRenderer = actor.GetComponent<SpriteRendererComponent>();
			spriteRenderer.SpriteColor = *color;
		}

		void SpriteRendererComponent_GetUV(UUID actorUUID, Math::vec2* outScale)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access SpriteRenderer.Scale without a Sprite Renderer!");
				return;
			}

			const SpriteRendererComponent& spriteRenderer = actor.GetComponent<SpriteRendererComponent>();
			*outScale = spriteRenderer.TextureUV;
		}

		void SpriteRendererComponent_SetUV(UUID actorUUID, Math::vec2* scale)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set SpriteRenderer.Scale without a Sprite Renderer!");
				return;
			}

			SpriteRendererComponent& spriteRenderer = actor.GetComponent<SpriteRendererComponent>();
			spriteRenderer.TextureUV = *scale;
		}

		bool SpriteRendererComponent_IsVisible(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access SpriteRenderer.Visible without a Sprite Renderer!");
				return false;
			}

			const SpriteRendererComponent& spriteRendererComponent = actor.GetComponent<SpriteRendererComponent>();
			return spriteRendererComponent.Visible;
		}

		void SpriteRendererComponent_SetVisible(UUID actorUUID, bool visible)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SpriteRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set SpriteRenderer.Visible without a Sprite Renderer!");
				return;
			}

			SpriteRendererComponent& spriteRendererComponent = actor.GetComponent<SpriteRendererComponent>();
			spriteRendererComponent.Visible = visible;
		}

#pragma endregion

#pragma region Circle Renderer Component

		void CircleRendererComponent_GetColor(UUID actorUUID, Math::vec4* outColor)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access CircleRenderer.Color without a Circle Renderer!");
				return;
			}

			const CircleRendererComponent& circleRenderer = actor.GetComponent<CircleRendererComponent>();
			*outColor = circleRenderer.Color;
		}

		void CircleRendererComponent_SetColor(UUID actorUUID, Math::vec4* color)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set CircleRenderer.Color without a Circle Renderer!");
				return;
			}

			CircleRendererComponent& circleRenderer = actor.GetComponent<CircleRendererComponent>();
			circleRenderer.Color = *color;
		}

		void CircleRendererComponent_GetThickness(UUID actorUUID, float* outThickness)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access CircleRenderer.Thickness without a Circle Renderer!");
				return;
			}

			const CircleRendererComponent& circleRenderer = actor.GetComponent<CircleRendererComponent>();
			*outThickness = circleRenderer.Thickness;
		}

		void CircleRendererComponent_SetThickness(UUID actorUUID, float thickness)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set CircleRenderer.Thickness without a Circle Renderer!");
				return;
			}

			CircleRendererComponent& circleRenderer = actor.GetComponent<CircleRendererComponent>();
			circleRenderer.Thickness = thickness;
		}

		void CircleRendererComponent_GetFade(UUID actorUUID, float* outFade)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access CircleRenderer.Fade without a Circle Renderer!");
				return;
			}

			const CircleRendererComponent& circleRenderer = actor.GetComponent<CircleRendererComponent>();
			*outFade = circleRenderer.Fade;
		}

		void CircleRendererComponent_SetFade(UUID actorUUID, float fade)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set CircleRenderer.Fade without a Circle Renderer!");
				return;
			}

			CircleRendererComponent& circleRenderer = actor.GetComponent<CircleRendererComponent>();
			circleRenderer.Fade = fade;
		}

		bool CircleRendererComponent_IsVisible(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access CircleRenderer.Visible without a Circle Renderer!");
				return false;
			}

			const CircleRendererComponent& circleRendererComponent = actor.GetComponent<CircleRendererComponent>();
			return circleRendererComponent.Visible;
		}

		void CircleRendererComponent_SetVisible(UUID actorUUID, bool visible)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleRendererComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set CircleRenderer.Visible without a Circle Renderer!");
				return;
			}

			CircleRendererComponent& circleRendererComponent = actor.GetComponent<CircleRendererComponent>();
			circleRendererComponent.Visible = visible;
		}

#pragma endregion

#pragma region Particle Emitter Component

		void ParticleEmitterComponent_GetVelocity(UUID actorUUID, Math::vec3* outVelocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.Velocity without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}
			
			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outVelocity = particleEmitter->GetProperties().Velocity;
		}

		void ParticleEmitterComponent_SetVelocity(UUID actorUUID, Math::vec3* velocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.Velocity without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}
			
			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().Velocity = *velocity;
		}

		void ParticleEmitterComponent_GetVelocityVariation(UUID actorUUID, Math::vec3* outVelocityVariation)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.VelocityVariation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.VelocityVariation with invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outVelocityVariation = particleEmitter->GetProperties().VelocityVariation;
		}

		void ParticleEmitterComponent_SetVelocityVariation(UUID actorUUID, Math::vec3* velocityVariation)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.VelocityVariation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().VelocityVariation = *velocityVariation;
		}

		void ParticleEmitterComponent_GetOffset(UUID actorUUID, Math::vec3* outOffset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.Offset without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outOffset = particleEmitter->GetProperties().Offset;
		}

		void ParticleEmitterComponent_SetOffset(UUID actorUUID, Math::vec3* offset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.Offset without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().Offset = *offset;
		}

		void ParticleEmitterComponent_GetSizeBegin(UUID actorUUID, Math::vec2* outSizeBegin)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.SizeBegin without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.SizeBegin with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outSizeBegin = particleEmitter->GetProperties().SizeBegin;
		}

		void ParticleEmitterComponent_SetSizeBegin(UUID actorUUID, Math::vec2* sizeBegin)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.SizeBegin without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.SizeBegin with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().SizeBegin = *sizeBegin;
		}

		void ParticleEmitterComponent_GetSizeEnd(UUID actorUUID, Math::vec2* outSizeEnd)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.SizeEnd without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.SizeEnd with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outSizeEnd = particleEmitter->GetProperties().SizeEnd;
		}

		void ParticleEmitterComponent_SetSizeEnd(UUID actorUUID, Math::vec2* sizeEnd)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.SizeEnd without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.SizeEnd with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().SizeEnd = *sizeEnd;
		}

		void ParticleEmitterComponent_GetSizeVariation(UUID actorUUID, Math::vec2* outSizeVariation)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.SizeVariation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.SizeVariation with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outSizeVariation = particleEmitter->GetProperties().SizeVariation;
		}

		void ParticleEmitterComponent_SetSizeVariation(UUID actorUUID, Math::vec2* sizeVariation)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.SizeVariation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.SizeVariation with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().SizeVariation = *sizeVariation;
		}

		void ParticleEmitterComponent_GetColorBegin(UUID actorUUID, Math::vec4* outColorBegin)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.ColorBegin without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.ColorBegin with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outColorBegin = particleEmitter->GetProperties().ColorBegin;
		}

		void ParticleEmitterComponent_SetColorBegin(UUID actorUUID, Math::vec4* colorBegin)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.ColorBegin without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.ColorBegin with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().ColorBegin = *colorBegin;
		}

		void ParticleEmitterComponent_GetColorEnd(UUID actorUUID, Math::vec4* outColorEnd)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.ColorEnd without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.ColorEnd with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outColorEnd = particleEmitter->GetProperties().ColorEnd;
		}

		void ParticleEmitterComponent_SetColorEnd(UUID actorUUID, Math::vec4* colorEnd)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.ColorEnd without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.ColorEnd with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().ColorEnd = *colorEnd;
		}

		void ParticleEmitterComponent_GetRotation(UUID actorUUID, float* outRotation)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.Rotation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.Rotation with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outRotation = particleEmitter->GetProperties().Rotation;
		}

		void ParticleEmitterComponent_SetRotation(UUID actorUUID, float colorEnd)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.Rotation without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.Rotation with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().Rotation = colorEnd;
		}

		void ParticleEmitterComponent_GetLifeTime(UUID actorUUID, float* outLifeTime)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.LifeTime without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.LifeTime with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			*outLifeTime = particleEmitter->GetProperties().LifeTime;
		}

		void ParticleEmitterComponent_SetLifeTime(UUID actorUUID, float lifetime)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.LifeTime without a Particle Emitter!");
				return;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set ParticleEmitter.LifeTime with an invalid asset handle!");
				return;
			}

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				return;

			particleEmitter->GetProperties().LifeTime = lifetime;
		}

		void ParticleEmitterComponent_Start(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling ParticleEmitter.Start without a Particle Emitter!");
				return;
			}

			ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling ParticleEmitter.Start with an invalid asset handle!");
				return;
			}

			pmc.IsActive = true;
		}

		void ParticleEmitterComponent_Stop(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling ParticleEmitter.Stop without a Particle Emitter!");
				return;
			}

			ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Calling ParticleEmitter.Stop with an invalid asset handle!");
				return;
			}

			pmc.IsActive = false;
		}

		bool ParticleEmitterComponent_IsActive(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<ParticleEmitterComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.IsActive without a Particle Emitter!");
				return false;
			}

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access ParticleEmitter.IsActive with an invalid asset handle!");
				return false;
			}

			return pmc.IsActive;
		}

#pragma endregion

#pragma region AudioSource Component

		void AudioSourceComponent_GetPosition(UUID actorUUID, Math::vec3* outPosition)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.Position without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.Position with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			*outPosition = Utils::FromWaveVector(audioSource->GetPlaybackDevice().GetSound().GetPosition());
		}

		void AudioSourceComponent_SetPosition(UUID actorUUID, Math::vec3* position)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.Position without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.Position with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetPosition(Utils::ToWaveVector(*position));
		}

		void AudioSourceComponent_GetDirection(UUID actorUUID, Math::vec3* outDirection)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.Direction without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.Direction with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			*outDirection = Utils::FromWaveVector(audioSource->GetPlaybackDevice().GetSound().GetDirection());
		}

		void AudioSourceComponent_SetDirection(UUID actorUUID, Math::vec3* direction)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.Direction without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.Direction with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetDirection(Utils::ToWaveVector(*direction));
		}

		void AudioSourceComponent_GetVelocity(UUID actorUUID, Math::vec3* outVelocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.Velocity without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			*outVelocity = Utils::FromWaveVector(audioSource->GetPlaybackDevice().GetSound().GetVelocity());
		}

		void AudioSourceComponent_SetVelocity(UUID actorUUID, Math::vec3* velocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.Velocity without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.Velocity with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetVelocity(Utils::ToWaveVector(*velocity));
		}

		float AudioSourceComponent_GetMinGain(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.MinGain without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.MinGain with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetMinGain();
		}

		void AudioSourceComponent_SetMinGain(UUID actorUUID, float minGain)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.MinGain without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.MinGain with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetMinGain(minGain);
		}

		float AudioSourceComponent_GetMaxGain(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.MaxGain without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.MaxGain with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetMaxGain();
		}

		void AudioSourceComponent_SetMaxGain(UUID actorUUID, float maxGain)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.MaxGain without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.MaxGain with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetMaxGain(maxGain);
		}

		float AudioSourceComponent_GetDirectionalAttenuationFactor(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.DirectionalAttenuationFactor without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.DirectionalAttenuationFactor with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetDirectionalAttenuationFactor();
		}

		void AudioSourceComponent_SetDirectionalAttenuationFactor(UUID actorUUID, float factor)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.DirectionalAttenuationFactor without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.DirectionalAttenuationFactor with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetDirectionalAttenuationFactor(factor);
		}

		AttenuationModel AudioSourceComponent_GetAttenuationModel(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.AttenuationModel without a Audio Source!");
				return AttenuationModel::None;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.AttenuationModel with an invalid asset handle!");
				return AttenuationModel::None;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return AttenuationModel::None;

			return Utils::FromWaveAttenuationModel(audioSource->GetPlaybackDevice().GetSound().GetAttenuationModel());
		}

		void AudioSourceComponent_SetAttenuationModel(UUID actorUUID, AttenuationModel model)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.AttenuationModel without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.AttenuationModel with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetAttenuationModel(Utils::ToWaveAttenuationModel(model));
		}

		float AudioSourceComponent_GetPan(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.Pan without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioSource.Pan with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			return audioSource->GetPlaybackDevice().GetSound().GetPan();
		}

		void AudioSourceComponent_SetPan(UUID actorUUID, float pan)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.Pan without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to set AudioSource.Pan with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetPan(pan);
		}

		PanMode AudioSourceComponent_GetPanMode(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PanModel without a Audio Source!");
				return PanMode::Balance;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PanModel with an invalid asset handle!");
				return PanMode::Balance;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return PanMode::Balance;

			return Utils::FromWavePanMode(audioSource->GetPlaybackDevice().GetSound().GetPanMode());
		}

		void AudioSourceComponent_SetPanMode(UUID actorUUID, PanMode mode)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.PanModel without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.PanModel with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetPanMode(Utils::ToWavePanMode(mode));
		}

		PositioningMode AudioSourceComponent_GetPositioningMode(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PositioningModel without a Audio Source!");
				return PositioningMode::Absolute;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PositioningModel with an invalid asset handle!");
				return PositioningMode::Absolute;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return PositioningMode::Absolute;

			return Utils::FromWavePositioningMode(audioSource->GetPlaybackDevice().GetSound().GetPositioning());
		}

		void AudioSourceComponent_SetPositioningMode(UUID actorUUID, PositioningMode mode)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.PositioningModel without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.PositioningModel with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetPositioning(Utils::ToWavePositioningMode(mode));
		}

        float AudioSourceComponent_GetFalloff(UUID actorUUID)
        {
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Falloff without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetFalloff(UUID actorUUID, float falloff)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Falloff without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		float AudioSourceComponent_GetMinDistance(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MinDistance without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetMinDistance(UUID actorUUID, float minDistance)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MinDistance without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		float AudioSourceComponent_GetMaxDistance(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.MaxDistance without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetMaxDistance(UUID actorUUID, float maxDistance)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.MaxDistance without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		float AudioSourceComponent_GetPitch(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Pitch without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetPitch(UUID actorUUID, float pitch)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Pitch without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		float AudioSourceComponent_GetDopplerFactor(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.DopplerFactor without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetDopplerFactor(UUID actorUUID, float dopplerFactor)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.DopplerFactor without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		float AudioSourceComponent_GetVolume(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Volume without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetVolume(UUID actorUUID, float volume)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.Volume without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_GetDirectionToListener(UUID actorUUID, Math::vec3* outDirection)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.GetDirectionToListener without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.GetDirectionToListener with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			*outDirection = Utils::FromWaveVector(audioSource->GetPlaybackDevice().GetSound().GetDirectionToListener());
		}

		bool AudioSourceComponent_GetPlayOnStart(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PlayOnStart without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetPlayOnStart(UUID actorUUID, bool playOnStart)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.PlayOnStart without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		bool AudioSourceComponent_GetIsSpacialized(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsSpacialized without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetIsSpacialized(UUID actorUUID, bool spacialized)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.IsSpacialized without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		bool AudioSourceComponent_GetIsLooping(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsLooping without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetIsLooping(UUID actorUUID, bool loop)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioSource.IsLooping without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		bool AudioSourceComponent_GetIsPlaying(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsPlaying without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		bool AudioSourceComponent_GetIsPaused(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsPaused without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.IsPaused with an invalid asset handle!");
				return false;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return false;

			return audioSource->GetPlaybackDevice().GetSound().IsPaused();
		}

		uint64_t AudioSourceComponent_GetCursorInMilliseconds(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Cursor without a Audio Source!");
				return 0;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.Cursor with an invalid asset handle!");
				return 0;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0;

			float seconds = audioSource->GetPlaybackDevice().GetSound().GetCursorInSeconds();
			return (uint64_t)(seconds * 1000);
		}

		uint32_t AudioSourceComponent_GetPinnedListenerIndex(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PinnedListenerIndex without a Audio Source!");
				return 0;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PinnedListenerIndex with an invalid asset handle!");
				return 0;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0;

			return audioSource->GetPlaybackDevice().GetSound().GetPinnedListenerIndex();
		}

		void AudioSourceComponent_SetPinnedListenerIndex(UUID actorUUID, uint32_t listenerIndex)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PinnedListenerIndex without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioSource.PinnedListenerIndex with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetPinnedListenerIndex(listenerIndex);
		}

		void AudioSourceComponent_Play(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Play without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetStartTimeInMilliseconds(UUID actorUUID, uint64_t millis)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStartTimeInMilliseconds without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStartTimeInMilliseconds with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetStartTimeInMilliseconds(millis);
		}

		void AudioSourceComponent_SetStartTimeInPCMFrames(UUID actorUUID, uint64_t frames)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStartTimeInPCMFrames without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStartTimeInPCMFrames with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetStartTimeInPCMFrames(frames);
		}

		void AudioSourceComponent_SetFadeInMilliseconds(UUID actorUUID, float volumeStart, float volumeEnd, uint64_t lengthInMillis)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetFadeInMilliseconds without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetFadeInMilliseconds with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetFadeInMilliseconds(volumeStart, volumeEnd, lengthInMillis);
		}

		void AudioSourceComponent_SetFadeStartInMilliseconds(UUID actorUUID, float volumeStart, float volumeEnd, uint64_t lengthInMillis, uint64_t absoluteGlobalTime)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetFadeStartInMilliseconds without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetFadeStartInMilliseconds with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetFadeStartInMilliseconds(volumeStart, volumeEnd, lengthInMillis, absoluteGlobalTime);
		}

		void AudioSourceComponent_SetFadeInPCMFrames(UUID actorUUID, float volumeStart, float volumeEnd, uint64_t lengthInFrames)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetFadeInPCMFrames without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetFadeInPCMFrames with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetFadeInPCMFrames(volumeStart, volumeEnd, lengthInFrames);
		}

		void AudioSourceComponent_SetFadeStartInPCMFrames(UUID actorUUID, float volumeStart, float volumeEnd, uint64_t lengthInFrames, uint64_t absoluteGlobalTime)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetFadeInPCMFrames without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetFadeInPCMFrames with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetFadeStartInPCMFrames(volumeStart, volumeEnd, lengthInFrames, absoluteGlobalTime);
		}

		float AudioSourceComponent_GetCurrentFadeVolume(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.GetCurrentFadeVolume without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.GetCurrentFadeVolume with an invalid asset handle!");
				return 0.0f;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return 0.0f;

			audioSource->GetPlaybackDevice().GetSound().GetCurrentFadeVolume();
		}

		void AudioSourceComponent_PlayOneShot(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.PlayOneShot without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_Pause(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Pause without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Pause with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().Pause();
		}

		void AudioSourceComponent_Restart(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Restart without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_Stop(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Stop without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioSourceComponent_SetStopTimeInMilliseconds(UUID actorUUID, uint64_t millis)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStopTimeInMilliseconds without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStopTimeInMilliseconds with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetStopTimeInMilliseconds(millis);
		}

		void AudioSourceComponent_SetStopTimeInPCMFrames(UUID actorUUID, uint64_t frames)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStopTimeInPCMFrames without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStopTimeInPCMFrames with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetStopTimeInPCMFrames(frames);
		}

		void AudioSourceComponent_SetStopTimeWithFadeInMilliseconds(UUID actorUUID, uint64_t stopTimeInMillis, uint64_t fadeLengthInMillis)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStopTimeWithFadeInMilliseconds without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStopTimeWithFadeInMilliseconds with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetStopTimeWithFadeInMilliseconds(stopTimeInMillis, fadeLengthInMillis);
		}

		void AudioSourceComponent_SetStopTimeWithFadeInPCMFrames(UUID actorUUID, uint64_t stopTimeInFrames, uint64_t fadeLengthInFrames)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStopTimeWithFadeInPCMFrames without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.SetStopTimeWithFadeInPCMFrames with an invalid asset handle!");
				return;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return;

			audioSource->GetPlaybackDevice().GetSound().SetStopTimeWithFadeInPCMFrames(stopTimeInFrames, fadeLengthInFrames);
		}

		bool AudioSourceComponent_SeekToPCMFrame(UUID actorUUID, uint64_t frameIndex)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Seek without a Audio Source!");
				return false;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("Calling AudioSource.Seek with an invalid asset handle!");
				return false;
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				return false;

			return audioSource->GetPlaybackDevice().GetSound().SeekToPCMFrame(frameIndex);
		}

#pragma endregion

#pragma region Audio Clip

		MonoString* AudioClip_GetName(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioClip.Name without a Audio Source!");
				ManagedString mstring("");
				return mstring.GetAddressOf();
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioClip.Name with an invalid asset handle!");
				ManagedString mstring("");
				return mstring.GetAddressOf();
			}

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
			{
				VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to access AudioClip.Name with an invalid Audio Source!");
				ManagedString mstring("");
				return mstring.GetAddressOf();
			}

			/*const AudioClip& audioClip = audioSource->GetAudioClip();
			const std::string& clipName = audioClip.Name;

			ManagedString mstring(clipName);

			return mstring.GetManagedString();*/
		}

		float AudioClip_GetLength(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioClip.Length without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		float AudioCone_GetInnerAngle(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioCone.InnerAngle without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioCone_SetInnerAngle(UUID actorUUID, float innerAngle)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioCone.InnerAngle without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		float AudioCone_GetOuterAngle(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioCone.OuterAngle without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioCone_SetOuterAngle(UUID actorUUID, float outerAngle)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioCone.OuterAngle without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		float AudioCone_GetOuterGain(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access AudioCone.OuterGain without a Audio Source!");
				return 0.0f;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		void AudioCone_SetOuterGain(UUID actorUUID, float outerGain)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<AudioSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set AudioCone.OuterGain without a Audio Source!");
				return;
			}

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		RigidBodyType RigidBodyComponent_GetBodyType(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody.BodyType without a RigidBody!");
				return RigidBodyType::Static;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			return rigidbody.Type;
		}

		void RigidBodyComponent_SetBodyType(UUID actorUUID, RigidBodyType bodyType)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.BodyType without a RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			const bool consistentBodyType = bodyType == rigidbody.Type;

			if (consistentBodyType)
				return;

			rigidbody.Type = bodyType;
			Physics::ReCreateActor(actor);
		}

		CollisionDetectionType RigidBodyComponent_GetCollisionDetectionType(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody.CollisionDetection without a RigidBody!");
				return CollisionDetectionType::Discrete;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.CollisionDetection with a non-dynamic Rigidbody!");
				return CollisionDetectionType::Discrete;
			}

			return rigidbody.CollisionDetection;
		}

		void RigidBodyComponent_SetCollisionDetectionType(UUID actorUUID, CollisionDetectionType collisionDetectionType)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.CollisionDetection without a RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.CollisionDetectiom with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.CollisionDetection = collisionDetectionType;
		}

		float RigidBodyComponent_GetMass(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.Mass with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.Mass;
		}

		void RigidBodyComponent_SetMass(UUID actorUUID, float mass)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.Mass with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.Mass = mass;
		}

		void RigidBodyComponent_GetLinearVelocity(UUID actorUUID, Math::vec3* outVelocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.LinearVelocity with a non-dynamic Rigidbody!");
				return;
			}

			*outVelocity = rigidbody.LinearVelocity;
		}

		void RigidBodyComponent_SetLinearVelocity(UUID actorUUID, Math::vec3* velocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.LinearVelocity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.LinearVelocity = *velocity;
		}

		float RigidBodyComponent_GetMaxLinearVelocity(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.MaxLinearVelocity with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.MaxLinearVelocity;
		}

		void RigidBodyComponent_SetMaxLinearVelocity(UUID actorUUID, float maxLinearVelocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.MaxLinearVelocity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.MaxLinearVelocity = maxLinearVelocity;
		}

		float RigidBodyComponent_GetLinearDrag(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.LinearDrag with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.LinearDrag;
		}

		void RigidBodyComponent_SetLinearDrag(UUID actorUUID, float drag)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.LinearDrag with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.LinearDrag = drag;
		}

		void RigidBodyComponent_GetAngularVelocity(UUID actorUUID, Math::vec3* outVelocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.AngularVelocity with a non-dynamic Rigidbody!");
				return;
			}

			*outVelocity = rigidbody.AngularVelocity;
		}

		void RigidBodyComponent_SetAngularVelocity(UUID actorUUID, Math::vec3* velocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.AngularVelocity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.AngularVelocity = *velocity;
		}

		float RigidBodyComponent_GetMaxAngularVelocity(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.MaxAngularVelocity with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.MaxAngularVelocity;
		}

		void RigidBodyComponent_SetMaxAngularVelocity(UUID actorUUID, float maxAngularVelocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.MaxAngularVelocity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.MaxAngularVelocity = maxAngularVelocity;
		}

		float RigidBodyComponent_GetAngularDrag(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return 0.0f;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.AngularDrag with a non-dynamic Rigidbody!");
				return 0.0f;
			}

			return rigidbody.AngularDrag;
		}

		void RigidBodyComponent_SetAngularDrag(UUID actorUUID, float drag)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.AngluarDrag with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.AngularDrag = drag;
		}

		bool RigidBodyComponent_GetDisableGravity(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return false;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.DisableGravity with a non-dynamic Rigidbody!");
				return false;
			}

			return rigidbody.DisableGravity;
		}

		void RigidBodyComponent_SetDisableGravity(UUID actorUUID, bool disabled)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.DisableGravity with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.DisableGravity = disabled;
		}

		bool RigidBodyComponent_GetIsKinematic(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return false;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.IsKinematic with a non-dynamic Rigidbody!");
				return false;
			}

			return actor.GetComponent<RigidBodyComponent>().IsKinematic;
		}

		void RigidBodyComponent_SetIsKinematic(UUID actorUUID, bool isKinematic)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Trying to set RigidBody.IsKinematic with a non-dynamic Rigidbody!");
				return;
			}

			rigidbody.IsKinematic = isKinematic;
		}

		void RigidBodyComponent_GetKinematicTargetTranslation(UUID actorUUID, Math::vec3* outTranslation)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody.KinematicTarget without a Kinematic RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (!rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget with a non-kinematic RigidBody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			physx::PxTransform target;

			if (pxActor->getKinematicTarget(target))
			{
				*outTranslation = PhysicsUtils::FromPhysXVector(target.p);
			}

			*outTranslation = Math::vec3(0.0f);
		}

		void RigidBodyComponent_SetKinematicTargetTranslation(UUID actorUUID, Math::vec3* translation)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget without a Kinematic RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (!rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget with a non-kinematic RigidBody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			physx::PxTransform targetTransform;
			targetTransform.p = PhysicsUtils::ToPhysXVector(*translation);

			physx::PxTransform t;

			if (pxActor->getKinematicTarget(t))
			{
				targetTransform.q = t.q;
			}

			pxActor->setKinematicTarget(targetTransform);
		}

		void RigidBodyComponent_GetKinematicTargetRotation(UUID actorUUID, Math::quaternion* outRotation)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody.KinematicTarget without a Kinematic RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (!rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget with a non-kinematic RigidBody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			physx::PxTransform target;

			if (pxActor->getKinematicTarget(target))
			{
				*outRotation = PhysicsUtils::FromPhysXQuat(target.q);
			}

			*outRotation = Math::quaternion(1, 0, 0, 0);
		}

		void RigidBodyComponent_SetKinematicTargetRotation(UUID actorUUID, Math::quaternion* rotation)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget without a Kinematic RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (!rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody.KinematicTarget with a non-kinematic RigidBody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			physx::PxTransform targetTransform;
			targetTransform.q = PhysicsUtils::ToPhysXQuat(*rotation);

			physx::PxTransform t;

			if (pxActor->getKinematicTarget(t))
			{
				targetTransform.p = t.p;
			}

			pxActor->setKinematicTarget(targetTransform);
		}

		uint32_t RigidBodyComponent_GetLockFlags(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return 0;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.LockFlags with a non-dynamic Rigidbody!");
				return 0;
			}

			return (uint32_t)rigidbody.LockFlags;
		}

		void RigidBodyComponent_SetLockFlag(UUID actorUUID, ActorLockFlag flag, bool value, bool forceWake)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

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

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();

			pxActor->setRigidDynamicLockFlag((physx::PxRigidDynamicLockFlag::Enum)flag, value);

			if (forceWake)
			{
				pxActor->wakeUp();
			}
		}

		bool RigidBodyComponent_IsLockFlagSet(UUID actorUUID, ActorLockFlag flag)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return false;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.LockFlag with a non-dynamic Rigidbody!");
				return false;
			}

			return rigidbody.LockFlags & (uint8_t)flag;
		}

		bool RigidBodyComponent_IsSleeping(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return false;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Trying to access RigidBody.IsSleeping with a non-dynamic Rigidbody!");
				return false;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			return pxActor->isSleeping();
		}

		void RigidBodyComponent_WakeUp(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling RigidBody.Wakeup with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			pxActor->wakeUp();
		}

		void RigidBodyComponent_AddForce(UUID actorUUID, Math::vec3* force, ForceMode mode)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling Rigidbody.AddForce with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			pxActor->addForce(PhysicsUtils::ToPhysXVector(*force), (physx::PxForceMode::Enum)mode);
		}

		void RigidBodyComponent_AddForceAtPosition(UUID actorUUID, Math::vec3* force, Math::vec3* position, ForceMode mode)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor)
			{
				return;
			}

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling Rigidbody.AddForceAtPosition with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			physx::PxRigidBodyExt::addForceAtPos(*pxActor, PhysicsUtils::ToPhysXVector(*force), PhysicsUtils::ToPhysXVector(*position), (physx::PxForceMode::Enum)mode);
		}

		void RigidBodyComponent_AddTorque(UUID actorUUID, Math::vec3* torque, ForceMode mode)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling Rigidbody.AddTorque with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			pxActor->addTorque(PhysicsUtils::ToPhysXVector(*torque), (physx::PxForceMode::Enum)mode);
		}

		void RigidBodyComponent_ClearTorque(UUID actorUUID, ForceMode mode)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CORE_ASSERT(false, "Calling RigidBody.ClearTorque with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			pxActor->clearTorque((physx::PxForceMode::Enum)mode);
		}

		void RigidBodyComponent_ClearForce(UUID actorUUID, ForceMode mode)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBodyComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Actor doesn't have RigidBody!");
				return;
			}

			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling RigidBody.ClearForce with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* pxActor = Physics::GetPhysicsActor(actorUUID)->is<physx::PxRigidDynamic>();
			pxActor->clearForce((physx::PxForceMode::Enum)mode);
		}

#pragma endregion

#pragma region Physics

		bool Physics_Raycast(Math::vec3* origin, Math::vec3* direction, float maxDistance, RaycastHit* outHit)
		{
			Scene* contextScene = GetContextScene();

			const uint32_t result = PhysicsScene::Raycast(*origin, *direction, maxDistance, outHit);

			return (bool)result;
		}

		void Physics_GetSceneGravity(Math::vec3* outGravity)
		{
			Scene* contextScene = GetContextScene();

			*outGravity = PhysicsScene::GetGravity();
		}

		void Physics_SetSceneGravity(Math::vec3* gravity)
		{
			Scene* contextScene = GetContextScene();

			VX_CORE_ASSERT(PhysicsScene::GetScene(), "invalid physics scene!");

			PhysicsScene::SetGravity(*gravity);
		}

		uint32_t Physics_GetScenePositionIterations()
		{
			Scene* contextScene = GetContextScene();

			return PhysicsScene::GetPositionIterations();
		}

		void Physics_SetScenePositionIterations(uint32_t positionIterations)
		{
			Scene* contextScene = GetContextScene();

			PhysicsScene::SetPositionIterations(positionIterations);
		}

		uint32_t Physics_GetSceneVelocityIterations()
		{
			Scene* contextScene = GetContextScene();

			return PhysicsScene::GetVelocityIterations();
		}

		void Physics_SetSceneVelocityIterations(uint32_t velocityIterations)
		{
			Scene* contextScene = GetContextScene();

			PhysicsScene::SetVelocityIterations(velocityIterations);
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

		void CharacterControllerComponent_Move(UUID actorUUID, Math::vec3* displacement)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Calling CharacterController.Move without a Character Controller!");
				return;
			}

			const TimeStep delta = Time::GetDeltaTime();

			Physics::RT_DisplaceCharacterController(delta, actorUUID, *displacement);
		}

		void CharacterControllerComponent_Jump(UUID actorUUID, float jumpForce)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Calling CharacterController.Jump without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			characterController.SpeedDown = -1.0f * jumpForce;
		}

		bool CharacterControllerComponent_IsGrounded(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.IsGrounded without a Character Controller!");
				return false;
			}

			physx::PxController* controller = Physics::GetController(actorUUID);
			physx::PxControllerState state;
			controller->getState(state);

			return state.collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN;
		}

		void CharacterControllerComponent_GetFootPosition(UUID actorUUID, Math::vec3* outFootPos)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.FootPosition without a Character Controller!");
				return;
			}

			physx::PxExtendedVec3 footPosition = Physics::GetController(actorUUID)->getFootPosition();
			*outFootPos = PhysicsUtils::FromPhysXExtendedVector(footPosition);
		}

		float CharacterControllerComponent_GetSpeedDown(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.SpeedDown without a Character Controller!");
				return 0.0f;
			}

			const CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			return characterController.SpeedDown;
		}

		float CharacterControllerComponent_GetSlopeLimit(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.SlopeLimit without a Character Controller!");
				return 0.0f;
			}

			const CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			return characterController.SlopeLimitDegrees;
		}

		void CharacterControllerComponent_SetSlopeLimit(UUID actorUUID, float slopeLimit)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.SlopeLimit without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			characterController.SlopeLimitDegrees = slopeLimit;

			Physics::GetController(actorUUID)->setSlopeLimit(Math::Max(0.0f, cosf(Math::Deg2Rad(slopeLimit))));
		}

		float CharacterControllerComponent_GetStepOffset(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.StepOffset without a Character Controller!");
				return 0.0f;
			}

			const CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			return characterController.StepOffset;
		}

		void CharacterControllerComponent_SetStepOffset(UUID actorUUID, float stepOffset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.StepOffset without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			characterController.StepOffset = stepOffset;

			Physics::GetController(actorUUID)->setStepOffset(stepOffset);
		}

		float CharacterControllerComponent_GetContactOffset(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.ContactOffset without a Character Controller!");
				return 0.0f;
			}

			const CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			return characterController.ContactOffset;
		}

		void CharacterControllerComponent_SetContactOffset(UUID actorUUID, float contactOffset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.ContactOffset without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			characterController.ContactOffset = contactOffset;

			Physics::GetController(actorUUID)->setContactOffset(contactOffset);
		}

		NonWalkableMode CharacterControllerComponent_GetNonWalkableMode(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.NonWalkMode without a Character Controller!");
				return NonWalkableMode::PreventClimbing;
			}

			const CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			return characterController.NonWalkMode;
		}

		void CharacterControllerComponent_SetNonWalkableMode(UUID actorUUID, NonWalkableMode mode)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.NonWalkMode without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			characterController.NonWalkMode = mode;

			Physics::GetController(actorUUID)->setNonWalkableMode((physx::PxControllerNonWalkableMode::Enum)mode);
		}

		CapsuleClimbMode CharacterControllerComponent_GetClimbMode(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.ClimbMode without a Character Controller!");
				return CapsuleClimbMode::Easy;
			}

			const CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			return characterController.ClimbMode;
		}

		void CharacterControllerComponent_SetClimbMode(UUID actorUUID, CapsuleClimbMode mode)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.ClimbMode without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			characterController.ClimbMode = mode;
			// TODO any way to set capsule climbing mode during runtime?
		}

		bool CharacterControllerComponent_GetDisableGravity(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access CharacterController.DisableGravity without a Character Controller!");
				return false;
			}

			const CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			return characterController.DisableGravity;
		}

		void CharacterControllerComponent_SetDisableGravity(UUID actorUUID, bool disableGravity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set CharacterController.DisableGravity without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
			characterController.DisableGravity = disableGravity;
		}

#pragma endregion

#pragma region FixedJoint Component

		uint64_t FixedJointComponent_GetConnectedActor(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.ConnectedActor without a Fixed Joint!");
				return 0;
			}

			const FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			return fixedJointComponent.ConnectedActor;
		}

		void FixedJointComponent_SetConnectedActor(UUID actorUUID, UUID connectedActorUUID)
		{
			Scene* contextScene = GetContextScene();
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.ConnectedActor without a Fixed Joint!");
				return;
			}

			if (!contextScene->TryGetActorWithUUID(connectedActorUUID))
				return;

			physx::PxRigidActor* actor0 = Physics::GetPhysicsActor(actorUUID);
			physx::PxRigidActor* actor1 = Physics::GetPhysicsActor(connectedActorUUID);

			if (!actor0 || !actor1)
			{
				return;
			}

			FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			fixedJointComponent.ConnectedActor = connectedActorUUID;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(actorUUID);

			fixedJoint->setActors(actor0, actor1);
		}

		float FixedJointComponent_GetBreakForce(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.BreakForce without a Fixed Joint!");
				return 0.0f;
			}

			const FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			return fixedJointComponent.BreakForce;
		}

		void FixedJointComponent_SetBreakForce(UUID actorUUID, float breakForce)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.BreakForce without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			fixedJointComponent.BreakForce = breakForce;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(actorUUID);
			fixedJoint->setBreakForce(breakForce, fixedJointComponent.BreakTorque);
		}

		float FixedJointComponent_GetBreakTorque(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.BreakTorque without a Fixed Joint!");
				return 0.0f;
			}

			const FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			return fixedJointComponent.BreakTorque;
		}

		void FixedJointComponent_SetBreakTorque(UUID actorUUID, float breakTorque)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.BreakTorque without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			fixedJointComponent.BreakTorque = breakTorque;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(actorUUID);
			fixedJoint->setBreakForce(fixedJointComponent.BreakForce, breakTorque);
		}

		void FixedJointComponent_SetBreakForceAndTorque(UUID actorUUID, float breakForce, float breakTorque)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Calling FixedJoint.SetBreakForceAndTorque without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			fixedJointComponent.BreakForce = breakForce;
			fixedJointComponent.BreakTorque = breakTorque;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(actorUUID);
			fixedJoint->setBreakForce(breakForce, breakTorque);
		}

		bool FixedJointComponent_GetEnableCollision(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.CollisionEnabled without a Fixed Joint!");
				return false;
			}

			const FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			return fixedJointComponent.EnableCollision;
		}

		void FixedJointComponent_SetCollisionEnabled(UUID actorUUID, bool enableCollision)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.CollisionEnabled without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			fixedJointComponent.EnableCollision = enableCollision;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(actorUUID);
			fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, enableCollision);
		}

		bool FixedJointComponent_GetPreProcessingEnabled(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.PreProcessingEnabled without a Fixed Joint!");
				return false;
			}

			const FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			return fixedJointComponent.EnablePreProcessing;
		}

		void FixedJointComponent_SetPreProcessingEnabled(UUID actorUUID, bool enablePreProcessing)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.PreProcessingEnabled without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			fixedJointComponent.EnableCollision = enablePreProcessing;

			physx::PxFixedJoint* fixedJoint = Physics::GetFixedJoint(actorUUID);
			fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eDISABLE_PREPROCESSING, !enablePreProcessing);
		}

		bool FixedJointComponent_IsBroken(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.IsBroken without a Fixed Joint!");
				return false;
			}

			return Physics::IsConstraintBroken(actorUUID);
		}

		bool FixedJointComponent_GetIsBreakable(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to access FixedJoint.IsBreakable without a Fixed Joint!");
				return false;
			}

			const FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			return fixedJointComponent.IsBreakable;
		}

		void FixedJointComponent_SetIsBreakable(UUID actorUUID, bool isBreakable)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Trying to set FixedJoint.IsBreakable without a Fixed Joint!");
				return;
			}

			FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			fixedJointComponent.IsBreakable = isBreakable;
		}

		void FixedJointComponent_Break(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Calling FixedJoint.Break without a Fixed Joint!");
				return;
			}

			Physics::BreakJoint(actorUUID);
		}

#pragma endregion

#pragma region BoxCollider Component

		void BoxColliderComponent_GetHalfSize(UUID actorUUID, Math::vec3* outHalfSize)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider.HalfSize without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				*outHalfSize = boxCollider->GetHalfSize();
			}
		}

		void BoxColliderComponent_SetHalfSize(UUID actorUUID, Math::vec3* halfSize)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider.HalfSize without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				boxCollider->SetHalfSize(*halfSize);
			}
		}

		void BoxColliderComponent_GetOffset(UUID actorUUID, Math::vec3* outOffset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider.Offset without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				*outOffset = boxCollider->GetOffset();
			}
		}

		void BoxColliderComponent_SetOffset(UUID actorUUID, Math::vec3* offset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider.Offset without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				boxCollider->SetOffset(*offset);
			}
		}

		bool BoxColliderComponent_GetIsTrigger(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider.IsTrigger without a Box Collider!");
				return false;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				return boxCollider->IsTrigger();
			}

			return false;
		}

		void BoxColliderComponent_SetIsTrigger(UUID actorUUID, bool isTrigger)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider.IsTrigger without a Box Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<BoxColliderShape> boxCollider = collider.Is<BoxColliderShape>())
			{
				boxCollider->SetTrigger(isTrigger);
			}
		}

		bool BoxColliderComponent_GetMaterialHandle(UUID actorUUID, AssetHandle* outHandle)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider.Material without a Box Collider!");
				return false;
			}

			const BoxColliderComponent& boxCollider = actor.GetComponent<BoxColliderComponent>();
			*outHandle = boxCollider.Material;
			return AssetManager::IsHandleValid(boxCollider.Material);
		}

#pragma endregion

#pragma region SphereCollider Component

		float SphereColliderComponent_GetRadius(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SphereCollider.Radius without a Sphere Collider!");
				return 0.0f;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				return sphereCollider->GetRadius();
			}

			return 0.0f;
		}

		void SphereColliderComponent_SetRadius(UUID actorUUID, float radius)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SphereCollider.Radius without a Sphere Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				sphereCollider->SetRadius(radius);
			}
		}

		void SphereColliderComponent_GetOffset(UUID actorUUID, Math::vec3* outOffset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SphereCollider.Offset without a Sphere Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				*outOffset = sphereCollider->GetOffset();
			}
		}

		void SphereColliderComponent_SetOffset(UUID actorUUID, Math::vec3* offset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SphereCollider.Offset without a Sphere Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				sphereCollider->SetOffset(*offset);
			}
		}

		bool SphereColliderComponent_GetIsTrigger(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SphereCollider.IsTrigger without a Sphere Collider!");
				return false;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				return sphereCollider->IsTrigger();
			}

			return false;
		}

		void SphereColliderComponent_SetIsTrigger(UUID actorUUID, bool isTrigger)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set SphereCollider.Offset without a Sphere Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<SphereColliderShape> sphereCollider = collider.Is<SphereColliderShape>())
			{
				sphereCollider->SetTrigger(isTrigger);
			}
		}

		bool SphereColliderComponent_GetMaterialHandle(UUID actorUUID, AssetHandle* outHandle)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access SphereCollider.Material without a Sphere Collider!");
				return false;
			}

			const SphereColliderComponent& sphereCollider = actor.GetComponent<SphereColliderComponent>();
			*outHandle = sphereCollider.Material;
			return AssetManager::IsHandleValid(sphereCollider.Material);
		}

#pragma endregion

#pragma region CapsuleCollider Component

		float CapsuleColliderComponent_GetRadius(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.Radius without a Capsule Collider!");
				return 0.0f;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				return capsuleCollider->GetRadius();
			}

			return 0.0f;
		}

		void CapsuleColliderComponent_SetRadius(UUID actorUUID, float radius)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CapsuleCollider.Radius without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				capsuleCollider->SetRadius(radius);
			}
		}

		float CapsuleColliderComponent_GetHeight(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.Height without a Capsule Collider!");
				return 0.0f;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				return capsuleCollider->GetHeight();
			}

			return 0.0f;
		}

		void CapsuleColliderComponent_SetHeight(UUID actorUUID, float height)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CapsuleCollider.Height without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				capsuleCollider->SetHeight(height);
			}
		}

		void CapsuleColliderComponent_GetOffset(UUID actorUUID, Math::vec3* outOffset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.Offset without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				*outOffset = capsuleCollider->GetOffset();
			}
		}

		void CapsuleColliderComponent_SetOffset(UUID actorUUID, Math::vec3* offset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<SphereColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CapsuleCollider.Offset without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				capsuleCollider->SetOffset(*offset);
			}
		}

		bool CapsuleColliderComponent_GetIsTrigger(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.IsTrigger without a Capsule Collider!");
				return false;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			const auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				return capsuleCollider->IsTrigger();
			}

			return false;
		}

		void CapsuleColliderComponent_SetIsTrigger(UUID actorUUID, bool isTrigger)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CapsuleCollider.IsTrigger without a Capsule Collider!");
				return;
			}

			const auto& colliders = Physics::GetActorColliders(actorUUID);
			auto& collider = colliders.back();

			if (SharedReference<CapsuleColliderShape> capsuleCollider = collider.Is<CapsuleColliderShape>())
			{
				capsuleCollider->SetTrigger(isTrigger);
			}
		}

		bool CapsuleColliderComponent_GetMaterialHandle(UUID actorUUID, AssetHandle* outHandle)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CapsuleColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CapsuleCollider.Material without a Capsule Collider!");
				return false;
			}

			const CapsuleColliderComponent& capsuleCollider = actor.GetComponent<CapsuleColliderComponent>();
			*outHandle = capsuleCollider.Material;
			return AssetManager::IsHandleValid(capsuleCollider.Material);
		}

#pragma endregion

#pragma region MeshCollider Component

		// Finish these once we have collider assets

		bool MeshColliderComponent_IsStaticMesh(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);
			
			if (!actor.HasComponent<MeshColliderComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access MeshCollider.IsStaticMesh without a Mesh Collider!");
				return false;
			}

			const MeshColliderComponent& meshCollider = actor.GetComponent<MeshColliderComponent>();
			const auto& colliders = Physics::GetActorColliders(actorUUID);
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

		bool MeshColliderComponent_IsColliderMeshValid(UUID actorUUID, AssetHandle* assetHandle)
		{
			return false;
		}

		bool MeshColliderComponent_GetColliderMesh(UUID actorUUID, AssetHandle* outHandle)
		{
			return false;
		}

		void MeshColliderComponent_SetColliderMesh(UUID actorUUID, AssetHandle assetHandle)
		{
		}

		bool MeshColliderComponent_GetIsTrigger(UUID actorUUID)
		{
			return false;
		}

		void MeshColliderComponent_SetIsTrigger(UUID actorUUID, bool isTrigger)
		{
		}

		bool MeshColliderComponent_GetMaterialHandle(UUID actorUUID, AssetHandle* outHandle)
		{
			return false;
		}

#pragma endregion

#pragma region RigidBody2D Component

		RigidBody2DType RigidBody2DComponent_GetBodyType(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.BodyType without a RigidBody 2D!");
				return RigidBody2DType::Static;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			return rigidbody.Type;
		}

		void RigidBody2DComponent_SetBodyType(UUID actorUUID, RigidBody2DType bodyType)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.BodyType without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			const bool consistentBodyType = bodyType == rigidbody.Type;

			if (consistentBodyType)
				return;

			Physics2D::DestroyPhysicsBody(actor);
			rigidbody.Type = bodyType;
			rigidbody.RuntimeBody = nullptr;
			Physics2D::CreatePhysicsBody(actor, actor.GetTransform(), rigidbody);
		}

		void RigidBody2DComponent_GetVelocity(UUID actorUUID, Math::vec2* outVelocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.Velocity without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			*outVelocity = rigidbody.Velocity;
		}

		void RigidBody2DComponent_SetVelocity(UUID actorUUID, Math::vec2* velocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.Velocity without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			rigidbody.Velocity = *velocity;
		}

		float RigidBody2DComponent_GetDrag(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.Drag without a RigidBody 2D!");
				return 0.0f;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			return rigidbody.Drag;
		}

		void RigidBody2DComponent_SetDrag(UUID actorUUID, float drag)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.Drag without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			rigidbody.Drag = drag;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetLinearDamping(drag);
		}

		float RigidBody2DComponent_GetAngularVelocity(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.AngularVelocity without a RigidBody 2D!");
				return 0.0f;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			return rigidbody.AngularVelocity;
		}
		
		void RigidBody2DComponent_SetAngularVelocity(UUID actorUUID, float angularVelocity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.AngularVelocity without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			rigidbody.AngularVelocity = angularVelocity;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetAngularVelocity(angularVelocity);
		}
		
		float RigidBody2DComponent_GetAngularDrag(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.AngularDrag without a RigidBody 2D!");
				return 0.0f;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			return rigidbody.AngularDrag;
		}

		void RigidBody2DComponent_SetAngularDrag(UUID actorUUID, float angularDrag)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.AngularDrag without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			rigidbody.AngularDrag = angularDrag;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetAngularDamping(angularDrag);
		}

		bool RigidBody2DComponent_GetFixedRotation(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.FxiedRotation without a RigidBody 2D!");
				return false;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			return rigidbody.FixedRotation;
		}

		void RigidBody2DComponent_SetFixedRotation(UUID actorUUID, bool freeze)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.FxiedRotation without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			rigidbody.FixedRotation = freeze;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetFixedRotation(freeze);
		}

		float RigidBody2DComponent_GetGravityScale(UUID actorUUID)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access RigidBody2D.GravityScale without a RigidBody 2D!");
				return 0.0f;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			return rigidbody.GravityScale;
		}

		void RigidBody2DComponent_SetGravityScale(UUID actorUUID, float gravityScale)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set RigidBody2D.GravityScale without a RigidBody 2D!");
				return;
			}

			RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			rigidbody.GravityScale = gravityScale;

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetGravityScale(gravityScale);
		}

		void RigidBody2DComponent_ApplyForce(UUID actorUUID, Math::vec2* force, Math::vec2* point, bool wake)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling RigidBody2D.ApplyForce without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->ApplyForce(b2Vec2(force->x, force->y), b2Vec2(point->x, point->y), wake);
		}

		void RigidBody2DComponent_ApplyForceToCenter(UUID actorUUID, Math::vec2* force, bool wake)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling RigidBody2D.ApplyForceToCenter without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
		}

		void RigidBody2DComponent_ApplyLinearImpulse(UUID actorUUID, Math::vec2* impulse, Math::vec2* point, bool wake)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling RigidBody2D.ApplyLinearImpulse without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
		}

		void RigidBody2DComponent_ApplyLinearImpulseToCenter(UUID actorUUID, Math::vec2* impulse, bool wake)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<RigidBody2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Calling RigidBody2D.ApplyLinearImpulseToCenter without a RigidBody 2D!");
				return;
			}

			const RigidBody2DComponent& rigidbody = actor.GetComponent<RigidBody2DComponent>();

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

		void BoxCollider2DComponent_GetOffset(UUID actorUUID, Math::vec2* outOffset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Offset without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			*outOffset = boxCollider.Offset;
		}

		void BoxCollider2DComponent_SetOffset(UUID actorUUID, Math::vec2* offset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Offset without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			boxCollider.Offset = *offset;

			Physics2D::DestroyPhysicsBody(actor);
			Physics2D::CreatePhysicsBody(actor, GetContextScene()->GetWorldSpaceTransform(actor), actor.GetComponent<RigidBody2DComponent>());
		}

		void BoxCollider2DComponent_GetSize(UUID actorUUID, Math::vec2* outSize)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Size without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			*outSize = boxCollider.Size;
		}

		void BoxCollider2DComponent_SetSize(UUID actorUUID, Math::vec2* size)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Size without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			boxCollider.Size = *size;

			Physics2D::DestroyPhysicsBody(actor);
			Physics2D::CreatePhysicsBody(actor, GetContextScene()->GetWorldSpaceTransform(actor), actor.GetComponent<RigidBody2DComponent>());
		}

		void BoxCollider2DComponent_GetDensity(UUID actorUUID, float* outDensity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Density without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			*outDensity = boxCollider.Density;
		}

		void BoxCollider2DComponent_SetDensity(UUID actorUUID, float density)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Density without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			boxCollider.Density = density;

			b2Fixture* fixture = (b2Fixture*)boxCollider.RuntimeFixture;
			
			fixture->SetDensity(density);

			// Since we changed the density we must recalculate the mass data according to box2d
			fixture->GetBody()->ResetMassData();
		}

		void BoxCollider2DComponent_GetFriction(UUID actorUUID, float* outFriction)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Friction without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			*outFriction = boxCollider.Friction;
		}

		void BoxCollider2DComponent_SetFriction(UUID actorUUID, float friction)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Friction without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			boxCollider.Friction = friction;

			b2Fixture* fixture = (b2Fixture*)boxCollider.RuntimeFixture;

			fixture->SetFriction(friction);
		}

		void BoxCollider2DComponent_GetRestitution(UUID actorUUID, float* outRestitution)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.Restitution without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			*outRestitution = boxCollider.Restitution;
		}

		void BoxCollider2DComponent_SetRestitution(UUID actorUUID, float restitution)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.Restitution without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			boxCollider.Restitution = restitution;

			b2Fixture* fixture = (b2Fixture*)boxCollider.RuntimeFixture;
			
			fixture->SetRestitution(restitution);
		}

		void BoxCollider2DComponent_GetRestitutionThreshold(UUID actorUUID, float* outRestitutionThreshold)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access BoxCollider2D.RestitutionThreshold without a Box Collider 2D!");
				return;
			}

			const BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			*outRestitutionThreshold = boxCollider.RestitutionThreshold;
		}

		void BoxCollider2DComponent_SetRestitutionThreshold(UUID actorUUID, float restitutionThreshold)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<BoxCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set BoxCollider2D.RestitutionThreshold without a Box Collider 2D!");
				return;
			}

			BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();

			boxCollider.RestitutionThreshold = restitutionThreshold;

			b2Fixture* fixture = (b2Fixture*)boxCollider.RuntimeFixture;

			fixture->SetRestitutionThreshold(restitutionThreshold);
		}

#pragma endregion

#pragma region Circle Collider2D Component

		void CircleCollider2DComponent_GetOffset(UUID actorUUID, Math::vec2* outOffset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Offset without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			*outOffset = circleCollider.Offset;
		}

		void CircleCollider2DComponent_SetOffset(UUID actorUUID, Math::vec2* offset)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Offset without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			circleCollider.Offset = *offset;

			Physics2D::DestroyPhysicsBody(actor);
			Physics2D::CreatePhysicsBody(actor, GetContextScene()->GetWorldSpaceTransform(actor), actor.GetComponent<RigidBody2DComponent>());
		}

		void CircleCollider2DComponent_GetRadius(UUID actorUUID, float* outRadius)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Radius without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			*outRadius = circleCollider.Radius;
		}

		void CircleCollider2DComponent_SetRadius(UUID actorUUID, float radius)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Radius without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			circleCollider.Radius = radius;

			Physics2D::DestroyPhysicsBody(actor);
			Physics2D::CreatePhysicsBody(actor, GetContextScene()->GetWorldSpaceTransform(actor), actor.GetComponent<RigidBody2DComponent>());
		}

		void CircleCollider2DComponent_GetDensity(UUID actorUUID, float* outDensity)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Density without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			*outDensity = circleCollider.Density;
		}

		void CircleCollider2DComponent_SetDensity(UUID actorUUID, float density)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Density without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			circleCollider.Density = density;

			b2Fixture* fixture = (b2Fixture*)circleCollider.RuntimeFixture;

			fixture->SetDensity(density);

			// Since we changed the density we must recalculate the mass data according to box2d
			fixture->GetBody()->ResetMassData();
		}

		void CircleCollider2DComponent_GetFriction(UUID actorUUID, float* outFriction)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Friction without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			*outFriction = circleCollider.Friction;
		}

		void CircleCollider2DComponent_SetFriction(UUID actorUUID, float friction)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Friction without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			circleCollider.Friction = friction;

			b2Fixture* fixture = (b2Fixture*)circleCollider.RuntimeFixture;

			fixture->SetFriction(friction);
		}

		void CircleCollider2DComponent_GetRestitution(UUID actorUUID, float* outRestitution)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.Restitution without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			*outRestitution = circleCollider.Restitution;
		}

		void CircleCollider2DComponent_SetRestitution(UUID actorUUID, float restitution)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.Restitution without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			circleCollider.Restitution = restitution;

			b2Fixture* fixture = (b2Fixture*)circleCollider.RuntimeFixture;

			fixture->SetRestitution(restitution);
		}

		void CircleCollider2DComponent_GetRestitutionThreshold(UUID actorUUID, float* outRestitutionThreshold)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to access CircleCollider2D.RestitutionThreshold without a Circle Collider 2D!");
				return;
			}

			const CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			*outRestitutionThreshold = circleCollider.RestitutionThreshold;
		}

		void CircleCollider2DComponent_SetRestitutionThreshold(UUID actorUUID, float restitutionThreshold)
		{
			Actor actor = GetActor(actorUUID);

			if (!actor.HasComponent<CircleCollider2DComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Trying to set CircleCollider2D.RestitutionThreshold without a Circle Collider 2D!");
				return;
			}

			CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();

			circleCollider.RestitutionThreshold = restitutionThreshold;

			b2Fixture* fixture = (b2Fixture*)circleCollider.RuntimeFixture;

			fixture->SetRestitutionThreshold(restitutionThreshold);
		}

#pragma endregion

#pragma region Texture2D

		bool Texture2D_LoadFromPath(MonoString* filepath, AssetHandle* outHandle)
		{
			ManagedString mstring(filepath);
			AssetHandle textureHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(mstring.String());

			*outHandle = textureHandle;

			return AssetManager::IsHandleValid(textureHandle);
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
			const float currentTime = Time::GetTime();
			const float startTime = s_Data.SceneStartTime;
			return currentTime - startTime;
		}

		float Time_GetDeltaTime()
		{
			TimeStep dt = Time::GetDeltaTime();
			return dt.GetDeltaTime();
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

#pragma region PlayerPrefs

		bool PlayerPrefs_HasKey(MonoString* key)
		{
			ManagedString mstring(key);
			return s_Data.Serializer.HasKey(mstring.String());
		}

		bool PlayerPrefs_RemoveKey(MonoString* key)
		{
			ManagedString mstring(key);
			
			if (!s_Data.Serializer.Remove(mstring.String())) {
				VX_CONSOLE_LOG_ERROR("[Player Prefs] Trying to remove invalid key '{}'", mstring.String());
				return false;
			}

			return s_Data.Serializer.Remove(mstring.String());
		}

		void PlayerPrefs_WriteInt(MonoString* key, int32_t value)
		{
			ManagedString keyString(key);
			s_Data.Serializer.WriteInt(keyString.String(), value);
		}

		int32_t PlayerPrefs_ReadInt(MonoString* key)
		{
			ManagedString keyString(key);

			if (!s_Data.Serializer.HasKey(keyString.String())) {
				VX_CONSOLE_LOG_ERROR("[Player Prefs] Trying to read int with invalid key '{}'", keyString.String());
				return 0;
			}

			int32_t value;
			if (!s_Data.Serializer.ReadInt(keyString.String(), &value)) {
				VX_CONSOLE_LOG_ERROR("[Player Prefs] Failed to read int with key '{}'", keyString.String());
				return 0;
			}

			return value;
		}

		int32_t PlayerPrefs_ReadIntWithDefault(MonoString* key, int32_t defaultValue)
		{
			ManagedString keyString(key);

			if (!s_Data.Serializer.HasKey(keyString.String())) {
				VX_CONSOLE_LOG_ERROR("[Player Prefs] Trying to read int with invalid key '{}'", keyString.String());
				return defaultValue;
			}

			int32_t value;
			const bool success = s_Data.Serializer.ReadInt(keyString.String(), &value);
			if (!success)
				return defaultValue;

			return value;
		}

		void PlayerPrefs_WriteString(MonoString* key, MonoString* value)
		{
			ManagedString keyString(key);
			ManagedString valueString(value);
			s_Data.Serializer.WriteString(keyString.String(), valueString.String());
		}

		MonoString* PlayerPrefs_ReadString(MonoString* key)
		{
			ManagedString keyString(key);

			if (!s_Data.Serializer.HasKey(keyString.String())) {
				VX_CONSOLE_LOG_ERROR("[Player Prefs] Trying to read string with invalid key '{}'", keyString.String());
				ManagedString mstring("");
				return mstring.GetAddressOf();
			}

			std::string value;
			if (!s_Data.Serializer.ReadString(keyString.String(), &value)) {
				VX_CONSOLE_LOG_ERROR("[Player Prefs] Failed to read string with key '{}'", keyString.String());
				ManagedString mstring("");
				return mstring.GetAddressOf();
			}

			ManagedString result(value);
			return result.GetAddressOf();
		}

		MonoString* PlayerPrefs_ReadStringWithDefault(MonoString* key, MonoString* defaultValue)
		{
			ManagedString keyString(key);

			if (!s_Data.Serializer.HasKey(keyString.String())) {
				VX_CONSOLE_LOG_ERROR("[Player Prefs] Trying to read string with invalid key '{}'", keyString.String());
				return defaultValue;
			}

			std::string value;
			const bool success = s_Data.Serializer.ReadString(keyString.String(), &value);
			if (!success)
				return defaultValue;

			ManagedString result(value);
			return result.GetAddressOf();
		}

#pragma endregion

#pragma region Gui

		void Gui_Begin(MonoString* label)
		{
			ManagedString mstring(label);

			Gui::Begin(mstring.String().c_str());
		}

		void Gui_End()
		{
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
			ManagedString mstring(text);

			Gui::Text(mstring.String().c_str());
		}

		bool Gui_Button(MonoString* label)
		{
			ManagedString mstring(label);

			return Gui::Button(mstring.String().c_str());
		}

		bool Gui_ButtonWithSize(MonoString* label, const Math::vec2* size)
		{
			ManagedString mstring(label);

			return Gui::Button(mstring.String().c_str(), *(ImVec2*)size);
		}

		void Gui_BeginPropertyGrid()
		{
			UI::BeginPropertyGrid();
		}

		void Gui_EndPropertyGrid()
		{
			UI::EndPropertyGrid();
		}

		bool Gui_PropertyGridHeader(MonoString* label, bool defaultOpen)
		{
			ManagedString mstring(label);

			return UI::PropertyGridHeader(mstring.String().c_str(), defaultOpen);
		}

		void Gui_EndGridHeader()
		{
			UI::EndTreeNode();
		}

		bool Gui_PropertyBool(MonoString* label, bool* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), *outValue);
		}

		bool Gui_PropertyInt(MonoString* label, int* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), *outValue);
		}

		bool Gui_PropertyULong(MonoString* label, unsigned int* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), *outValue);
		}

		bool Gui_PropertyFloat(MonoString* label, float* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), *outValue);
		}

		bool Gui_PropertyDouble(MonoString* label, double* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), *outValue);
		}

		bool Gui_PropertyVec2(MonoString* label, Math::vec2* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), *outValue);
		}

		bool Gui_PropertyVec3(MonoString* label, Math::vec3* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), *outValue);
		}

		bool Gui_PropertyVec4(MonoString* label, Math::vec4* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), *outValue);
		}

		bool Gui_PropertyColor3(MonoString* label, Math::vec3* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), outValue);
		}

		bool Gui_PropertyColor4(MonoString* label, Math::vec4* outValue)
		{
			ManagedString mstring(label);

			return UI::Property(mstring.String().c_str(), outValue);
		}

#pragma endregion

#pragma region Log

		void Log_Message(MonoString* message, Log::LogLevel type)
		{
			ManagedString mstring(message);
			const std::string& msg = mstring.String();

			switch (type)
			{
				case Log::LogLevel::Trace: VX_CONSOLE_LOG_TRACE("{}", msg); break;
				case Log::LogLevel::Info:  VX_CONSOLE_LOG_INFO("{}", msg);  break;
				case Log::LogLevel::Warn:  VX_CONSOLE_LOG_WARN("{}", msg);  break;
				case Log::LogLevel::Error: VX_CONSOLE_LOG_ERROR("{}", msg); break;
				case Log::LogLevel::Fatal: VX_CONSOLE_LOG_FATAL("{}", msg); break;
			}
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

			MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetScriptCoreAssemblyImage());

			if (!managedType)
			{
#ifdef VX_DEBUG
				VX_CONSOLE_LOG_ERROR("[Script Engine] Could not find Component type {}", managedTypename);
#endif
				return;
			}

			s_Data.ActorAddComponentFuncs[managedType] = [](Actor actor) { actor.AddComponent<TComponent>(); };
			s_Data.ActorHasComponentFuncs[managedType] = [](Actor actor) { return actor.HasComponent<TComponent>(); };
			s_Data.ActorRemoveComponentFuncs[managedType] = [](Actor actor) { actor.RemoveComponent<TComponent>(); };
		}(), ...);
	}

	template <typename... TComponent>
	static void RegisterComponent(ComponentGroup<TComponent...>)
	{
		RegisterComponent<TComponent...>();
	}

	void ScriptRegistry::RegisterComponents()
	{
		s_Data.ActorHasComponentFuncs.clear();
		s_Data.ActorAddComponentFuncs.clear();
		s_Data.ActorRemoveComponentFuncs.clear();

		RegisterComponent(AllComponents{});
	}

	void ScriptRegistry::SetSceneStartTime(float currentTime)
	{
		s_Data.SceneStartTime = currentTime;
	}

    bool ScriptRegistry::SavePlayerPrefs()
    {
		return s_Data.Serializer.Save(s_Data.PlayerPrefsFilename.string());
    }

    bool ScriptRegistry::LoadPlayerPrefs()
    {
		return s_Data.Serializer.Load(s_Data.PlayerPrefsFilename.string());
    }

	void ScriptRegistry::RegisterInternalCalls()
	{
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Application_Quit);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_GetSize);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_GetPosition);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_IsMaximized);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_SetMaximized);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_IsResizeable);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_SetResizeable);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_IsDecorated);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_SetDecorated);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_IsVSyncEnabled);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Window_SetVSync);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(SceneRenderer_GetExposure);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SceneRenderer_SetExposure);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SceneRenderer_GetGamma);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SceneRenderer_SetGamma);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(DebugRenderer_DrawLine);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(DebugRenderer_SetLineWidth);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(DebugRenderer_DrawQuadBillboard);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(DebugRenderer_DrawCircleVec2);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(DebugRenderer_DrawCircleVec3);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(DebugRenderer_DrawBoundingBox);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(DebugRenderer_DrawBoundingBoxFromTransform);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(DebugRenderer_Flush);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_GetPrimaryCamera);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_FindActorByID);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_FindActorByName);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_FindChildByName);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_CreateActor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_Instantiate);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_InstantiateAsChild);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_IsPaused);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_Pause);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Scene_Resume);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(SceneManager_LoadScene);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_AddComponent);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_HasComponent);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_RemoveComponent);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_GetChildren);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_GetChild);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_GetTag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_SetTag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_GetMarker);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_SetMarker);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_AddChild);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_RemoveChild);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_GetScriptInstance);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_Destroy);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_DestroyWithDelay);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_Invoke);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_InvokeWithDelay);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_IsActive);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_SetActive);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_AddTimer);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Actor_IsValid);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(AssetHandle_IsValid);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Timer_GetTimeLeft);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Timer_IsStarted);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Timer_IsFinished);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Timer_Start);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetTranslation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_SetTranslation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetRotation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_SetRotation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetEulerAngles);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_SetEulerAngles);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_Rotate);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_RotateAround);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_SetTranslationAndRotation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetScale);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_SetScale);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetWorldSpaceTransform);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetTransformMatrix);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_SetTransformMatrix);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetForwardDirection);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetBackwardDirection);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetUpDirection);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetDownDirection);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetRightDirection);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetLeftDirection);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_LookAt);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_GetParent);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_SetParent);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_Unparent);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TransformComponent_Multiply);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetProjectionType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetProjectionType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetPrimary);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetPrimary);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetPerspectiveVerticalFOV);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetPerspectiveVerticalFOV);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetNearClip);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetNearClip);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetFarClip);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetFarClip);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetOrthographicSize);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetOrthographicSize);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetOrthographicNear);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetOrthographicNear);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetOrthographicFar);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetOrthographicFar);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetFixedAspectRatio);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetFixedAspectRatio);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_GetClearColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_SetClearColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_Raycast);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_ScreenToWorldPoint);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CameraComponent_ScreenToViewportPoint);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(PostProcessInfo_GetEnabled);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PostProcessInfo_SetEnabled);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(BloomInfo_GetThreshold);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BloomInfo_SetThreshold);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BloomInfo_GetKnee);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BloomInfo_SetKnee);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BloomInfo_GetIntensity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BloomInfo_SetIntensity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BloomInfo_GetEnabled);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BloomInfo_SetEnabled);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_GetLightType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_SetLightType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_GetRadiance);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_SetRadiance);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_GetIntensity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_SetIntensity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_GetCutoff);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_SetCutoff);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_GetOuterCutoff);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_SetOuterCutoff);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_GetShadowBias);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_SetShadowBias);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_GetCastShadows);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_SetCastShadows);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_GetSoftShadows);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_SetSoftShadows);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_IsVisible);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(LightSourceComponent_SetVisible);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_GetTextString);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_SetTextString);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_GetColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_SetColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_GetOutlineColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_SetOutlineColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_GetLineSpacing);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_SetLineSpacing);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_GetKerning);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_SetKerning);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_GetMaxWidth);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_SetMaxWidth);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_IsVisible);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(TextMeshComponent_SetVisible);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(AnimatorComponent_IsPlaying);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AnimatorComponent_Play);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AnimatorComponent_Stop);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshRendererComponent_GetMaterialHandle);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshRendererComponent_IsVisible);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshRendererComponent_SetVisible);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshRendererComponent_GetCastShadows);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshRendererComponent_SetCastShadows);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(StaticMeshRendererComponent_GetMeshType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(StaticMeshRendererComponent_SetMeshType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(StaticMeshRendererComponent_SetMaterialHandle);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(StaticMeshRendererComponent_IsVisible);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(StaticMeshRendererComponent_SetVisible);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(StaticMeshRendererComponent_GetCastShadows);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(StaticMeshRendererComponent_SetCastShadows);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(StaticMeshRendererComponent_GetMaterialHandle);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetAlbedo);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetAlbedo);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetAlbedoMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetAlbedoMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetNormalMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetNormalMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetMetallic);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetMetallic);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetMetallicMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetMetallicMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetRoughness);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetRoughness);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetRoughnessMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetRoughnessMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetEmission);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetEmission);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetEmissionMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetEmissionMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetAmbientOcclusionMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetAmbientOcclusionMap);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetUV);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetUV);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_GetOpacity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetOpacity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_IsFlagSet);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Material_SetFlag);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(SpriteRendererComponent_GetTextureHandle);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SpriteRendererComponent_SetTextureHandle);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SpriteRendererComponent_SetColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SpriteRendererComponent_GetUV);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SpriteRendererComponent_SetUV);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SpriteRendererComponent_IsVisible);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SpriteRendererComponent_SetVisible);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleRendererComponent_GetColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleRendererComponent_SetColor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleRendererComponent_GetThickness);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleRendererComponent_SetThickness);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleRendererComponent_GetFade);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleRendererComponent_SetFade);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleRendererComponent_IsVisible);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleRendererComponent_SetVisible);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetPosition);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetPosition);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetDirection);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetDirection);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetMinGain);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetMinGain);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetMaxGain);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetMaxGain);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetDirectionalAttenuationFactor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetDirectionalAttenuationFactor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetAttenuationModel);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetAttenuationModel);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetPan);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetPan);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetPanMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetPanMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetPositioningMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetPositioningMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetFalloff);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetFalloff);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetMinDistance);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetMinDistance);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetMaxDistance);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetMaxDistance);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetPitch);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetPitch);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetDopplerFactor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetDopplerFactor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetVolume);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetVolume);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetDirectionToListener);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetPlayOnStart);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetPlayOnStart);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetIsSpacialized);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetIsSpacialized);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetIsLooping);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetIsLooping);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetIsPlaying);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetIsPaused);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetCursorInMilliseconds);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetPinnedListenerIndex);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetPinnedListenerIndex);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_Play);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetStartTimeInMilliseconds);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetStartTimeInPCMFrames);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetFadeInMilliseconds);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetFadeStartInMilliseconds);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetFadeInPCMFrames);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetFadeStartInPCMFrames);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_GetCurrentFadeVolume);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_PlayOneShot);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_Pause);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_Restart);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_Stop);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetStopTimeInMilliseconds);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetStopTimeInPCMFrames);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetStopTimeWithFadeInMilliseconds);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SetStopTimeWithFadeInPCMFrames);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioSourceComponent_SeekToPCMFrame);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioClip_GetName);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioClip_GetLength);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioCone_GetInnerAngle);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioCone_SetInnerAngle);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioCone_GetOuterAngle);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioCone_SetOuterAngle);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioCone_GetOuterGain);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(AudioCone_SetOuterGain);
		
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics_Raycast);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics_GetSceneGravity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics_SetSceneGravity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics_GetScenePositionIterations);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics_SetScenePositionIterations);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics_GetSceneVelocityIterations);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics_SetSceneVelocityIterations);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_GetStaticFriction);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_SetStaticFriction);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_GetDynamicFriction);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_SetDynamicFriction);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_GetBounciness);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_SetBounciness);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_GetFrictionCombineMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_SetFrictionCombineMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_GetBouncinessCombineMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PhysicsMaterial_SetBouncinessCombineMode);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetBodyType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetBodyType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetCollisionDetectionType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetCollisionDetectionType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetMass);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetMass);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetLinearVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetLinearVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetMaxLinearVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetMaxLinearVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetLinearDrag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetLinearDrag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetAngularVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetAngularVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetMaxAngularVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetMaxAngularVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetAngularDrag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetAngularDrag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetDisableGravity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetDisableGravity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetIsKinematic);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetIsKinematic);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetKinematicTargetTranslation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetKinematicTargetTranslation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetKinematicTargetRotation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetKinematicTargetRotation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_GetLockFlags);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_SetLockFlag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_IsLockFlagSet);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_IsSleeping);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_WakeUp);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_AddForce);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_AddForceAtPosition);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_AddTorque);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_ClearTorque);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBodyComponent_ClearForce);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_Move);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_Jump); 
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_IsGrounded);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_GetFootPosition);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_GetSpeedDown);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_GetSlopeLimit);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_SetSlopeLimit);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_GetStepOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_SetStepOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_GetContactOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_SetContactOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_GetNonWalkableMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_SetNonWalkableMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_GetClimbMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_SetClimbMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_GetDisableGravity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CharacterControllerComponent_SetDisableGravity);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_GetConnectedActor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_SetConnectedActor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_GetBreakForce);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_SetBreakForce);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_GetBreakTorque);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_SetBreakTorque);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_SetBreakForceAndTorque);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_GetEnableCollision);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_SetCollisionEnabled);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_GetPreProcessingEnabled);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_SetPreProcessingEnabled);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_IsBroken);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_GetIsBreakable);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_SetIsBreakable);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(FixedJointComponent_Break);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxColliderComponent_GetHalfSize);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxColliderComponent_SetHalfSize);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxColliderComponent_GetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxColliderComponent_SetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxColliderComponent_GetIsTrigger);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxColliderComponent_SetIsTrigger);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxColliderComponent_GetMaterialHandle);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(SphereColliderComponent_GetRadius);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SphereColliderComponent_SetRadius);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SphereColliderComponent_GetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SphereColliderComponent_SetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SphereColliderComponent_GetIsTrigger);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SphereColliderComponent_SetIsTrigger);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(SphereColliderComponent_GetMaterialHandle);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(CapsuleColliderComponent_GetRadius);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CapsuleColliderComponent_SetRadius);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CapsuleColliderComponent_GetHeight);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CapsuleColliderComponent_SetHeight);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CapsuleColliderComponent_GetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CapsuleColliderComponent_SetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CapsuleColliderComponent_GetIsTrigger);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CapsuleColliderComponent_SetIsTrigger);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CapsuleColliderComponent_GetMaterialHandle);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshColliderComponent_IsStaticMesh);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshColliderComponent_IsColliderMeshValid);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshColliderComponent_GetColliderMesh);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshColliderComponent_SetColliderMesh);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshColliderComponent_GetIsTrigger);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshColliderComponent_SetIsTrigger);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(MeshColliderComponent_GetMaterialHandle);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_GetBodyType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_SetBodyType);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_GetVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_SetVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_GetDrag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_SetDrag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_GetAngularVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_SetAngularVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_GetAngularDrag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_SetAngularDrag);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_GetFixedRotation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_SetFixedRotation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_GetGravityScale);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_SetGravityScale);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_ApplyForce);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_ApplyForceToCenter);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulseToCenter);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics2D_Raycast);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics2D_GetWorldGravity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics2D_SetWorldGravity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics2D_GetWorldPositionIterations);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics2D_SetWorldPositionIterations);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics2D_GetWorldVelocityIterations);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Physics2D_SetWorldVelocityIterations);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_GetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_SetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_GetSize);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_SetSize);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_GetDensity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_SetDensity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_GetFriction);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_SetFriction);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_GetRestitution);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_SetRestitution);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_GetRestitutionThreshold);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(BoxCollider2DComponent_SetRestitutionThreshold);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_GetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_SetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_GetRadius);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_SetRadius);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_GetDensity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_SetDensity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_GetFriction);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_SetFriction);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_GetRestitution);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_SetRestitution);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_GetRestitutionThreshold);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(CircleCollider2DComponent_SetRestitutionThreshold);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetVelocity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetVelocityVariation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetVelocityVariation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetOffset);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetSizeBegin);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetSizeBegin);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetSizeEnd);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetSizeEnd);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetSizeVariation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetSizeVariation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetColorBegin);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetColorBegin);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetColorEnd);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetColorEnd);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetRotation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetRotation);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_GetLifeTime);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_SetLifeTime);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_Start);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_Stop);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(ParticleEmitterComponent_IsActive);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Texture2D_LoadFromPath);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Texture2D_Constructor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Texture2D_GetWidth);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Texture2D_GetHeight);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Texture2D_SetPixel);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Random_RangedInt32);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Random_RangedFloat);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Random_Float);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Quaternion_Inverse);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Matrix4_Rotate);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Matrix4_LookAt);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Matrix4_Multiply);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_GetPI);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_GetPI_D);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Round);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Abs);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Sqrt);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Pow);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Sin);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Cos);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Acos);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Tan);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Max);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Max);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Deg2Rad);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Rad2Deg);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Deg2RadVector3);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_Rad2DegVector3);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_LookAt);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Mathf_InverseQuat);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_Constructor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_Destructor);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_GetFrequency);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_SetFrequency);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_GetFractalOctaves);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_SetFractalOctaves);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_GetFractalLacunarity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_SetFractalLacunarity);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_GetFractalGain);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_SetFractalGain);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_GetVec2);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_GetVec3);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_SetSeed);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_PerlinNoiseVec2);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Noise_PerlinNoiseVec3);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Time_GetElapsed);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Time_GetDeltaTime);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsKeyPressed);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsKeyReleased);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsKeyDown);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsKeyUp);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsMouseButtonPressed);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsMouseButtonReleased);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsMouseButtonDown);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsMouseButtonUp);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_GetMousePosition);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_SetMousePosition);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_GetMouseWheelMovement);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsGamepadButtonDown);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_IsGamepadButtonUp);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_GetGamepadAxis);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_GetCursorMode);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Input_SetCursorMode);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(PlayerPrefs_HasKey);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PlayerPrefs_RemoveKey);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PlayerPrefs_WriteInt);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PlayerPrefs_ReadInt);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PlayerPrefs_ReadIntWithDefault);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PlayerPrefs_WriteString);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PlayerPrefs_ReadString);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(PlayerPrefs_ReadStringWithDefault);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_Begin);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_End);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_Underline);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_Spacing);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_Text);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_Button);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_ButtonWithSize);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_BeginPropertyGrid);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_EndPropertyGrid);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyGridHeader);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_EndGridHeader);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyBool);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyInt);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyULong);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyFloat);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyDouble);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyVec2);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyVec3);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyVec4);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyColor3);
		VX_REGISTER_DEFAULT_INTERNAL_CALL(Gui_PropertyColor4);

		VX_REGISTER_DEFAULT_INTERNAL_CALL(Log_Message);
	}

}
