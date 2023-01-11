#include "vxpch.h"
#include "ScriptRegistry.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Core/Input.h"
#include "Vortex/Core/UUID.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scripting/ScriptEngine.h"

#include "Vortex/Audio/AudioSource.h"

#include "Vortex/Physics/Physics.h"
#include "Vortex/Physics/Physics2D.h"
#include "Vortex/Physics/PhysXTypes.h"
#include "Vortex/Physics/PhysXAPIHelpers.h"

#include "Vortex/Renderer/RenderCommand.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Renderer/Model.h"

#include "Vortex/Animation/Animator.h"

#include "Vortex/Utils/PlatformUtils.h"
#include "Vortex/Core/Log.h"

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

#define VX_ADD_INTERNAL_CALL(icall) mono_add_internal_call("Vortex.InternalCalls::" #icall, icall)

	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityAddComponentFuncs;
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityRemoveComponentFuncs;

	static Entity s_HoveredEntity = Entity{};

	static float s_SceneStartTime = 0.0f;

	static std::string s_ActiveSceneName = "";
	static std::string s_SceneToBeLoaded = "";

	static Math::vec4 s_RaycastDebugLineColor = Math::vec4(1.0f, 0.0f, 0.0f, 1.0f);

#pragma region Application

	static void Application_Quit()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Application::Get().Quit();
	}

	static void Application_GetSize(Math::vec2* outSize)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		VX_CORE_TRACE(Application::Get().GetWindow().GetSize());
		*outSize = Application::Get().GetWindow().GetSize();
	}

	static void Application_GetPosition(Math::vec2* outPosition)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		*outPosition = Application::Get().GetWindow().GetPosition();
	}

	static bool Application_IsMaximized()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		return Application::Get().GetWindow().IsMaximized();
	}

#pragma endregion

#pragma region DebugRenderer

	static void DebugRenderer_BeginScene()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Entity primaryCameraEntity = contextScene->GetPrimaryCameraEntity();

		if (!primaryCameraEntity)
		{
			VX_CORE_WARN("Scene must include a primary camera to call debug render functions!");
			return;
		}

		SceneCamera& camera = primaryCameraEntity.GetComponent<CameraComponent>().Camera;
		Renderer2D::BeginScene(camera, primaryCameraEntity.GetTransform().GetTransform());
	}

	static void DebugRenderer_SetClearColor(Math::vec3* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		RenderCommand::SetClearColor(*color);
	}

	static void DebugRenderer_DrawLine(Math::vec3* p1, Math::vec3* p2, Math::vec4* color)
	{
		Renderer2D::DrawLine(*p1, *p2, *color);
	}

	static void DebugRenderer_DrawQuadBillboard(Math::vec3* translation, Math::vec2* size, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity primaryCameraEntity = contextScene->GetPrimaryCameraEntity();

		if (!primaryCameraEntity)
		{
			VX_CORE_WARN("Scene must include a primary camera to call debug render functions!");
			return;
		}

		Math::mat4 cameraView = Math::Inverse(primaryCameraEntity.GetTransform().GetTransform());

		Renderer2D::DrawQuadBillboard(cameraView, *translation, *size, *color);
	}

	static void DebugRenderer_DrawBoundingBox(Math::vec3* worldPosition, Math::vec3* size, Math::vec4* color)
	{
		Math::AABB aabb{
			- Math::vec3(0.5f),
			+ Math::vec3(0.5f),
		};

		Math::mat4 transform = Math::Identity() * Math::Translate(*worldPosition) * Math::Scale(*size);
		Renderer2D::DrawAABB(aabb, transform, *color);
	}

	static void DebugRenderer_Flush()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Renderer2D::EndScene();
	}

#pragma endregion

#pragma region Scene

	static bool Scene_FindEntityByID(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		if (entity)
		{
			return true;
		}

		return false;
	}

	static uint64_t Scene_Instantiate(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		if (!entity)
		{
			VX_CORE_WARN("Scene.Instantiate called with Invalid Entity UUID!");
			return 0;
		}

		Entity clonedEntity = contextScene->DuplicateEntity(entity);
		UUID uuid = clonedEntity.GetUUID();
		return uuid;
	}

	static bool Scene_IsPaused()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		return contextScene->IsPaused();
	}

	static void Scene_Pause()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		contextScene->SetPaused(true);
	}

	static void Scene_Resume()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		contextScene->SetPaused(false);
	}

	static uint64_t Scene_GetHoveredEntity()
	{
		if (!s_HoveredEntity)
			return 0;

		return s_HoveredEntity.GetUUID();
	}

#pragma endregion

#pragma region SceneManager

	static void SceneManager_LoadScene(MonoString* sceneName)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		char* sceneNameCStr = mono_string_to_utf8(sceneName);
		s_SceneToBeLoaded = std::string(sceneNameCStr);
		mono_free(sceneNameCStr);
	}

	static MonoString* SceneManager_GetActiveScene()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		return mono_string_new(mono_domain_get(), s_ActiveSceneName.c_str());
	}

#pragma endregion

#pragma region Entity

	static void Entity_AddComponent(UUID entityUUID, MonoReflectionType* componentType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		VX_CORE_ASSERT(s_EntityAddComponentFuncs.find(managedType) != s_EntityAddComponentFuncs.end(), "Managed type was not found in Map!");
		
		s_EntityAddComponentFuncs.at(managedType)(entity);
	}

	static bool Entity_HasComponent(UUID entityUUID, MonoReflectionType* componentType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		VX_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end(), "Managed type was not found in Map!");

		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	static void Entity_RemoveComponent(UUID entityUUID, MonoReflectionType* componentType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		VX_CORE_ASSERT(s_EntityRemoveComponentFuncs.find(managedType) != s_EntityRemoveComponentFuncs.end(), "Managed type was not found in Map!");

		s_EntityRemoveComponentFuncs.at(managedType)(entity);
	}

	static MonoArray* Entity_GetChildren(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& children = entity.Children();

		MonoClass* coreEntityClass =  ScriptEngine::GetCoreEntityClass().GetMonoClass();
		VX_CORE_ASSERT(coreEntityClass, "Core Entity Class was Invalid!");

		MonoArray* result = mono_array_new(mono_domain_get(), coreEntityClass, children.size());

		for (uint32_t i = 0; i < children.size(); i++)
		{
			MonoClass* arrayClass = mono_object_get_class((MonoObject*)result);
			MonoClass* elementClass = mono_class_get_element_class(arrayClass);
			int32_t elementSize = mono_array_element_size(arrayClass);
			MonoType* elementType = mono_class_get_type(elementClass);

			if (mono_type_is_reference(elementType) || mono_type_is_byref(elementType))
			{
				MonoObject* boxed = mono_object_new(mono_domain_get(), elementClass);
				mono_array_setref(result, (uintptr_t)i, boxed);
			}
			else
			{
				char* dst = mono_array_addr_with_size(result, elementSize, i);
				auto child = contextScene->TryGetEntityWithUUID(children[i]);
				memcpy(dst, &child, elementSize);
			}
		}

		return result;
	}

	static uint64_t Entity_GetChild(UUID entityUUID, uint32_t index)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

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

	static MonoString* Entity_GetTag(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return mono_string_new(mono_domain_get(), entity.GetName().c_str());
	}

	static MonoString* Entity_GetMarker(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return mono_string_new(mono_domain_get(), entity.GetMarker().c_str());
	}

	static uint64_t Entity_CreateWithName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->CreateEntity(nameCStr);
		mono_free(nameCStr);

		return entity.GetUUID();
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->FindEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity.GetUUID();
	}

	static bool Entity_AddChild(UUID parentUUID, UUID childUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity parent = contextScene->TryGetEntityWithUUID(parentUUID);
		VX_CORE_ASSERT(parent, "Parent UUID was Invalid!");
		Entity child = contextScene->TryGetEntityWithUUID(childUUID);
		VX_CORE_ASSERT(child, "Child UUID was Invalid!");

		if (parent && child)
		{
			contextScene->ParentEntity(child, parent);
			return true;
		}

		return false;
	}

	static bool Entity_RemoveChild(UUID parentUUID, UUID childUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity parent = contextScene->TryGetEntityWithUUID(parentUUID);
		Entity child = contextScene->TryGetEntityWithUUID(childUUID);

		if (parent && child)
		{
			contextScene->UnparentEntity(child);
			return true;
		}

		return false;
	}

	static MonoObject* Entity_GetScriptInstance(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		return ScriptEngine::GetManagedInstance(entityUUID);
	}

	static void Entity_Destroy(UUID entityUUID, bool excludeChildren)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		contextScene->DestroyEntity(entity, excludeChildren);
	}

	static void Entity_SetActive(UUID entityUUID, bool isActive)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.SetActive(isActive);
	}

#pragma endregion

#pragma region Transform Component

	static void TransformComponent_GetTranslation(UUID entityUUID, Math::vec3* outTranslation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;
	}

	static void TransformComponent_SetTranslation(UUID entityUUID, Math::vec3* translation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	static void TransformComponent_GetRotation(UUID entityUUID, Math::vec3* outRotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRotation = entity.GetComponent<TransformComponent>().GetRotationEuler();

		// Since we store rotation in radians we must convert to degrees here
		outRotation->x = Math::Rad2Deg(outRotation->x);
		outRotation->y = Math::Rad2Deg(outRotation->y);
		outRotation->z = Math::Rad2Deg(outRotation->z);
	}

	static void TransformComponent_SetRotation(UUID entityUUID, Math::vec3* rotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		// Since we store rotation in radians we must convert to radians here
		rotation->x = Math::Deg2Rad(rotation->x);
		rotation->y = Math::Deg2Rad(rotation->y);
		rotation->z = Math::Deg2Rad(rotation->z);

		entity.GetComponent<TransformComponent>().SetRotationEuler(*rotation);
	}

	static void TransformComponent_SetRotationQuaterion(UUID entityUUID, Math::quaternion* orientation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetTransform().SetRotation(*orientation);
	}

	static void TransformComponent_GetScale(UUID entityUUID, Math::vec3* outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	static void TransformComponent_SetScale(UUID entityUUID, Math::vec3* scale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TransformComponent>().Scale = *scale;
	}

	static void TransformComponent_GetWorldSpaceTransform(UUID entityUUID, Math::vec3* outTranslation, Math::vec3* outRotationEuler, Math::vec3* outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		TransformComponent worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);
		*outTranslation = worldSpaceTransform.Translation;
		*outRotationEuler = worldSpaceTransform.GetRotationEuler();
		*outScale = worldSpaceTransform.Scale;
	}

	static void TransformComponent_GetForwardDirection(UUID entityUUID, Math::vec3* outDirection)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

		Math::vec3 rotation = worldSpaceTransform.GetRotationEuler();

		*outDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), Math::vec3(0.0f, 0.0f, -1.0f));
	}

	static void TransformComponent_GetUpDirection(UUID entityUUID, Math::vec3* outDirection)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

		Math::vec3 rotation = worldSpaceTransform.GetRotationEuler();

		*outDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), Math::vec3(0.0f, 1.0f, 0.0f));
	}

	static void TransformComponent_GetRightDirection(UUID entityUUID, Math::vec3* outDirection)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

		Math::vec3 rotation = worldSpaceTransform.GetRotationEuler();

		*outDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), Math::vec3(1.0f, 0.0f, 0.0f));
	}

	static void TransformComponent_LookAt(UUID entityUUID, Math::vec3* worldPoint)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		TransformComponent& transform = entity.GetTransform();
		Math::vec3 upDirection(0.0f, 1.0f, 0.0f);
		Math::mat4 result = Math::LookAt(transform.Translation, *worldPoint, upDirection);
		Math::vec3 translation, rotation, scale;
		Math::DecomposeTransform(Math::Inverse(result), translation, rotation, scale);
		transform.Translation = translation;
		transform.SetRotationEuler(rotation);
		transform.Scale = scale;
	}
	
	static uint64_t TransformComponent_GetParent(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity child = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(child, "Invalid Child UUID!");

		Entity parent = contextScene->TryGetEntityWithUUID(child.GetParentUUID());
		
		if (!parent)
		{
			VX_CORE_WARN("Invalid Parent UUID!");
			return 0;
		}

		return parent.GetUUID();
	}

	static void TransformComponent_SetParent(UUID childUUID, UUID parentUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity child = contextScene->TryGetEntityWithUUID(childUUID);
		VX_CORE_ASSERT(child, "Invalid Child UUID!");

		Entity parent = contextScene->TryGetEntityWithUUID(parentUUID);
		VX_CORE_ASSERT(parent, "Invalid Parent UUID!");

		contextScene->ParentEntity(child, parent);
	}

	static void TransformComponent_Unparent(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		contextScene->UnparentEntity(entity);
	}

	static void TransformComponent_Multiply(TransformComponent* a, TransformComponent* b, TransformComponent* outTransform)
	{
		Math::mat4 transform = a->GetTransform() * b->GetTransform();
		TransformComponent& out = *outTransform;
		Math::quaternion orientation;
		Math::DecomposeTransform(transform, out.Translation, orientation, out.Scale);
		outTransform->SetRotation(orientation);
	}

#pragma endregion

#pragma region Camera Component

	static void CameraComponent_GetPrimary(UUID entityUUID, bool* outPrimary)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outPrimary = entity.GetComponent<CameraComponent>().Primary;
	}

	static void CameraComponent_SetPrimary(UUID entityUUID, bool primary)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CameraComponent>().Primary = primary;
	}
	
	static float CameraComponent_GetPerspectiveVerticalFOV(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return Math::Rad2Deg(entity.GetComponent<CameraComponent>().Camera.GetPerspectiveVerticalFOV());
	}

	static void CameraComponent_SetPerspectiveVerticalFOV(UUID entityUUID, float perspectiveVerticalFOV)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CameraComponent>().Camera.SetPerspectiveVerticalFOV(Math::Deg2Rad(perspectiveVerticalFOV));
	}

	static void CameraComponent_GetFixedAspectRatio(UUID entityUUID, bool* outFixedAspectRatio)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outFixedAspectRatio = entity.GetComponent<CameraComponent>().FixedAspectRatio;
	}
	
	static void CameraComponent_SetFixedAspectRatio(UUID entityUUID, bool fixedAspectRatio)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CameraComponent>().FixedAspectRatio = fixedAspectRatio;
	}

#pragma endregion

#pragma region Light Source Component

	static void LightSourceComponent_GetRadiance(UUID entityUUID, Math::vec3* outRadiance)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRadiance = entity.GetComponent<LightSourceComponent>().Source->GetRadiance();
	}

	static void LightSourceComponent_SetRadiance(UUID entityUUID, Math::vec3* radiance)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetRadiance(*radiance);
	}

	static float LightSourceComponent_GetIntensity(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<LightSourceComponent>().Source->GetIntensity();
	}

	static void LightSourceComponent_SetIntensity(UUID entityUUID, float intensity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetIntensity(intensity);
	}

	static float LightSourceComponent_GetShadowBias(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<LightSourceComponent>().Source->GetShadowBias();
	}

	static void LightSourceComponent_SetShadowBias(UUID entityUUID, float shadowBias)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetShadowBias(shadowBias);
	}

	static bool LightSourceComponent_GetCastShadows(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<LightSourceComponent>().Source->ShouldCastShadows();
	}

	static void LightSourceComponent_SetCastShadows(UUID entityUUID, bool castShadows)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetCastShadows(castShadows);
	}

	static float LightSourceComponent_GetCutoff(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<LightSourceComponent>().Source->GetCutOff();
	}

	static void LightSourceComponent_SetCutoff(UUID entityUUID, float cutoff)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetCutOff(cutoff);
	}

	static float LightSourceComponent_GetOuterCutoff(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<LightSourceComponent>().Source->GetOuterCutOff();
	}

	static void LightSourceComponent_SetOuterCutoff(UUID entityUUID, float outerCutoff)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetOuterCutOff(outerCutoff);
	}

#pragma endregion

#pragma region TextMesh Component

	static MonoString* TextMeshComponent_GetTextString(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return mono_string_new(mono_domain_get(), entity.GetComponent<TextMeshComponent>().TextString.c_str());
	}

	static void TextMeshComponent_SetTextString(UUID entityUUID, MonoString* textString)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		char* textCStr = mono_string_to_utf8(textString);

		entity.GetComponent<TextMeshComponent>().TextString = std::string(textCStr);
		mono_free(textCStr);
	}

	static void TextMeshComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<TextMeshComponent>().Color;
	}

	static void TextMeshComponent_SetColor(UUID entityUUID, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TextMeshComponent>().Color = *color;
	}

	static float TextMeshComponent_GetLineSpacing(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<TextMeshComponent>().LineSpacing;
	}

	static void TextMeshComponent_SetLineSpacing(UUID entityUUID, float lineSpacing)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TextMeshComponent>().LineSpacing = lineSpacing;
	}

	static float TextMeshComponent_GetKerning(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<TextMeshComponent>().Kerning;
	}

	static void TextMeshComponent_SetKerning(UUID entityUUID, float kerning)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TextMeshComponent>().Kerning = kerning;
	}

	static float TextMeshComponent_GetMaxWidth(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<TextMeshComponent>().MaxWidth;
	}

	static void TextMeshComponent_SetMaxWidth(UUID entityUUID, float maxWidth)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TextMeshComponent>().MaxWidth = maxWidth;
	}

#pragma endregion

#pragma region Animator Component

	static bool AnimatorComponent_IsPlaying(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<AnimatorComponent>().Animator->IsPlaying();
	}

	static void AnimatorComponent_Play(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		SharedRef<Animation> animation = entity.GetComponent<AnimationComponent>().Animation;

		if (!animation)
		{
			VX_CORE_WARN("Animation was invalid! exiting early");
			return;
		}

		SharedRef<Animator> animator = entity.GetComponent<AnimatorComponent>().Animator;
		animator->PlayAnimation();
	}

#pragma endregion

#pragma region Mesh Renderer Component

	static MeshType MeshRendererComponent_GetMeshType(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<MeshRendererComponent>().Type;
	}

	static void MeshRendererComponent_SetMeshType(UUID entityUUID, MeshType meshType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		if (meshType != MeshType::Custom)
		{
			MeshRendererComponent& meshRenderer = entity.GetComponent<MeshRendererComponent>();
			meshRenderer.Type = meshType;
			meshRenderer.Mesh = Model::Create(Model::DefaultMeshSourcePaths[static_cast<uint32_t>(meshType)], entity.GetTransform(), ModelImportOptions(), (int)(entt::entity)entity);
		}
	}

	static void MeshRendererComponent_GetScale(UUID entityUUID, Math::vec2* outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outScale = entity.GetComponent<MeshRendererComponent>().Scale;
	}

	static void MeshRendererComponent_SetScale(UUID entityUUID, Math::vec2* scale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<MeshRendererComponent>().Scale = *scale;
	}

#pragma endregion

#pragma region Material
	
	static void Material_GetAlbedo(UUID entityUUID, Math::vec3* outAlbedo)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outAlbedo = entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->GetAlbedo();
	}

	static void Material_SetAlbedo(UUID entityUUID, Math::vec3* albedo)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->SetAlbedo(*albedo);
	}

	static float Material_GetMetallic(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->GetMetallic();
	}

	static void Material_SetMetallic(UUID entityUUID, float metallic)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->SetMetallic(metallic);
	}

	static float Material_GetRoughness(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->GetRoughness();
	}

	static void Material_SetRoughness(UUID entityUUID, float roughness)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->SetRoughness(roughness);
	}

	static void Material_GetEmission(UUID entityUUID, Math::vec3* outEmission)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outEmission = entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->GetEmission();
	}

	static void Material_SetEmission(UUID entityUUID, Math::vec3* emission)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->SetEmission(*emission);
	}

#pragma endregion

#pragma region Sprite Renderer Component

	static void SpriteRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<SpriteRendererComponent>().SpriteColor;
	}

	static void SpriteRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteRendererComponent>().SpriteColor = *color;
	}

	static void SpriteRendererComponent_GetScale(UUID entityUUID, Math::vec2* outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outScale = entity.GetComponent<SpriteRendererComponent>().Scale;
	}

	static MonoString* SpriteRendererComponent_GetTexture(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const std::string& texturePath = entity.GetComponent<SpriteRendererComponent>().Texture->GetPath();

		return mono_string_new(mono_domain_get(), texturePath.c_str());
	}

	static void SpriteRendererComponent_SetTexture(UUID entityUUID, MonoString* texturePathString)
	{
		char* texturePathCStr = mono_string_to_utf8(texturePathString);

		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteRendererComponent>().Texture = Texture2D::Create(std::string(texturePathCStr));

		mono_free(texturePathCStr);
	}

	static void SpriteRendererComponent_SetScale(UUID entityUUID, Math::vec2* scale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteRendererComponent>().Scale = *scale;
	}

#pragma endregion

#pragma region Circle Renderer Component

	static void CircleRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<CircleRendererComponent>().Color;
	}

	static void CircleRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleRendererComponent>().Color = *color;
	}

	static void CircleRendererComponent_GetThickness(UUID entityUUID, float* outThickness)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outThickness = entity.GetComponent<CircleRendererComponent>().Thickness;
	}

	static void CircleRendererComponent_SetThickness(UUID entityUUID, float thickness)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleRendererComponent>().Thickness = thickness;
	}

	static void CircleRendererComponent_GetFade(UUID entityUUID, float* outFade)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outFade = entity.GetComponent<CircleRendererComponent>().Fade;
	}

	static void CircleRendererComponent_SetFade(UUID entityUUID, float fade)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleRendererComponent>().Fade = fade;
	}

#pragma endregion

#pragma region Particle Emitter Component

	static void ParticleEmitterComponent_GetVelocity(UUID entityUUID, Math::vec3* outVelocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocity = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Velocity;
	}

	static void ParticleEmitterComponent_SetVelocity(UUID entityUUID, Math::vec3* velocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Velocity = *velocity;
	}

	static void ParticleEmitterComponent_GetVelocityVariation(UUID entityUUID, Math::vec3* outVelocityVariation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocityVariation = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().VelocityVariation;
	}

	static void ParticleEmitterComponent_SetVelocityVariation(UUID entityUUID, Math::vec3* velocityVariation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().VelocityVariation = *velocityVariation;
	}

	static void ParticleEmitterComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Offset;
	}

	static void ParticleEmitterComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Offset = *offset;
	}

	static void ParticleEmitterComponent_GetSizeBegin(UUID entityUUID, Math::vec2* outSizeBegin)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSizeBegin = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeBegin;
	}

	static void ParticleEmitterComponent_SetSizeBegin(UUID entityUUID, Math::vec2* sizeBegin)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeBegin = *sizeBegin;
	}

	static void ParticleEmitterComponent_GetSizeEnd(UUID entityUUID, Math::vec2* outSizeEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSizeEnd = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeEnd;
	}

	static void ParticleEmitterComponent_SetSizeEnd(UUID entityUUID, Math::vec2* sizeEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeEnd = *sizeEnd;
	}

	static void ParticleEmitterComponent_GetSizeVariation(UUID entityUUID, Math::vec2* outSizeVariation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSizeVariation = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeVariation;
	}

	static void ParticleEmitterComponent_SetSizeVariation(UUID entityUUID, Math::vec2* sizeVariation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeVariation = *sizeVariation;
	}

	static void ParticleEmitterComponent_GetColorBegin(UUID entityUUID, Math::vec4* outColorBegin)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColorBegin = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorBegin;
	}

	static void ParticleEmitterComponent_SetColorBegin(UUID entityUUID, Math::vec4* colorBegin)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorBegin = *colorBegin;
	}

	static void ParticleEmitterComponent_GetColorEnd(UUID entityUUID, Math::vec4* outColorEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColorEnd = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorEnd;
	}

	static void ParticleEmitterComponent_SetColorEnd(UUID entityUUID, Math::vec4* colorEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorEnd = *colorEnd;
	}

	static void ParticleEmitterComponent_GetRotation(UUID entityUUID, float* outRotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRotation = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Rotation;
	}

	static void ParticleEmitterComponent_SetRotation(UUID entityUUID, float colorEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Rotation = colorEnd;
	}

	static void ParticleEmitterComponent_GetLifeTime(UUID entityUUID, float* outLifeTime)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outLifeTime = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().LifeTime;
	}

	static void ParticleEmitterComponent_SetLifeTime(UUID entityUUID, float lifetime)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().LifeTime = lifetime;
	}

	static void ParticleEmitterComponent_Start(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->Start();
	}

	static void ParticleEmitterComponent_Stop(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->Stop();
	}

#pragma endregion

#pragma region AudioSource Component

	static bool AudioSourceComponent_GetIsPlaying(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<AudioSourceComponent>().Source->IsPlaying();
	}

	static void AudioSourceComponent_Play(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<AudioSourceComponent>().Source->Play();
	}

	static void AudioSourceComponent_Stop(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<AudioSourceComponent>().Source->Stop();
	}

#pragma endregion

#pragma region RigidBody Component

	static void RigidBodyComponent_GetTranslation(UUID entityUUID, Math::vec3* outTranslation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outTranslation = FromPhysXVector(((physx::PxRigidDynamic*)entity.GetComponent<RigidBodyComponent>().RuntimeActor)->getGlobalPose().p);
	}

	static void RigidBodyComponent_SetTranslation(UUID entityUUID, Math::vec3* translation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& transformComponent = entity.GetTransform();
		Math::vec3 rotation = transformComponent.GetRotationEuler();
		auto entityTransform = TransformComponent{ *translation, rotation, transformComponent.Scale }.GetTransform();
		auto physxTransform = ToPhysXTransform(entityTransform);

		((physx::PxRigidDynamic*)entity.GetComponent<RigidBodyComponent>().RuntimeActor)->setGlobalPose(physxTransform);
	}

	static void RigidBodyComponent_GetRotation(UUID entityUUID, Math::vec3* outRotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRotation = Math::EulerAngles(FromPhysXQuat(((physx::PxRigidDynamic*)entity.GetComponent<RigidBodyComponent>().RuntimeActor)->getGlobalPose().q));
	}

	static void RigidBodyComponent_SetRotation(UUID entityUUID, Math::vec3* rotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		RigidBodyComponent& rigidBody = entity.GetComponent<RigidBodyComponent>();
		physx::PxRigidDynamic* actor = ((physx::PxRigidDynamic*)rigidBody.RuntimeActor);
		physx::PxTransform physxTransform = actor->getGlobalPose();
		physxTransform.q = ToPhysXQuat(Math::quaternion(*rotation));

		actor->setGlobalPose(physxTransform);
	}

	static void RigidBodyComponent_Translate(UUID entityUUID, Math::vec3* translation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		RigidBodyComponent& rigidBody = entity.GetComponent<RigidBodyComponent>();
		physx::PxRigidDynamic* actor = ((physx::PxRigidDynamic*)rigidBody.RuntimeActor);
		physx::PxTransform physxTransform = actor->getGlobalPose();
		physxTransform.p += ToPhysXVector(*translation);

		actor->setGlobalPose(physxTransform);
	}

	static void RigidBodyComponent_Rotate(UUID entityUUID, Math::vec3* rotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		RigidBodyComponent& rigidBody = entity.GetComponent<RigidBodyComponent>();
		physx::PxRigidDynamic* actor = ((physx::PxRigidDynamic*)rigidBody.RuntimeActor);
		physx::PxTransform physxTransform = actor->getGlobalPose();
		physxTransform.q *= physx::PxQuat(Math::Deg2Rad(rotation->x), { 1.0f, 0.0f, 0.0f })
			* physx::PxQuat(Math::Deg2Rad(rotation->y), { 0.0f, 1.0f, 0.0f })
			* physx::PxQuat(Math::Deg2Rad(rotation->z), { 0.0f, 0.0f, 1.0f });

		actor->setGlobalPose(physxTransform);
	}

	static void RigidBodyComponent_LookAt(UUID entityUUID, Math::vec3* worldPoint)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		RigidBodyComponent& rigidBody = entity.GetComponent<RigidBodyComponent>();
		physx::PxRigidDynamic* actor = ((physx::PxRigidDynamic*)rigidBody.RuntimeActor);
		physx::PxTransform physxTransform = actor->getGlobalPose();

		Math::mat4 transform = FromPhysXTransform(physxTransform);

		Math::vec3 upDirection(0.0f, 1.0f, 0.0f);
		Math::mat4 result = Math::LookAt(FromPhysXVector(physxTransform.p), *worldPoint, upDirection);
		Math::vec3 translation, rotation, scale;
		Math::DecomposeTransform(Math::Inverse(result), translation, rotation, scale);
		physxTransform.q = ToPhysXQuat(Math::quaternion(rotation));

		actor->setGlobalPose(ToPhysXTransform(transform));
	}

	static RigidBodyType RigidBodyComponent_GetBodyType(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().Type;
	}

	static void RigidBodyComponent_SetBodyType(UUID entityUUID, RigidBodyType bodyType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

		if (bodyType != rigidbody.Type) // recreate the body if the new type is different than the old one
		{
			if (rigidbody.RuntimeActor)
			{
				Physics::DestroyPhysicsBody(entity);
			}

			rigidbody.Type = bodyType;
			rigidbody.RuntimeActor = nullptr;
			Physics::CreatePhysicsBody(entity, entity.GetTransform(), rigidbody);
		}
	}

	static CollisionDetectionType RigidBodyComponent_GetCollisionDetectionType(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().CollisionDetection;
	}

	static void RigidBodyComponent_SetCollisionDetectionType(UUID entityUUID, CollisionDetectionType collisionDetectionType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().CollisionDetection = collisionDetectionType;
	}

	static float RigidBodyComponent_GetMass(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().Mass;
	}

	static void RigidBodyComponent_SetMass(UUID entityUUID, float mass)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().Mass = mass;
	}

	static void RigidBodyComponent_GetLinearVelocity(UUID entityUUID, Math::vec3* outVelocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocity = entity.GetComponent<RigidBodyComponent>().LinearVelocity;
	}

	static void RigidBodyComponent_SetLinearVelocity(UUID entityUUID, Math::vec3* velocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().LinearVelocity = *velocity;
	}

	static float RigidBodyComponent_GetLinearDrag(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().LinearDrag;
	}

	static void RigidBodyComponent_SetLinearDrag(UUID entityUUID, float drag)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().LinearDrag = drag;
	}

	static void RigidBodyComponent_GetAngularVelocity(UUID entityUUID, Math::vec3* outVelocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocity = entity.GetComponent<RigidBodyComponent>().AngularVelocity;
	}

	static void RigidBodyComponent_SetAngularVelocity(UUID entityUUID, Math::vec3* velocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().AngularVelocity = *velocity;
	}

	static float RigidBodyComponent_GetAngularDrag(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().AngularDrag;
	}

	static void RigidBodyComponent_SetAngularDrag(UUID entityUUID, float drag)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().AngularDrag = drag;
	}

	static bool RigidBodyComponent_GetDisableGravity(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().DisableGravity;
	}

	static void RigidBodyComponent_SetDisableGravity(UUID entityUUID, bool disabled)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().DisableGravity = disabled;
	}

	static bool RigidBodyComponent_GetIsKinematic(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().IsKinematic;
	}

	static void RigidBodyComponent_SetIsKinematic(UUID entityUUID, bool isKinematic)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().IsKinematic = isKinematic;
	}

	static void RigidBodyComponent_AddForce(UUID entityUUID, Math::vec3* force, ForceMode mode)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

		if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
		{
			VX_CORE_WARN("Calling Rigidbody.AddForce with a non-dynamic Rigidbody!");
			return;
		}

		physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rigidbody.RuntimeActor);
		actor->addForce(ToPhysXVector(*force), static_cast<physx::PxForceMode::Enum>(mode));
	}

	static void RigidBodyComponent_AddForceAtPosition(UUID entityUUID, Math::vec3* force, Math::vec3* position, ForceMode mode)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

		if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
		{
			VX_CORE_WARN("Calling Rigidbody.AddForceAtPosition with a non-dynamic Rigidbody!");
			return;
		}

		physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rigidbody.RuntimeActor);
		physx::PxRigidBodyExt::addForceAtPos(*actor, ToPhysXVector(*force), ToPhysXVector(*position), static_cast<physx::PxForceMode::Enum>(mode));
	}

	static void RigidBodyComponent_AddTorque(UUID entityUUID, Math::vec3* torque, ForceMode mode)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

		if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
		{
			VX_CORE_WARN("Calling Rigidbody.AddTorque with a non-dynamic Rigidbody!");
			return;
		}

		physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rigidbody.RuntimeActor);
		actor->addTorque(ToPhysXVector(*torque), static_cast<physx::PxForceMode::Enum>(mode));
	}

	static void RigidBodyComponent_ClearTorque(UUID entityUUID, ForceMode mode)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

		if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
		{
			VX_CORE_ASSERT("Calling RigidBody.ClearTorque with a non-dynamic Rigidbody!");
			return;
		}

		physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rigidbody.RuntimeActor);
		actor->clearTorque(static_cast<physx::PxForceMode::Enum>(mode));
	}

	static void RigidBodyComponent_ClearForce(UUID entityUUID, ForceMode mode)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

		if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
		{
			VX_CORE_WARN("Calling RigidBody.ClearForce with a non-dynamic Rigidbody!");
			return;
		}

		physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rigidbody.RuntimeActor);
		actor->clearForce(static_cast<physx::PxForceMode::Enum>(mode));
	}

#pragma endregion

#pragma region Physics

	static bool Physics_Raycast(Math::vec3* origin, Math::vec3* direction, float maxDistance, RaycastHit* outHit)
	{
		physx::PxScene* scene = Physics::GetPhysicsScene();
		physx::PxRaycastBuffer hitInfo;
		bool result = scene->raycast(ToPhysXVector(*origin), ToPhysXVector(Math::Normalize(*direction)), maxDistance, hitInfo);

		if (result)
		{
			void* userData = hitInfo.block.actor->userData;

			if (!userData)
			{
				*outHit = RaycastHit();
				return false;
			}

			PhysicsBodyData* physicsBodyData = (PhysicsBodyData*)userData;
			UUID entityUUID = physicsBodyData->EntityUUID;

			// Call Hit Entity's OnCollision Method
			Scene* contextScene = ScriptEngine::GetContextScene();
			VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
			Entity hitEntity = contextScene->TryGetEntityWithUUID(entityUUID);
			VX_CORE_ASSERT(hitEntity, "Entity UUID was Invalid!");

			if (hitEntity.HasComponent<ScriptComponent>())
			{
				if (ScriptEngine::EntityClassExists(hitEntity.GetComponent<ScriptComponent>().ClassName))
				{
					ScriptEngine::OnRaycastCollisionEntity(hitEntity);
				}
			}

			outHit->EntityID = entityUUID;
			outHit->Position = FromPhysXVector(hitInfo.block.position);
			outHit->Normal = FromPhysXVector(hitInfo.block.normal);
			outHit->Distance = hitInfo.block.distance;
		}

		return result;
	}

	static void Physics_GetSceneGravity(Math::vec3* outGravity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		*outGravity = Physics::GetPhysicsSceneGravity();
	}

	static void Physics_SetSceneGravity(Math::vec3* gravity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Physics::SetPhysicsSceneGravity(*gravity);
	}

	static uint32_t Physics_GetScenePositionIterations()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		return Physics::GetPhysicsScenePositionIterations();
	}

	static void Physics_SetScenePositionIterations(uint32_t positionIterations)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Physics::SetPhysicsScenePositionIterations(positionIterations);
	}

	static uint32_t Physics_GetSceneVelocityIterations()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		return Physics::GetPhysicsSceneVelocityIterations();
	}

	static void Physics_SetSceneVelocityIterations(uint32_t velocityIterations)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Physics::SetPhysicsSceneVelocityIterations(velocityIterations);
	}

#pragma endregion

#pragma region Character Controller Component

	static void CharacterControllerComponent_Move(UUID entityUUID, Math::vec3* displacement)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		if (!entity.HasComponent<CharacterControllerComponent>())
		{
			VX_CORE_WARN("Trying to move entity without Character Controller!");
			return;
		}

		CharacterControllerComponent& characterControllerComponent = entity.GetComponent<CharacterControllerComponent>();

		physx::PxControllerFilters filters; // TODO
		physx::PxController* controller = static_cast<physx::PxController*>(characterControllerComponent.RuntimeController);

		auto gravity = Physics::GetPhysicsSceneGravity();

		if (!characterControllerComponent.DisableGravity)
			characterControllerComponent.SpeedDown -= gravity.y * Time::GetDeltaTime();

		Math::vec3 movement = *displacement - FromPhysXVector(controller->getUpDirection()) * characterControllerComponent.SpeedDown * Time::GetDeltaTime();

		controller->move(ToPhysXVector(movement), 0.0f, Time::GetDeltaTime(), filters);
		entity.GetTransform().Translation = FromPhysXExtendedVector(controller->getPosition());

		physx::PxControllerState state;
		controller->getState(state);

		// test if grounded
		if (state.collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
			characterControllerComponent.SpeedDown = gravity.y * 0.01f;
	}

	static void CharacterControllerComponent_Jump(UUID entityUUID, float jumpForce)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID");

		CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
		characterController.SpeedDown = -1.0f * jumpForce;
	}

	static bool CharacterControllerComponent_IsGrounded(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID");

		CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
		physx::PxController* controller = static_cast<physx::PxController*>(characterController.RuntimeController);

		physx::PxControllerState state;
		controller->getState(state);

		// test if grounded
		if (state.collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
		{
			return true;
		}

		return false;
	}

	static bool CharacterControllerComponent_GetDisableGravity(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID");

		return entity.GetComponent<CharacterControllerComponent>().DisableGravity;
	}

	static void CharacterControllerComponent_SetDisableGravity(UUID entityUUID, bool disableGravity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID");

		entity.GetComponent<CharacterControllerComponent>().DisableGravity = disableGravity;
	}

#pragma endregion

#pragma region BoxCollider Component

	static void BoxColliderComponent_GetHalfSize(UUID entityUUID, Math::vec3* outHalfSize)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outHalfSize = entity.GetComponent<BoxColliderComponent>().HalfSize;
	}
	
	static void BoxColliderComponent_SetHalfSize(UUID entityUUID, Math::vec3* halfSize)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxColliderComponent>().HalfSize = *halfSize;
	}

	static void BoxColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<BoxColliderComponent>().Offset;
	}

	static void BoxColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxColliderComponent>().Offset = *offset;
	}

	static bool BoxColliderComponent_GetIsTrigger(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<BoxColliderComponent>().IsTrigger;
	}

	static void BoxColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxColliderComponent>().IsTrigger = isTrigger;
	}

#pragma endregion

#pragma region SphereCollider Component

	static float SphereColliderComponent_GetRadius(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<SphereColliderComponent>().Radius;
	}

	static void SphereColliderComponent_SetRadius(UUID entityUUID, float radius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SphereColliderComponent>().Radius = radius;
	}

	static void SphereColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<SphereColliderComponent>().Offset;
	}

	static void SphereColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SphereColliderComponent>().Offset = *offset;
	}
	
	static bool SphereColliderComponent_GetIsTrigger(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<SphereColliderComponent>().IsTrigger;
	}
	
	static void SphereColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SphereColliderComponent>().IsTrigger = isTrigger;
	}

#pragma endregion

#pragma region CapsuleCollider Component

	static float CapsuleColliderComponent_GetRadius(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<CapsuleColliderComponent>().Radius;
	}
	
	static void CapsuleColliderComponent_SetRadius(UUID entityUUID, float radius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CapsuleColliderComponent>().Radius = radius;
	}
	
	static float CapsuleColliderComponent_GetHeight(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<CapsuleColliderComponent>().Height;
	}
	
	static void CapsuleColliderComponent_SetHeight(UUID entityUUID, float height)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CapsuleColliderComponent>().Height = height;
	}

	static void CapsuleColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<CapsuleColliderComponent>().Offset;
	}

	static void CapsuleColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CapsuleColliderComponent>().Offset = *offset;
	}

	static bool CapsuleColliderComponent_GetIsTrigger(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<CapsuleColliderComponent>().IsTrigger;
	}

	static void CapsuleColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CapsuleColliderComponent>().IsTrigger = isTrigger;
	}

#pragma endregion

#pragma region RigidBody2D Component

	static RigidBody2DType RigidBody2DComponent_GetBodyType(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBody2DComponent>().Type;
	}

	static void RigidBody2DComponent_SetBodyType(UUID entityUUID, RigidBody2DType bodyType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		RigidBody2DComponent& rb2d = entity.GetComponent<RigidBody2DComponent>();

		if (bodyType != rb2d.Type)
		{
			Physics2D::DestroyPhysicsBody(entity);
			rb2d.Type = bodyType;
			rb2d.RuntimeBody = nullptr;
			Physics2D::CreatePhysicsBody(entity, entity.GetTransform(), rb2d);
		}
	}

	static void RigidBody2DComponent_ApplyForce(UUID entityUUID, Math::vec2* force, Math::vec2* point, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyForce(b2Vec2(force->x, force->y), b2Vec2(point->x, point->y), wake);
	}

	static void RigidBody2DComponent_ApplyForceToCenter(UUID entityUUID, Math::vec2* force, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
	}

	static void RigidBody2DComponent_ApplyLinearImpulse(UUID entityUUID, Math::vec2* impulse, Math::vec2* point, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}

	static void RigidBody2DComponent_ApplyLinearImpulseToCenter(UUID entityUUID, Math::vec2* impulse, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

	static void RigidBody2DComponent_GetVelocity(UUID entityUUID, Math::vec2* outVelocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocity = entity.GetComponent<RigidBody2DComponent>().Velocity;
	}

	static void RigidBody2DComponent_SetVelocity(UUID entityUUID, Math::vec2* velocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBody2DComponent>().Velocity = *velocity;
	}

	static float RigidBody2DComponent_GetDrag(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBody2DComponent>().Drag;
	}

	static void RigidBody2DComponent_SetDrag(UUID entityUUID, float drag)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBody2DComponent>().Drag = drag;
	}

	static bool RigidBody2DComponent_GetFixedRotation(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBody2DComponent>().FixedRotation;
	}

	static void RigidBody2DComponent_SetFixedRotation(UUID entityUUID, bool freeze)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBody2DComponent>().FixedRotation = freeze;
	}

#pragma endregion

#pragma region Physics2D

	// Derived class of Box2D RayCastCallback
	class RayCastCallback : public b2RayCastCallback
	{
	public:
		RayCastCallback() : fixture(nullptr) { }

		float ReportFixture(b2Fixture* fixture_, const b2Vec2& point_, const b2Vec2& normal_, float fraction_) override
		{
			fixture = fixture_;
			point = point_;
			normal = normal_;
			fraction = fraction_;
			return fraction;
		}

		b2Fixture* fixture; // This is the fixture that was hit by the raycast
		b2Vec2 point;
		b2Vec2 normal;
		float fraction;
	};

	struct RayCastHit2D
	{
		Math::vec2 Point;
		Math::vec2 Normal;
		MonoString* Tag;
		bool Hit;

		RayCastHit2D(const RayCastCallback& raycastInfo, Scene* contextScene)
		{
			Hit = raycastInfo.fixture != nullptr;

			if (Hit)
			{
				Point = Math::vec2(raycastInfo.point.x, raycastInfo.point.y);
				Normal = Math::vec2(raycastInfo.normal.x, raycastInfo.normal.y);
				UUID entityUUID = reinterpret_cast<PhysicsBody2DData*>(raycastInfo.fixture->GetUserData().pointer)->EntityUUID;
				Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
				Tag = mono_string_new(mono_domain_get(), entity.GetName().c_str());

				if (ScriptEngine::GetEntityScriptInstance(entityUUID) != nullptr)
					ScriptEngine::OnRaycastCollisionEntity(entity); // Call the Entity's OnCollision Function
			}
			else
			{
				Point = Math::vec2();
				Normal = Math::vec2();
				Tag = mono_string_new(mono_domain_get(), "");
			}
		}
	};

	static uint64_t Physics2D_Raycast(Math::vec2* start, Math::vec2* end, RayCastHit2D* outResult, bool drawDebugLine)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		
		// Create an instance of the callback and initialize it
		RayCastCallback raycastCallback;
		Physics2D::GetPhysicsScene()->RayCast(&raycastCallback, { start->x, start->y }, { end->x, end->y });

		*outResult = RayCastHit2D(raycastCallback, contextScene);

		// Render Raycast Hits
		if (drawDebugLine && outResult->Hit)
		{
			Renderer2D::DrawLine({ start->x, start->y, 0.0f }, { end->x, end->y, 0.0f }, s_RaycastDebugLineColor);
			Renderer2D::Flush();
		}

		if (outResult->Hit)
		{
			return reinterpret_cast<PhysicsBody2DData*>(raycastCallback.fixture->GetUserData().pointer)->EntityUUID;
		}
		else
		{
			return 0; // Invalid entity
		}
	}

	static void Physics2D_GetWorldGravity(Math::vec2* outGravity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		*outGravity = Physics2D::GetPhysicsWorldGravity();
	}

	static void Physics2D_SetWorldGravity(Math::vec2* gravity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Physics2D::SetPhysicsWorldGravitty(*gravity);
	}

	static uint32_t Physics2D_GetWorldPositionIterations()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		return Physics2D::GetPhysicsWorldPositionIterations();
	}

	static void Physics2D_SetWorldPositionIterations(uint32_t positionIterations)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Physics2D::SetPhysicsWorldPositionIterations(positionIterations);
	}

	static uint32_t Physics2D_GetWorldVelocityIterations()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		return Physics2D::GetPhysicsWorldVelocityIterations();
	}

	static void Physics2D_SetWorldVelocityIterations(uint32_t velocityIterations)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Physics2D::SetPhysicsWorldVelocityIterations(velocityIterations);
	}

#pragma endregion

#pragma region Box Collider2D Component

	static void BoxCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<BoxCollider2DComponent>().Offset;
	}

	static void BoxCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().Offset = *offset;
	}
	
	static void BoxCollider2DComponent_GetSize(UUID entityUUID, Math::vec2* outSize)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSize = entity.GetComponent<BoxCollider2DComponent>().Size;
	}
	
	static void BoxCollider2DComponent_SetSize(UUID entityUUID, Math::vec2* size)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().Size = *size;
	}

	static void BoxCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outDensity = entity.GetComponent<BoxCollider2DComponent>().Density;
	}

	static void BoxCollider2DComponent_SetDensity(UUID entityUUID, float density)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetDensity(density);
		// Since we changed the density we must recalculate the mass data according to box2d
		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->GetBody()->ResetMassData();
	}

	static void BoxCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outFriction = entity.GetComponent<BoxCollider2DComponent>().Friction;
	}

	static void BoxCollider2DComponent_SetFriction(UUID entityUUID, float friction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetFriction(friction);
	}

	static void BoxCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRestitution = entity.GetComponent<BoxCollider2DComponent>().Restitution;
	}

	static void BoxCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetRestitution(restitution);
	}

	static void BoxCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRestitutionThreshold = entity.GetComponent<BoxCollider2DComponent>().RestitutionThreshold;
	}

	static void BoxCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetRestitutionThreshold(restitutionThreshold);
	}

#pragma endregion

#pragma region Circle Collider2D Component

	static void CircleCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<CircleCollider2DComponent>().Offset;
	}

	static void CircleCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().Offset = *offset;
	}

	static void CircleCollider2DComponent_GetRadius(UUID entityUUID, float* outRadius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRadius = entity.GetComponent<CircleCollider2DComponent>().Radius;
	}

	static void CircleCollider2DComponent_SetRadius(UUID entityUUID, float radius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().Radius = radius;
	}

	static void CircleCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outDensity = entity.GetComponent<CircleCollider2DComponent>().Density;
	}

	static void CircleCollider2DComponent_SetDensity(UUID entityUUID, float density)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetDensity(density);
		// Since we changed the density we must recalculate the mass data according to box2d
		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->GetBody()->ResetMassData();
	}

	static void CircleCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outFriction = entity.GetComponent<CircleCollider2DComponent>().Friction;
	}

	static void CircleCollider2DComponent_SetFriction(UUID entityUUID, float friction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetFriction(friction);
	}

	static void CircleCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRestitution = entity.GetComponent<CircleCollider2DComponent>().Restitution;
	}

	static void CircleCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetRestitution(restitution);
	}

	static void CircleCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRestitutionThreshold = entity.GetComponent<CircleCollider2DComponent>().RestitutionThreshold;
	}

	static void CircleCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetRestitution(restitutionThreshold);
	}

#pragma endregion

#pragma region Random

	static int RandomDevice_RangedInt32(int min, int max)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		std::random_device randomDevice;
		std::mt19937 engine(randomDevice());
		std::uniform_int_distribution<int> uniformDistribution(min, max);

		return uniformDistribution(engine);
	}

	static float RandomDevice_RangedFloat(float min, float max)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		static bool seedGenerated = false;

		if (!seedGenerated)
		{
			srand((uint32_t)time(0));
			seedGenerated = true;
		}

		auto createRandomFloat = [max]() { return (float)rand() / (float)RAND_MAX * (max); };

		float randomValue = createRandomFloat();

		while (randomValue < min)
			randomValue = createRandomFloat();

		return randomValue;
	}

#pragma endregion

#pragma region Mathf

	static float Mathf_GetPI()
	{
		return Math::PI;
	}

	static double Mathf_GetPI_D()
	{
		return Math::PI_D;
	}

	static float Mathf_Sqrt(float in)
	{
		return Math::Sqrt(in);
	}

	static float Mathf_Sin(float in)
	{
		return Math::Sin(in);
	}

	static float Mathf_Cos(float in)
	{
		return Math::Cos(in);
	}

	static float Mathf_Acos(float in)
	{
		return Math::Acos(in);
	}

	static float Mathf_Tan(float in)
	{
		return Math::Tan(in);
	}

	static float Mathf_Max(float x, float y)
	{
		return Math::Max(x, y);
	}

	static float Mathf_Min(float x, float y)
	{
		return Math::Min(x, y);
	}

	static float Mathf_Deg2Rad(float degrees)
	{
		return Math::Deg2Rad(degrees);
	}

	static float Mathf_Rad2Deg(float radians)
	{
		return Math::Rad2Deg(radians);
	}

	static void Mathf_Deg2RadVector3(Math::vec3* value, Math::vec3* outResult)
	{
		*outResult = Math::Deg2Rad(*value);
	}

	static void Mathf_Rad2DegVector3(Math::vec3* value, Math::vec3* outResult)
	{
		*outResult = Math::Rad2Deg(*value);
	}

#pragma endregion

#pragma region Vector3

	static void Vector3_CrossProductVec3(Math::vec3* left, Math::vec3* right, Math::vec3* outResult)
	{
		*outResult = Math::Cross(*left, *right);
	}

	static float Vector3_DotProductVec3(Math::vec3* left, Math::vec3* right)
	{
		return Math::Dot(*left, *right);
	}

#pragma endregion

#pragma region Time

	static float Time_GetElapsed()
	{
		return Time::GetTime() - s_SceneStartTime;
	}

	static float Time_GetDeltaTime()
	{
		return Time::GetDeltaTime();
	}

#pragma endregion

#pragma region Input

	static bool Input_IsKeyDown(KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}
	
	static bool Input_IsKeyUp(KeyCode key)
	{
		return Input::IsKeyReleased(key);
	}

	static bool Input_IsMouseButtonDown(MouseCode mouseButton)
	{
		return Input::IsMouseButtonPressed(mouseButton);
	}

	static bool Input_IsMouseButtonUp(MouseCode mouseButton)
	{
		return Input::IsMouseButtonReleased(mouseButton);
	}

	static void Input_GetMousePosition(Math::vec2* outPosition)
	{
		Math::vec2 mousePos = Input::GetMousePosition();
		mousePos.y *= -1.0f; // This makes more sense
		*outPosition = mousePos;
	}

	static void Input_GetMouseScrollOffset(Math::vec2* outMouseScrollOffset)
	{
		*outMouseScrollOffset = Input::GetMouseScrollOffset();
	}
	
	static bool Input_IsGamepadButtonDown(Gamepad button)
	{
		return Input::IsGamepadButtonPressed(button);
	}

	static bool Input_IsGamepadButtonUp(Gamepad button)
	{
		return Input::IsGamepadButtonReleased(button);
	}

	static float Input_GetGamepadAxis(Gamepad axis)
	{
		return Input::GetGamepadAxis(axis);
	}

	static void Input_ShowMouseCursor(bool enabled)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Application::Get().GetWindow().ShowMouseCursor(enabled);
	}

#pragma endregion

#pragma region Gui

	namespace Gui = ImGui;

	static uint32_t defaultWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking;

	static void BeginWindow(char* text, uint32_t flags = 0)
	{
		ImGuiIO& io = Gui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		Gui::Begin(text, nullptr, defaultWindowFlags | flags);
		Gui::PushFont(largeFont);
		Gui::TextCentered(text);
		Gui::PopFont();
		Gui::Separator();
		Gui::Spacing();
	}

	static void Gui_Begin(MonoString* text)
	{
		char* textCStr = mono_string_to_utf8(text);

		BeginWindow(textCStr);

		mono_free(textCStr);
	}
	
	static void Gui_BeginWithPosition(MonoString* text, Math::vec2* position)
	{
		char* textCStr = mono_string_to_utf8(text);

		Gui::SetNextWindowPos({ position->x, position->y });
		BeginWindow(textCStr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

		mono_free(textCStr);
	}

	static void Gui_BeginWithSize(MonoString* text, float width, float height)
	{
		char* textCStr = mono_string_to_utf8(text);

		Gui::SetNextWindowSize({ width, height });
		BeginWindow(textCStr, ImGuiWindowFlags_NoResize);

		mono_free(textCStr);
	}

	static void Gui_BeginWithPositionAndSize(MonoString* text, Math::vec2* position, Math::vec2* size)
	{
		char* textCStr = mono_string_to_utf8(text);

		Gui::SetNextWindowPos({ position->x, position->y });
		Gui::SetNextWindowSize({ size->x, size->y });
		BeginWindow(textCStr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		mono_free(textCStr);
	}

	static void Gui_End()
	{
		Gui::End();
	}

	static void Gui_Separator()
	{
		Gui::Separator();
	}

	static void Gui_Spacing()
	{
		Gui::Spacing();
	}

	static void Gui_Text(MonoString* text)
	{
		char* textCStr = mono_string_to_utf8(text);

		Gui::Text(textCStr);

		mono_free(textCStr);
	}

	static bool Gui_Button(MonoString* text)
	{
		char* textCStr = mono_string_to_utf8(text);

		bool result = Gui::Button(textCStr);

		mono_free(textCStr);

		return result;
	}

#pragma endregion

#pragma region Debug

	static void Debug_Log(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		VX_TRACE("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Info(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		VX_INFO("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Warn(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		VX_WARN("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Error(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		VX_ERROR("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Critical(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		VX_CRITICAL("{}", managedString);

		mono_free(managedString);
	}

#pragma endregion

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
				VX_CORE_ERROR("Could not find Component type {}", managedTypename);
				return;
			}

			s_EntityAddComponentFuncs[managedType] = [](Entity entity) { entity.AddComponent<TComponent>(); };
			s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<TComponent>(); };
			s_EntityRemoveComponentFuncs[managedType] = [](Entity entity) { entity.RemoveComponent<TComponent>(); };
		}(), ...);
	}

	template <typename... TComponent>
	static void RegisterComponent(ComponentGroup<TComponent...>)
	{
		RegisterComponent<TComponent...>();
	}

	void ScriptRegistry::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		s_EntityAddComponentFuncs.clear();
		s_EntityRemoveComponentFuncs.clear();

		RegisterComponent(AllComponents{});
	}

	void ScriptRegistry::SetHoveredEntity(Entity entity)
	{
		s_HoveredEntity = entity;
	}

	void ScriptRegistry::SetSceneStartTime(float startTime)
	{
		s_SceneStartTime = startTime;
	}

	void ScriptRegistry::SetActiveSceneName(const std::string& sceneName)
	{
		s_ActiveSceneName = sceneName;
	}

	const char* ScriptRegistry::GetSceneToBeLoaded()
	{
		const char* sceneName = s_SceneToBeLoaded.c_str();
		return sceneName;
	}

	void ScriptRegistry::ResetSceneToBeLoaded()
	{
		s_SceneToBeLoaded.clear();
	}

	void ScriptRegistry::RegisterMethods()
	{
		VX_ADD_INTERNAL_CALL(Application_Quit);
		VX_ADD_INTERNAL_CALL(Application_GetSize);
		VX_ADD_INTERNAL_CALL(Application_GetPosition);
		VX_ADD_INTERNAL_CALL(Application_IsMaximized);

		VX_ADD_INTERNAL_CALL(DebugRenderer_BeginScene);
		VX_ADD_INTERNAL_CALL(DebugRenderer_SetClearColor);
		VX_ADD_INTERNAL_CALL(DebugRenderer_DrawLine);
		VX_ADD_INTERNAL_CALL(DebugRenderer_DrawQuadBillboard);
		VX_ADD_INTERNAL_CALL(DebugRenderer_DrawBoundingBox);
		VX_ADD_INTERNAL_CALL(DebugRenderer_Flush);

		VX_ADD_INTERNAL_CALL(Scene_FindEntityByID);
		VX_ADD_INTERNAL_CALL(Scene_Instantiate);
		VX_ADD_INTERNAL_CALL(Scene_IsPaused);
		VX_ADD_INTERNAL_CALL(Scene_Pause);
		VX_ADD_INTERNAL_CALL(Scene_Resume);
		VX_ADD_INTERNAL_CALL(Scene_GetHoveredEntity);

		VX_ADD_INTERNAL_CALL(SceneManager_LoadScene);
		VX_ADD_INTERNAL_CALL(SceneManager_GetActiveScene);

		VX_ADD_INTERNAL_CALL(Entity_AddComponent);
		VX_ADD_INTERNAL_CALL(Entity_HasComponent);
		VX_ADD_INTERNAL_CALL(Entity_RemoveComponent);
		VX_ADD_INTERNAL_CALL(Entity_GetChildren);
		VX_ADD_INTERNAL_CALL(Entity_GetChild);
		VX_ADD_INTERNAL_CALL(Entity_GetTag);
		VX_ADD_INTERNAL_CALL(Entity_GetMarker);
		VX_ADD_INTERNAL_CALL(Entity_CreateWithName);
		VX_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		VX_ADD_INTERNAL_CALL(Entity_AddChild);
		VX_ADD_INTERNAL_CALL(Entity_RemoveChild);
		VX_ADD_INTERNAL_CALL(Entity_GetScriptInstance);
		VX_ADD_INTERNAL_CALL(Entity_Destroy);
		VX_ADD_INTERNAL_CALL(Entity_SetActive);

		VX_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		VX_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		VX_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		VX_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		VX_ADD_INTERNAL_CALL(TransformComponent_SetRotationQuaterion);
		VX_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		VX_ADD_INTERNAL_CALL(TransformComponent_SetScale);
		VX_ADD_INTERNAL_CALL(TransformComponent_GetWorldSpaceTransform);
		VX_ADD_INTERNAL_CALL(TransformComponent_GetForwardDirection);
		VX_ADD_INTERNAL_CALL(TransformComponent_GetUpDirection);
		VX_ADD_INTERNAL_CALL(TransformComponent_GetRightDirection);
		VX_ADD_INTERNAL_CALL(TransformComponent_LookAt);
		VX_ADD_INTERNAL_CALL(TransformComponent_GetParent);
		VX_ADD_INTERNAL_CALL(TransformComponent_SetParent);
		VX_ADD_INTERNAL_CALL(TransformComponent_Unparent);
		VX_ADD_INTERNAL_CALL(TransformComponent_Multiply);

		VX_ADD_INTERNAL_CALL(CameraComponent_GetPrimary);
		VX_ADD_INTERNAL_CALL(CameraComponent_SetPrimary);
		VX_ADD_INTERNAL_CALL(CameraComponent_GetPerspectiveVerticalFOV);
		VX_ADD_INTERNAL_CALL(CameraComponent_SetPerspectiveVerticalFOV);
		VX_ADD_INTERNAL_CALL(CameraComponent_GetFixedAspectRatio);
		VX_ADD_INTERNAL_CALL(CameraComponent_SetFixedAspectRatio);

		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetRadiance);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetRadiance);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetIntensity);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetIntensity);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetShadowBias);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetShadowBias);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetCastShadows);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetCastShadows);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetCutoff);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetCutoff);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetOuterCutoff);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetOuterCutoff);

		VX_ADD_INTERNAL_CALL(TextMeshComponent_GetTextString);
		VX_ADD_INTERNAL_CALL(TextMeshComponent_SetTextString);
		VX_ADD_INTERNAL_CALL(TextMeshComponent_GetColor);
		VX_ADD_INTERNAL_CALL(TextMeshComponent_SetColor);
		VX_ADD_INTERNAL_CALL(TextMeshComponent_GetLineSpacing);
		VX_ADD_INTERNAL_CALL(TextMeshComponent_SetLineSpacing);
		VX_ADD_INTERNAL_CALL(TextMeshComponent_GetKerning);
		VX_ADD_INTERNAL_CALL(TextMeshComponent_SetKerning);
		VX_ADD_INTERNAL_CALL(TextMeshComponent_GetMaxWidth);
		VX_ADD_INTERNAL_CALL(TextMeshComponent_SetMaxWidth);

		VX_ADD_INTERNAL_CALL(AnimatorComponent_IsPlaying);
		VX_ADD_INTERNAL_CALL(AnimatorComponent_Play);

		VX_ADD_INTERNAL_CALL(MeshRendererComponent_GetMeshType);
		VX_ADD_INTERNAL_CALL(MeshRendererComponent_SetMeshType);
		VX_ADD_INTERNAL_CALL(MeshRendererComponent_GetScale);
		VX_ADD_INTERNAL_CALL(MeshRendererComponent_SetScale);

		VX_ADD_INTERNAL_CALL(Material_GetAlbedo);
		VX_ADD_INTERNAL_CALL(Material_SetAlbedo);
		VX_ADD_INTERNAL_CALL(Material_GetMetallic);
		VX_ADD_INTERNAL_CALL(Material_SetMetallic);
		VX_ADD_INTERNAL_CALL(Material_GetRoughness);
		VX_ADD_INTERNAL_CALL(Material_SetRoughness);
		VX_ADD_INTERNAL_CALL(Material_GetEmission);
		VX_ADD_INTERNAL_CALL(Material_SetEmission);

		VX_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		VX_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);
		VX_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTexture);
		VX_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture);
		VX_ADD_INTERNAL_CALL(SpriteRendererComponent_GetScale);
		VX_ADD_INTERNAL_CALL(SpriteRendererComponent_SetScale);

		VX_ADD_INTERNAL_CALL(CircleRendererComponent_GetColor);
		VX_ADD_INTERNAL_CALL(CircleRendererComponent_SetColor);
		VX_ADD_INTERNAL_CALL(CircleRendererComponent_GetThickness);
		VX_ADD_INTERNAL_CALL(CircleRendererComponent_SetThickness);
		VX_ADD_INTERNAL_CALL(CircleRendererComponent_GetFade);
		VX_ADD_INTERNAL_CALL(CircleRendererComponent_SetFade);

		VX_ADD_INTERNAL_CALL(AudioSourceComponent_GetIsPlaying);
		VX_ADD_INTERNAL_CALL(AudioSourceComponent_Play);
		VX_ADD_INTERNAL_CALL(AudioSourceComponent_Stop);

		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetTranslation);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetTranslation);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetRotation);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetRotation);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_Translate);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_Rotate);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_LookAt);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetBodyType);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetBodyType);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetCollisionDetectionType);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetCollisionDetectionType);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetMass);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetMass);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetLinearVelocity);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetLinearVelocity);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetLinearDrag);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetLinearDrag);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetAngularVelocity);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetAngularVelocity);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetAngularDrag);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetAngularDrag);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetDisableGravity);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetDisableGravity);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetIsKinematic);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetIsKinematic);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_AddForce);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_AddForceAtPosition);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_AddTorque);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_ClearTorque);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_ClearForce);

		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_Move);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_Jump); 
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_IsGrounded);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_GetDisableGravity);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_SetDisableGravity);

		VX_ADD_INTERNAL_CALL(BoxColliderComponent_GetHalfSize);
		VX_ADD_INTERNAL_CALL(BoxColliderComponent_SetHalfSize);
		VX_ADD_INTERNAL_CALL(BoxColliderComponent_GetOffset);
		VX_ADD_INTERNAL_CALL(BoxColliderComponent_SetOffset);
		VX_ADD_INTERNAL_CALL(BoxColliderComponent_GetIsTrigger);
		VX_ADD_INTERNAL_CALL(BoxColliderComponent_SetIsTrigger);

		VX_ADD_INTERNAL_CALL(SphereColliderComponent_GetRadius);
		VX_ADD_INTERNAL_CALL(SphereColliderComponent_SetRadius);
		VX_ADD_INTERNAL_CALL(SphereColliderComponent_GetOffset);
		VX_ADD_INTERNAL_CALL(SphereColliderComponent_SetOffset);
		VX_ADD_INTERNAL_CALL(SphereColliderComponent_GetIsTrigger);
		VX_ADD_INTERNAL_CALL(SphereColliderComponent_SetIsTrigger);

		VX_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetRadius);
		VX_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetRadius);
		VX_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetHeight);
		VX_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetHeight);
		VX_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetOffset);
		VX_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetOffset);
		VX_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetIsTrigger);
		VX_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetIsTrigger);

		VX_ADD_INTERNAL_CALL(Physics_Raycast);
		VX_ADD_INTERNAL_CALL(Physics_GetSceneGravity);
		VX_ADD_INTERNAL_CALL(Physics_SetSceneGravity);
		VX_ADD_INTERNAL_CALL(Physics_GetScenePositionIterations);
		VX_ADD_INTERNAL_CALL(Physics_SetScenePositionIterations);
		VX_ADD_INTERNAL_CALL(Physics_GetSceneVelocityIterations);
		VX_ADD_INTERNAL_CALL(Physics_SetSceneVelocityIterations);

		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_GetBodyType);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_SetBodyType);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyForce);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyForceToCenter);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulseToCenter);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_GetVelocity);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_SetVelocity);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_GetDrag);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_SetDrag);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_GetFixedRotation);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_SetFixedRotation);

		VX_ADD_INTERNAL_CALL(Physics2D_Raycast);
		VX_ADD_INTERNAL_CALL(Physics2D_GetWorldGravity);
		VX_ADD_INTERNAL_CALL(Physics2D_SetWorldGravity);
		VX_ADD_INTERNAL_CALL(Physics2D_GetWorldPositionIterations);
		VX_ADD_INTERNAL_CALL(Physics2D_SetWorldPositionIterations);
		VX_ADD_INTERNAL_CALL(Physics2D_GetWorldVelocityIterations);
		VX_ADD_INTERNAL_CALL(Physics2D_SetWorldVelocityIterations);

		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetOffset);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetOffset);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetSize);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetSize);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetDensity);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetDensity);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetFriction);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetFriction);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitution);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitution);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitutionThreshold);
		VX_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitutionThreshold);

		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetOffset);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetOffset);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRadius);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRadius);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetDensity);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetDensity);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetFriction);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetFriction);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRestitution);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRestitution);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRestitutionThreshold);
		VX_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRestitutionThreshold);

		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetVelocity);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetVelocity);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetVelocityVariation);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetVelocityVariation);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetOffset);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetOffset);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetSizeBegin);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetSizeBegin);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetSizeEnd);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetSizeEnd);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetSizeVariation);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetSizeVariation);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetColorBegin);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetColorBegin);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetColorEnd);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetColorEnd);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetRotation);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetRotation);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetLifeTime);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetLifeTime);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_Start);
		VX_ADD_INTERNAL_CALL(ParticleEmitterComponent_Stop);

		VX_ADD_INTERNAL_CALL(RandomDevice_RangedInt32);
		VX_ADD_INTERNAL_CALL(RandomDevice_RangedFloat);

		VX_ADD_INTERNAL_CALL(Mathf_GetPI);
		VX_ADD_INTERNAL_CALL(Mathf_GetPI_D);
		VX_ADD_INTERNAL_CALL(Mathf_Sqrt);
		VX_ADD_INTERNAL_CALL(Mathf_Sin);
		VX_ADD_INTERNAL_CALL(Mathf_Cos);
		VX_ADD_INTERNAL_CALL(Mathf_Acos);
		VX_ADD_INTERNAL_CALL(Mathf_Tan);
		VX_ADD_INTERNAL_CALL(Mathf_Max);
		VX_ADD_INTERNAL_CALL(Mathf_Max);
		VX_ADD_INTERNAL_CALL(Mathf_Deg2Rad);
		VX_ADD_INTERNAL_CALL(Mathf_Rad2Deg);
		VX_ADD_INTERNAL_CALL(Mathf_Deg2RadVector3);
		VX_ADD_INTERNAL_CALL(Mathf_Rad2DegVector3);

		VX_ADD_INTERNAL_CALL(Vector3_CrossProductVec3);
		VX_ADD_INTERNAL_CALL(Vector3_DotProductVec3);

		VX_ADD_INTERNAL_CALL(Time_GetElapsed);
		VX_ADD_INTERNAL_CALL(Time_GetDeltaTime);

		VX_ADD_INTERNAL_CALL(Input_IsKeyDown);
		VX_ADD_INTERNAL_CALL(Input_IsKeyUp);
		VX_ADD_INTERNAL_CALL(Input_IsMouseButtonDown);
		VX_ADD_INTERNAL_CALL(Input_IsMouseButtonUp);
		VX_ADD_INTERNAL_CALL(Input_GetMousePosition);
		VX_ADD_INTERNAL_CALL(Input_GetMouseScrollOffset);
		VX_ADD_INTERNAL_CALL(Input_IsGamepadButtonDown);
		VX_ADD_INTERNAL_CALL(Input_IsGamepadButtonUp);
		VX_ADD_INTERNAL_CALL(Input_GetGamepadAxis);
		VX_ADD_INTERNAL_CALL(Input_ShowMouseCursor);

		VX_ADD_INTERNAL_CALL(Gui_Begin);
		VX_ADD_INTERNAL_CALL(Gui_BeginWithPosition);
		VX_ADD_INTERNAL_CALL(Gui_BeginWithSize);
		VX_ADD_INTERNAL_CALL(Gui_BeginWithPositionAndSize);
		VX_ADD_INTERNAL_CALL(Gui_End);
		VX_ADD_INTERNAL_CALL(Gui_Separator);
		VX_ADD_INTERNAL_CALL(Gui_Spacing);
		VX_ADD_INTERNAL_CALL(Gui_Text);
		VX_ADD_INTERNAL_CALL(Gui_Button);

		VX_ADD_INTERNAL_CALL(Debug_Log);
		VX_ADD_INTERNAL_CALL(Debug_Info);
		VX_ADD_INTERNAL_CALL(Debug_Warn);
		VX_ADD_INTERNAL_CALL(Debug_Error);
		VX_ADD_INTERNAL_CALL(Debug_Critical);
	}

}
