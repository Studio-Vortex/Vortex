#include "sppch.h"
#include "ScriptRegistry.h"

#include "Sparky/Core/Application.h"
#include "Sparky/Core/Input.h"
#include "Sparky/Core/UUID.h"

#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scripting/ScriptEngine.h"

#include "Sparky/Audio/AudioSource.h"

#include "Sparky/Physics/Physics.h"
#include "Sparky/Physics/Physics2D.h"
#include "Sparky/Physics/PhysXTypes.h"
#include "Sparky/Physics/PhysXAPIHelpers.h"

#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/Renderer2D.h"
#include "Sparky/Renderer/LightSource.h"
#include "Sparky/Renderer/ParticleEmitter.h"
#include "Sparky/Renderer/Model.h"

#include "Sparky/Utils/PlatformUtils.h"
#include "Sparky/Core/Log.h"

#include <mono/metadata/object.h>
#include <mono/jit/jit.h>
#include <mono/metadata/reflection.h>

#include <box2d/b2_body.h>
#include <box2d/b2_world.h>
#include <box2d/b2_fixture.h>

#include <imgui.h>

#include <cstdlib>
#include <ctime>

namespace Sparky {

#define SP_ADD_INTERNAL_CALL(icall) mono_add_internal_call("Sparky.InternalCalls::" #icall, icall)

	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityAddComponentFuncs;
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityRemoveComponentFuncs;

	static Entity s_HoveredEntity = Entity{};

	static std::string s_SceneToBeLoaded = "";

	static Math::vec4 s_RaycastDebugLineColor = Math::vec4(1.0f, 0.0f, 0.0f, 1.0f);

#pragma region Application

	static void Application_Quit()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Application::Get().Quit();
	}

	static void Application_GetSize(Math::vec2* outSize)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_CORE_TRACE(Application::Get().GetWindow().GetSize());
		*outSize = Application::Get().GetWindow().GetSize();
	}

	static void Application_GetPosition(Math::vec2* outPosition)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		*outPosition = Application::Get().GetWindow().GetPosition();
	}

	static bool Application_IsMaximized()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		return Application::Get().GetWindow().IsMaximized();
	}

#pragma endregion

#pragma region DebugRenderer

	static void DebugRenderer_BeginScene()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Entity primaryCameraEntity = contextScene->GetPrimaryCameraEntity();

		if (!primaryCameraEntity)
		{
			SP_CORE_WARN("Scene must include a primary camera to call debug render functions!");
			return;
		}

		SceneCamera& camera = primaryCameraEntity.GetComponent<CameraComponent>().Camera;
		Renderer2D::BeginScene(camera, primaryCameraEntity.GetTransform().GetTransform());
	}

	static void DebugRenderer_SetClearColor(Math::vec3* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		RenderCommand::SetClearColor(*color);
	}

	static void DebugRenderer_DrawLine(Math::vec3* p1, Math::vec3* p2, Math::vec4* color)
	{
		Renderer2D::DrawLine(*p1, *p2, *color);
	}

	static void DebugRenderer_DrawQuadBillboard(Math::vec3* translation, Math::vec2* size, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity primaryCameraEntity = contextScene->GetPrimaryCameraEntity();

		if (!primaryCameraEntity)
		{
			SP_CORE_WARN("Scene must include a primary camera to call debug render functions!");
			return;
		}

		Math::mat4 cameraView = Math::Inverse(primaryCameraEntity.GetTransform().GetTransform());

		Renderer2D::DrawQuadBillboard(cameraView, *translation, *size, *color);
	}

	static void DebugRenderer_Flush()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Renderer2D::EndScene();
	}

#pragma endregion

#pragma region Scene

	static bool Scene_FindEntityByID(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		if (entity)
		{
			return true;
		}

		return false;
	}

	static uint64_t Scene_Instantiate(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		if (!entity)
		{
			SP_CORE_WARN("Scene.Instantiate called with Invalid Entity UUID!");
			return 0;
		}

		Entity clonedEntity = contextScene->DuplicateEntity(entity);
		UUID uuid = clonedEntity.GetUUID();
		return uuid;
	}

	static bool Scene_IsPaused()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		return contextScene->IsPaused();
	}

	static void Scene_Pause()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		contextScene->SetPaused(true);
	}

	static void Scene_Resume()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		char* sceneNameCStr = mono_string_to_utf8(sceneName);
		s_SceneToBeLoaded = std::string(sceneNameCStr);
		mono_free(sceneNameCStr);
	}

#pragma endregion

#pragma region Entity

	static void Entity_AddComponent(UUID entityUUID, MonoReflectionType* componentType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		SP_CORE_ASSERT(s_EntityAddComponentFuncs.find(managedType) != s_EntityAddComponentFuncs.end(), "Managed type was not found in Map!");
		
		s_EntityAddComponentFuncs.at(managedType)(entity);
	}

	static bool Entity_HasComponent(UUID entityUUID, MonoReflectionType* componentType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		SP_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end(), "Managed type was not found in Map!");

		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	static void Entity_RemoveComponent(UUID entityUUID, MonoReflectionType* componentType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		SP_CORE_ASSERT(s_EntityRemoveComponentFuncs.find(managedType) != s_EntityRemoveComponentFuncs.end(), "Managed type was not found in Map!");

		s_EntityRemoveComponentFuncs.at(managedType)(entity);
	}

	static MonoArray* Entity_GetChildren(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& children = entity.Children();

		MonoClass* coreEntityClass =  ScriptEngine::GetCoreEntityClass().GetMonoClass();
		SP_CORE_ASSERT(coreEntityClass, "Invalid Entity Class!");

		MonoArray* result = mono_array_new(mono_domain_get(), coreEntityClass, children.size());

		for (uint32_t i = 0; i < children.size(); i++)
		{
			uintptr_t length = mono_array_length(result);

			if (i >= length)
			{
				SP_CORE_WARN("Index out of bounds in C# array!");
				return nullptr;
			}

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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& children = entity.Children();
		if (index < children.size())
		{
			SP_CORE_ASSERT(false, "Index out of bounds!");
			return 0;
		}

		return (uint64_t)children[index];
	}

	static MonoString* Entity_GetTag(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return mono_string_new(mono_domain_get(), entity.GetName().c_str());
	}

	static MonoString* Entity_GetMarker(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return mono_string_new(mono_domain_get(), entity.GetMarker().c_str());
	}

	static uint64_t Entity_CreateWithName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->CreateEntity(nameCStr);
		mono_free(nameCStr);

		return entity.GetUUID();
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->FindEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity.GetUUID();
	}

	static bool Entity_AddChild(UUID parentUUID, UUID childUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity parent = contextScene->TryGetEntityWithUUID(parentUUID);
		Entity child = contextScene->TryGetEntityWithUUID(childUUID);

		if (parent && child)
		{
			parent.AddChild(child.GetUUID());
			return true;
		}

		SP_CORE_WARN("Parent or Child UUID was Invalid!");
		return false;
	}

	static bool Entity_RemoveChild(UUID parentUUID, UUID childUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity parent = contextScene->TryGetEntityWithUUID(parentUUID);
		Entity child = contextScene->TryGetEntityWithUUID(childUUID);

		if (parent && child)
		{
			parent.RemoveChild(child.GetUUID());
			return true;
		}

		return false;
	}

	static MonoObject* Entity_GetScriptInstance(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		return ScriptEngine::GetManagedInstance(entityUUID);
	}

	static void Entity_Destroy(UUID entityUUID, bool excludeChildren)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		contextScene->DestroyEntity(entity, excludeChildren);
	}

	static void Entity_SetActive(UUID entityUUID, bool isActive)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.SetActive(isActive);
	}

#pragma endregion

#pragma region Transform Component

	static void TransformComponent_GetTranslation(UUID entityUUID, Math::vec3* outTranslation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;
	}

	static void TransformComponent_SetTranslation(UUID entityUUID, Math::vec3* translation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	static void TransformComponent_GetRotation(UUID entityUUID, Math::vec3* outRotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRotation = entity.GetComponent<TransformComponent>().GetRotationEuler();

		// Since we store rotation in radians we must convert to degrees here
		outRotation->x = Math::Rad2Deg(outRotation->x);
		outRotation->y = Math::Rad2Deg(outRotation->y);
		outRotation->z = Math::Rad2Deg(outRotation->z);
	}

	static void TransformComponent_SetRotation(UUID entityUUID, Math::vec3* rotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		// Since we store rotation in radians we must convert to radians here
		rotation->x = Math::Deg2Rad(rotation->x);
		rotation->y = Math::Deg2Rad(rotation->y);
		rotation->z = Math::Deg2Rad(rotation->z);

		entity.GetComponent<TransformComponent>().SetRotationEuler(*rotation);
	}

	static void TransformComponent_GetScale(UUID entityUUID, Math::vec3* outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	static void TransformComponent_SetScale(UUID entityUUID, Math::vec3* scale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TransformComponent>().Scale = *scale;
	}

	static void TransformComponent_GetWorldSpaceTransform(UUID entityUUID, Math::vec3* outTranslation, Math::vec3* outRotationEuler, Math::vec3* outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		TransformComponent worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);
		*outTranslation = worldSpaceTransform.Translation;
		*outRotationEuler = worldSpaceTransform.GetRotationEuler();
		*outScale = worldSpaceTransform.Scale;
	}

	static void TransformComponent_GetForwardDirection(UUID entityUUID, Math::vec3* outDirection)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

		Math::vec3 rotation = worldSpaceTransform.GetRotationEuler();

		*outDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), Math::vec3(0.0f, 0.0f, -1.0f));
	}

	static void TransformComponent_GetUpDirection(UUID entityUUID, Math::vec3* outDirection)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

		Math::vec3 rotation = worldSpaceTransform.GetRotationEuler();

		*outDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), Math::vec3(0.0f, 1.0f, 0.0f));
	}

	static void TransformComponent_GetRightDirection(UUID entityUUID, Math::vec3* outDirection)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

		Math::vec3 rotation = worldSpaceTransform.GetRotationEuler();

		*outDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), Math::vec3(1.0f, 0.0f, 0.0f));
	}

	static void TransformComponent_LookAt(UUID entityUUID, Math::vec3* worldPoint)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		TransformComponent& transform = entity.GetTransform();
		Math::vec3 upDirection(0.0f, 1.0f, 0.0f);
		Math::mat4 result = Math::LookAt(transform.Translation, *worldPoint, upDirection);
		Math::vec3 translation, rotation, scale;
		Math::DecomposeTransform(Math::Inverse(result), translation, rotation, scale);
		transform.Translation = translation;
		transform.SetRotationEuler(rotation);
		transform.Scale = scale;
	}
	
	static void TransformComponent_SetParent(UUID childUUID, UUID parentUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity child = contextScene->TryGetEntityWithUUID(childUUID);
		SP_CORE_ASSERT(child, "Invalid Child UUID!");

		Entity parent = contextScene->TryGetEntityWithUUID(parentUUID);
		SP_CORE_ASSERT(parent, "Invalid Parent UUID!");

		contextScene->ParentEntity(child, parent);
	}

	static void TransformComponent_Unparent(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outPrimary = entity.GetComponent<CameraComponent>().Primary;
	}

	static void CameraComponent_SetPrimary(UUID entityUUID, bool primary)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CameraComponent>().Primary = primary;
	}
	
	static float CameraComponent_GetPerspectiveVerticalFOV(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return Math::Rad2Deg(entity.GetComponent<CameraComponent>().Camera.GetPerspectiveVerticalFOV());
	}

	static void CameraComponent_SetPerspectiveVerticalFOV(UUID entityUUID, float perspectiveVerticalFOV)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CameraComponent>().Camera.SetPerspectiveVerticalFOV(Math::Deg2Rad(perspectiveVerticalFOV));
	}

	static void CameraComponent_GetFixedAspectRatio(UUID entityUUID, bool* outFixedAspectRatio)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outFixedAspectRatio = entity.GetComponent<CameraComponent>().FixedAspectRatio;
	}
	
	static void CameraComponent_SetFixedAspectRatio(UUID entityUUID, bool fixedAspectRatio)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CameraComponent>().FixedAspectRatio = fixedAspectRatio;
	}

#pragma endregion

#pragma region Light Source Component

	static void LightSourceComponent_GetAmbient(UUID entityUUID, Math::vec3* outAmbient)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outAmbient = entity.GetComponent<LightSourceComponent>().Source->GetAmbient();
	}

	static void LightSourceComponent_SetAmbient(UUID entityUUID, Math::vec3* ambient)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetAmbient(*ambient);
	}

	static void LightSourceComponent_GetDiffuse(UUID entityUUID, Math::vec3* outDiffuse)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outDiffuse = entity.GetComponent<LightSourceComponent>().Source->GetDiffuse();
	}

	static void LightSourceComponent_SetDiffuse(UUID entityUUID, Math::vec3* diffuse)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetDiffuse(*diffuse);
	}

	static void LightSourceComponent_GetSpecular(UUID entityUUID, Math::vec3* outSpecular)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSpecular = entity.GetComponent<LightSourceComponent>().Source->GetSpecular();
	}

	static void LightSourceComponent_SetSpecular(UUID entityUUID, Math::vec3* specular)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetSpecular(*specular);
	}

	static void LightSourceComponent_GetColor(UUID entityUUID, Math::vec3* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<LightSourceComponent>().Source->GetColor();
	}

	static void LightSourceComponent_SetColor(UUID entityUUID, Math::vec3* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetColor(*color);
	}

	static void LightSourceComponent_GetDirection(UUID entityUUID, Math::vec3* outDirection)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outDirection = entity.GetComponent<LightSourceComponent>().Source->GetDirection();
	}

	static void LightSourceComponent_SetDirection(UUID entityUUID, Math::vec3* direction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<LightSourceComponent>().Source->SetDirection(*direction);
	}

#pragma endregion

#pragma region TextMesh Component

	static MonoString* TextMeshComponent_GetTextString(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return mono_string_new(mono_domain_get(), entity.GetComponent<TextMeshComponent>().TextString.c_str());
	}

	static void TextMeshComponent_SetTextString(UUID entityUUID, MonoString* textString)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		char* textCStr = mono_string_to_utf8(textString);

		entity.GetComponent<TextMeshComponent>().TextString = std::string(textCStr);
		mono_free(textCStr);
	}

	static void TextMeshComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<TextMeshComponent>().Color;
	}

	static void TextMeshComponent_SetColor(UUID entityUUID, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TextMeshComponent>().Color = *color;
	}

	static float TextMeshComponent_GetLineSpacing(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<TextMeshComponent>().LineSpacing;
	}

	static void TextMeshComponent_SetLineSpacing(UUID entityUUID, float lineSpacing)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TextMeshComponent>().LineSpacing = lineSpacing;
	}

	static float TextMeshComponent_GetKerning(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<TextMeshComponent>().Kerning;
	}

	static void TextMeshComponent_SetKerning(UUID entityUUID, float kerning)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TextMeshComponent>().Kerning = kerning;
	}

	static float TextMeshComponent_GetMaxWidth(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<TextMeshComponent>().MaxWidth;
	}

	static void TextMeshComponent_SetMaxWidth(UUID entityUUID, float maxWidth)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TextMeshComponent>().MaxWidth = maxWidth;
	}

#pragma endregion

#pragma region Mesh Renderer Component

	static MeshType MeshRendererComponent_GetMeshType(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<MeshRendererComponent>().Type;
	}

	static void MeshRendererComponent_SetMeshType(UUID entityUUID, MeshType meshType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		if (meshType != MeshType::Custom)
		{
			MeshRendererComponent& meshRenderer = entity.GetComponent<MeshRendererComponent>();
			meshRenderer.Type = meshType;
			meshRenderer.Mesh = Model::Create(Model::DefaultMeshSourcePaths[static_cast<uint32_t>(meshType)], MaterialInstance::Create(), entity.GetTransform(), (int)(entt::entity)entity);
		}
	}

	static void MeshRendererComponent_GetScale(UUID entityUUID, Math::vec2* outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outScale = entity.GetComponent<MeshRendererComponent>().Scale;
	}

	static void MeshRendererComponent_SetScale(UUID entityUUID, Math::vec2* scale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<MeshRendererComponent>().Scale = *scale;
	}

#pragma endregion

#pragma region Material
	
	static void Material_GetAlbedo(UUID entityUUID, Math::vec3* outAlbedo)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outAlbedo = entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->GetAlbedo();
	}

	static void Material_SetAlbedo(UUID entityUUID, Math::vec3* albedo)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->SetAlbedo(*albedo);
	}

	static float Material_GetMetallic(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->GetMetallic();
	}

	static void Material_SetMetallic(UUID entityUUID, float metallic)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->SetMetallic(metallic);
	}

	static float Material_GetRoughness(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->GetRoughness();
	}

	static void Material_SetRoughness(UUID entityUUID, float roughness)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->SetRoughness(roughness);
	}

#pragma endregion

#pragma region Sprite Renderer Component

	static void SpriteRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<SpriteRendererComponent>().SpriteColor;
	}

	static void SpriteRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteRendererComponent>().SpriteColor = *color;
	}

	static void SpriteRendererComponent_GetScale(UUID entityUUID, Math::vec2* outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outScale = entity.GetComponent<SpriteRendererComponent>().Scale;
	}

	static MonoString* SpriteRendererComponent_GetTexture(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const std::string& texturePath = entity.GetComponent<SpriteRendererComponent>().Texture->GetPath();

		return mono_string_new(mono_domain_get(), texturePath.c_str());
	}

	static void SpriteRendererComponent_SetTexture(UUID entityUUID, MonoString* texturePathString)
	{
		char* texturePathCStr = mono_string_to_utf8(texturePathString);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteRendererComponent>().Texture = Texture2D::Create(std::string(texturePathCStr));

		mono_free(texturePathCStr);
	}

	static void SpriteRendererComponent_SetScale(UUID entityUUID, Math::vec2* scale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteRendererComponent>().Scale = *scale;
	}

#pragma endregion

#pragma region Circle Renderer Component

	static void CircleRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<CircleRendererComponent>().Color;
	}

	static void CircleRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleRendererComponent>().Color = *color;
	}

	static void CircleRendererComponent_GetThickness(UUID entityUUID, float* outThickness)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outThickness = entity.GetComponent<CircleRendererComponent>().Thickness;
	}

	static void CircleRendererComponent_SetThickness(UUID entityUUID, float thickness)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleRendererComponent>().Thickness = thickness;
	}

	static void CircleRendererComponent_GetFade(UUID entityUUID, float* outFade)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outFade = entity.GetComponent<CircleRendererComponent>().Fade;
	}

	static void CircleRendererComponent_SetFade(UUID entityUUID, float fade)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleRendererComponent>().Fade = fade;
	}

#pragma endregion

#pragma region Particle Emitter Component

	static void ParticleEmitterComponent_GetVelocity(UUID entityUUID, Math::vec3* outVelocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocity = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Velocity;
	}

	static void ParticleEmitterComponent_SetVelocity(UUID entityUUID, Math::vec3* velocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Velocity = *velocity;
	}

	static void ParticleEmitterComponent_GetVelocityVariation(UUID entityUUID, Math::vec3* outVelocityVariation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocityVariation = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().VelocityVariation;
	}

	static void ParticleEmitterComponent_SetVelocityVariation(UUID entityUUID, Math::vec3* velocityVariation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().VelocityVariation = *velocityVariation;
	}

	static void ParticleEmitterComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Offset;
	}

	static void ParticleEmitterComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Offset = *offset;
	}

	static void ParticleEmitterComponent_GetSizeBegin(UUID entityUUID, Math::vec2* outSizeBegin)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSizeBegin = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeBegin;
	}

	static void ParticleEmitterComponent_SetSizeBegin(UUID entityUUID, Math::vec2* sizeBegin)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeBegin = *sizeBegin;
	}

	static void ParticleEmitterComponent_GetSizeEnd(UUID entityUUID, Math::vec2* outSizeEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSizeEnd = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeEnd;
	}

	static void ParticleEmitterComponent_SetSizeEnd(UUID entityUUID, Math::vec2* sizeEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeEnd = *sizeEnd;
	}

	static void ParticleEmitterComponent_GetSizeVariation(UUID entityUUID, Math::vec2* outSizeVariation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSizeVariation = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeVariation;
	}

	static void ParticleEmitterComponent_SetSizeVariation(UUID entityUUID, Math::vec2* sizeVariation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeVariation = *sizeVariation;
	}

	static void ParticleEmitterComponent_GetColorBegin(UUID entityUUID, Math::vec4* outColorBegin)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColorBegin = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorBegin;
	}

	static void ParticleEmitterComponent_SetColorBegin(UUID entityUUID, Math::vec4* colorBegin)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorBegin = *colorBegin;
	}

	static void ParticleEmitterComponent_GetColorEnd(UUID entityUUID, Math::vec4* outColorEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColorEnd = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorEnd;
	}

	static void ParticleEmitterComponent_SetColorEnd(UUID entityUUID, Math::vec4* colorEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorEnd = *colorEnd;
	}

	static void ParticleEmitterComponent_GetRotation(UUID entityUUID, float* outRotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRotation = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Rotation;
	}

	static void ParticleEmitterComponent_SetRotation(UUID entityUUID, float colorEnd)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Rotation = colorEnd;
	}

	static void ParticleEmitterComponent_GetLifeTime(UUID entityUUID, float* outLifeTime)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outLifeTime = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().LifeTime;
	}

	static void ParticleEmitterComponent_SetLifeTime(UUID entityUUID, float lifetime)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().LifeTime = lifetime;
	}

	static void ParticleEmitterComponent_Start(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->Start();
	}

	static void ParticleEmitterComponent_Stop(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<ParticleEmitterComponent>().Emitter->Stop();
	}

#pragma endregion

#pragma region AudioSource Component

	static bool AudioSourceComponent_GetIsPlaying(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<AudioSourceComponent>().Source->IsPlaying();
	}

	static void AudioSourceComponent_Play(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<AudioSourceComponent>().Source->Play();
	}

	static void AudioSourceComponent_Stop(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<AudioSourceComponent>().Source->Stop();
	}

#pragma endregion

#pragma region RigidBody Component

	static void RigidBodyComponent_GetTranslation(UUID entityUUID, Math::vec3* outTranslation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outTranslation = FromPhysXVector(((physx::PxRigidDynamic*)entity.GetComponent<RigidBodyComponent>().RuntimeActor)->getGlobalPose().p);
	}

	static void RigidBodyComponent_SetTranslation(UUID entityUUID, Math::vec3* translation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const auto& transformComponent = entity.GetTransform();
		Math::vec3 rotation = transformComponent.GetRotationEuler();
		auto entityTransform = TransformComponent{ *translation, rotation, transformComponent.Scale }.GetTransform();
		auto physxTransform = ToPhysXTransform(entityTransform);

		((physx::PxRigidDynamic*)entity.GetComponent<RigidBodyComponent>().RuntimeActor)->setGlobalPose(physxTransform);
	}

	static void RigidBodyComponent_GetRotation(UUID entityUUID, Math::vec3* outRotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRotation = Math::EulerAngles(FromPhysXQuat(((physx::PxRigidDynamic*)entity.GetComponent<RigidBodyComponent>().RuntimeActor)->getGlobalPose().q));
	}

	static void RigidBodyComponent_SetRotation(UUID entityUUID, Math::vec3* rotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		RigidBodyComponent& rigidBody = entity.GetComponent<RigidBodyComponent>();
		physx::PxRigidDynamic* actor = ((physx::PxRigidDynamic*)rigidBody.RuntimeActor);
		physx::PxTransform physxTransform = actor->getGlobalPose();
		physxTransform.q = ToPhysXQuat(Math::quaternion(*rotation));

		actor->setGlobalPose(physxTransform);
	}

	static void RigidBodyComponent_Translate(UUID entityUUID, Math::vec3* translation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		RigidBodyComponent& rigidBody = entity.GetComponent<RigidBodyComponent>();
		physx::PxRigidDynamic* actor = ((physx::PxRigidDynamic*)rigidBody.RuntimeActor);
		physx::PxTransform physxTransform = actor->getGlobalPose();
		physxTransform.p += ToPhysXVector(*translation);

		actor->setGlobalPose(physxTransform);
	}

	static void RigidBodyComponent_Rotate(UUID entityUUID, Math::vec3* rotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().Type;
	}

	static void RigidBodyComponent_SetBodyType(UUID entityUUID, RigidBodyType bodyType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

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

	static float RigidBodyComponent_GetMass(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().Mass;
	}

	static void RigidBodyComponent_SetMass(UUID entityUUID, float mass)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().Mass = mass;
	}

	static void RigidBodyComponent_GetLinearVelocity(UUID entityUUID, Math::vec3* outVelocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocity = entity.GetComponent<RigidBodyComponent>().LinearVelocity;
	}

	static void RigidBodyComponent_SetLinearVelocity(UUID entityUUID, Math::vec3* velocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().LinearVelocity = *velocity;
	}

	static float RigidBodyComponent_GetLinearDrag(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().LinearDrag;
	}

	static void RigidBodyComponent_SetLinearDrag(UUID entityUUID, float drag)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().LinearDrag = drag;
	}

	static void RigidBodyComponent_GetAngularVelocity(UUID entityUUID, Math::vec3* outVelocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocity = entity.GetComponent<RigidBodyComponent>().AngularVelocity;
	}

	static void RigidBodyComponent_SetAngularVelocity(UUID entityUUID, Math::vec3* velocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().AngularVelocity = *velocity;
	}

	static float RigidBodyComponent_GetAngularDrag(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().AngularDrag;
	}

	static void RigidBodyComponent_SetAngularDrag(UUID entityUUID, float drag)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().AngularDrag = drag;
	}

	static bool RigidBodyComponent_GetDisableGravity(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().DisableGravity;
	}

	static void RigidBodyComponent_SetDisableGravity(UUID entityUUID, bool disabled)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().DisableGravity = disabled;
	}

	static bool RigidBodyComponent_GetIsKinematic(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBodyComponent>().IsKinematic;
	}

	static void RigidBodyComponent_SetIsKinematic(UUID entityUUID, bool isKinematic)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBodyComponent>().IsKinematic = isKinematic;
	}

	static void RigidBodyComponent_AddForce(UUID entityUUID, Math::vec3* force, ForceMode mode)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

		if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
		{
			SP_CORE_WARN("Calling Rigidbody.AddForce with a non-dynamic Rigidbody!");
			return;
		}

		physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rigidbody.RuntimeActor);
		actor->addForce(ToPhysXVector(*force), (physx::PxForceMode::Enum)mode);
	}

	static void RigidBodyComponent_AddTorque(UUID entityUUID, Math::vec3* torque, ForceMode mode)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

		if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
		{
			SP_CORE_WARN("Calling Rigidbody.AddTorque with a non-dynamic Rigidbody!");
			return;
		}

		physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rigidbody.RuntimeActor);
		actor->addTorque(ToPhysXVector(*torque), (physx::PxForceMode::Enum)mode);
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

			outHit->EntityID = physicsBodyData->EntityUUID;
			outHit->Position = FromPhysXVector(hitInfo.block.position);
			outHit->Normal = FromPhysXVector(hitInfo.block.normal);
			outHit->Distance = hitInfo.block.distance;
		}

		return result;
	}

#pragma endregion

#pragma region Character Controller Component

	static void CharacterControllerComponent_Move(UUID entityUUID, Math::vec3* displacement)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID");

		CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
		characterController.SpeedDown = -1.0f * jumpForce;
	}

	static bool CharacterControllerComponent_IsGrounded(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID");

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

#pragma endregion

#pragma region BoxCollider Component

	static void BoxColliderComponent_GetHalfSize(UUID entityUUID, Math::vec3* outHalfSize)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outHalfSize = entity.GetComponent<BoxColliderComponent>().HalfSize;
	}
	
	static void BoxColliderComponent_SetHalfSize(UUID entityUUID, Math::vec3* halfSize)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxColliderComponent>().HalfSize = *halfSize;
	}

	static void BoxColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<BoxColliderComponent>().Offset;
	}

	static void BoxColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxColliderComponent>().Offset = *offset;
	}

	static bool BoxColliderComponent_GetIsTrigger(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<BoxColliderComponent>().IsTrigger;
	}

	static void BoxColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxColliderComponent>().IsTrigger = isTrigger;
	}

#pragma endregion

#pragma region SphereCollider Component

	static float SphereColliderComponent_GetRadius(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<SphereColliderComponent>().Radius;
	}

	static void SphereColliderComponent_SetRadius(UUID entityUUID, float radius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SphereColliderComponent>().Radius = radius;
	}

	static void SphereColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<SphereColliderComponent>().Offset;
	}

	static void SphereColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SphereColliderComponent>().Offset = *offset;
	}
	
	static bool SphereColliderComponent_GetIsTrigger(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<SphereColliderComponent>().IsTrigger;
	}
	
	static void SphereColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SphereColliderComponent>().IsTrigger = isTrigger;
	}

#pragma endregion

#pragma region CapsuleCollider Component

	static float CapsuleColliderComponent_GetRadius(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<CapsuleColliderComponent>().Radius;
	}
	
	static void CapsuleColliderComponent_SetRadius(UUID entityUUID, float radius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CapsuleColliderComponent>().Radius = radius;
	}
	
	static float CapsuleColliderComponent_GetHeight(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<CapsuleColliderComponent>().Height;
	}
	
	static void CapsuleColliderComponent_SetHeight(UUID entityUUID, float height)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CapsuleColliderComponent>().Height = height;
	}

	static void CapsuleColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<CapsuleColliderComponent>().Offset;
	}

	static void CapsuleColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CapsuleColliderComponent>().Offset = *offset;
	}

	static bool CapsuleColliderComponent_GetIsTrigger(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<CapsuleColliderComponent>().IsTrigger;
	}

	static void CapsuleColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CapsuleColliderComponent>().IsTrigger = isTrigger;
	}

#pragma endregion

#pragma region RigidBody2D Component

	static RigidBody2DType RigidBody2DComponent_GetBodyType(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBody2DComponent>().Type;
	}

	static void RigidBody2DComponent_SetBodyType(UUID entityUUID, RigidBody2DType bodyType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyForce(b2Vec2(force->x, force->y), b2Vec2(point->x, point->y), wake);
	}

	static void RigidBody2DComponent_ApplyForceToCenter(UUID entityUUID, Math::vec2* force, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
	}

	static void RigidBody2DComponent_ApplyLinearImpulse(UUID entityUUID, Math::vec2* impulse, Math::vec2* point, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}

	static void RigidBody2DComponent_ApplyLinearImpulseToCenter(UUID entityUUID, Math::vec2* impulse, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

	static void RigidBody2DComponent_GetVelocity(UUID entityUUID, Math::vec2* outVelocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outVelocity = entity.GetComponent<RigidBody2DComponent>().Velocity;
	}

	static void RigidBody2DComponent_SetVelocity(UUID entityUUID, Math::vec2* velocity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBody2DComponent>().Velocity = *velocity;
	}

	static float RigidBody2DComponent_GetDrag(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBody2DComponent>().Drag;
	}

	static void RigidBody2DComponent_SetDrag(UUID entityUUID, float drag)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<RigidBody2DComponent>().Drag = drag;
	}

	static bool RigidBody2DComponent_GetFixedRotation(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		return entity.GetComponent<RigidBody2DComponent>().FixedRotation;
	}

	static void RigidBody2DComponent_SetFixedRotation(UUID entityUUID, bool freeze)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

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
					ScriptEngine::OnCollisionEntity(entity); // Call the Entity's OnCollision Function
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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		
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

#pragma endregion

#pragma region Box Collider2D Component

	static void BoxCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<BoxCollider2DComponent>().Offset;
	}

	static void BoxCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().Offset = *offset;
	}
	
	static void BoxCollider2DComponent_GetSize(UUID entityUUID, Math::vec2* outSize)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSize = entity.GetComponent<BoxCollider2DComponent>().Size;
	}
	
	static void BoxCollider2DComponent_SetSize(UUID entityUUID, Math::vec2* size)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().Size = *size;
	}

	static void BoxCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outDensity = entity.GetComponent<BoxCollider2DComponent>().Density;
	}

	static void BoxCollider2DComponent_SetDensity(UUID entityUUID, float density)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetDensity(density);
		// Since we changed the density we must recalculate the mass data according to box2d
		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->GetBody()->ResetMassData();
	}

	static void BoxCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outFriction = entity.GetComponent<BoxCollider2DComponent>().Friction;
	}

	static void BoxCollider2DComponent_SetFriction(UUID entityUUID, float friction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetFriction(friction);
	}

	static void BoxCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRestitution = entity.GetComponent<BoxCollider2DComponent>().Restitution;
	}

	static void BoxCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetRestitution(restitution);
	}

	static void BoxCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRestitutionThreshold = entity.GetComponent<BoxCollider2DComponent>().RestitutionThreshold;
	}

	static void BoxCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetRestitutionThreshold(restitutionThreshold);
	}

#pragma endregion

#pragma region Circle Collider2D Component

	static void CircleCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<CircleCollider2DComponent>().Offset;
	}

	static void CircleCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().Offset = *offset;
	}

	static void CircleCollider2DComponent_GetRadius(UUID entityUUID, float* outRadius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRadius = entity.GetComponent<CircleCollider2DComponent>().Radius;
	}

	static void CircleCollider2DComponent_SetRadius(UUID entityUUID, float radius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().Radius = radius;
	}

	static void CircleCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outDensity = entity.GetComponent<CircleCollider2DComponent>().Density;
	}

	static void CircleCollider2DComponent_SetDensity(UUID entityUUID, float density)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetDensity(density);
		// Since we changed the density we must recalculate the mass data according to box2d
		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->GetBody()->ResetMassData();
	}

	static void CircleCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outFriction = entity.GetComponent<CircleCollider2DComponent>().Friction;
	}

	static void CircleCollider2DComponent_SetFriction(UUID entityUUID, float friction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetFriction(friction);
	}

	static void CircleCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRestitution = entity.GetComponent<CircleCollider2DComponent>().Restitution;
	}

	static void CircleCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetRestitution(restitution);
	}

	static void CircleCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRestitutionThreshold = entity.GetComponent<CircleCollider2DComponent>().RestitutionThreshold;
	}

	static void CircleCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->TryGetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetRestitution(restitutionThreshold);
	}

#pragma endregion

#pragma region Random

	static int RandomDevice_RangedInt32(int min, int max)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		std::random_device randomDevice;
		std::mt19937 engine(randomDevice());
		std::uniform_int_distribution<int> uniformDistribution(min, max);

		return uniformDistribution(engine);
	}

	static float RandomDevice_RangedFloat(float min, float max)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

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
		return Time::GetTime();
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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

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
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_TRACE("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Info(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_INFO("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Warn(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_WARN("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Error(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_ERROR("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Critical(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_CRITICAL("{}", managedString);

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
			std::string managedTypename = fmt::format("Sparky.{}", structName.substr(0, pos));

			MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());

			if (!managedType)
			{
				SP_CORE_ERROR("Could not find Component type {}", managedTypename);
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
		SP_ADD_INTERNAL_CALL(Application_Quit);
		SP_ADD_INTERNAL_CALL(Application_GetSize);
		SP_ADD_INTERNAL_CALL(Application_GetPosition);
		SP_ADD_INTERNAL_CALL(Application_IsMaximized);

		SP_ADD_INTERNAL_CALL(DebugRenderer_BeginScene);
		SP_ADD_INTERNAL_CALL(DebugRenderer_SetClearColor);
		SP_ADD_INTERNAL_CALL(DebugRenderer_DrawLine);
		SP_ADD_INTERNAL_CALL(DebugRenderer_DrawQuadBillboard);
		SP_ADD_INTERNAL_CALL(DebugRenderer_Flush);

		SP_ADD_INTERNAL_CALL(Scene_FindEntityByID);
		SP_ADD_INTERNAL_CALL(Scene_Instantiate);
		SP_ADD_INTERNAL_CALL(Scene_IsPaused);
		SP_ADD_INTERNAL_CALL(Scene_Pause);
		SP_ADD_INTERNAL_CALL(Scene_Resume);
		SP_ADD_INTERNAL_CALL(Scene_GetHoveredEntity);

		SP_ADD_INTERNAL_CALL(SceneManager_LoadScene);

		SP_ADD_INTERNAL_CALL(Entity_AddComponent);
		SP_ADD_INTERNAL_CALL(Entity_HasComponent);
		SP_ADD_INTERNAL_CALL(Entity_RemoveComponent);
		SP_ADD_INTERNAL_CALL(Entity_GetChildren);
		SP_ADD_INTERNAL_CALL(Entity_GetChild);
		SP_ADD_INTERNAL_CALL(Entity_GetTag);
		SP_ADD_INTERNAL_CALL(Entity_GetMarker);
		SP_ADD_INTERNAL_CALL(Entity_CreateWithName);
		SP_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		SP_ADD_INTERNAL_CALL(Entity_AddChild);
		SP_ADD_INTERNAL_CALL(Entity_RemoveChild);
		SP_ADD_INTERNAL_CALL(Entity_GetScriptInstance);
		SP_ADD_INTERNAL_CALL(Entity_Destroy);
		SP_ADD_INTERNAL_CALL(Entity_SetActive);

		SP_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		SP_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		SP_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		SP_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		SP_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		SP_ADD_INTERNAL_CALL(TransformComponent_SetScale);
		SP_ADD_INTERNAL_CALL(TransformComponent_GetWorldSpaceTransform);
		SP_ADD_INTERNAL_CALL(TransformComponent_GetForwardDirection);
		SP_ADD_INTERNAL_CALL(TransformComponent_GetUpDirection);
		SP_ADD_INTERNAL_CALL(TransformComponent_GetRightDirection);
		SP_ADD_INTERNAL_CALL(TransformComponent_LookAt);
		SP_ADD_INTERNAL_CALL(TransformComponent_SetParent);
		SP_ADD_INTERNAL_CALL(TransformComponent_Unparent);
		SP_ADD_INTERNAL_CALL(TransformComponent_Multiply);

		SP_ADD_INTERNAL_CALL(CameraComponent_GetPrimary);
		SP_ADD_INTERNAL_CALL(CameraComponent_SetPrimary);
		SP_ADD_INTERNAL_CALL(CameraComponent_GetPerspectiveVerticalFOV);
		SP_ADD_INTERNAL_CALL(CameraComponent_SetPerspectiveVerticalFOV);
		SP_ADD_INTERNAL_CALL(CameraComponent_GetFixedAspectRatio);
		SP_ADD_INTERNAL_CALL(CameraComponent_SetFixedAspectRatio);

		SP_ADD_INTERNAL_CALL(LightSourceComponent_GetAmbient);
		SP_ADD_INTERNAL_CALL(LightSourceComponent_SetAmbient);
		SP_ADD_INTERNAL_CALL(LightSourceComponent_GetDiffuse);
		SP_ADD_INTERNAL_CALL(LightSourceComponent_SetDiffuse);
		SP_ADD_INTERNAL_CALL(LightSourceComponent_GetSpecular);
		SP_ADD_INTERNAL_CALL(LightSourceComponent_SetSpecular);
		SP_ADD_INTERNAL_CALL(LightSourceComponent_GetColor);
		SP_ADD_INTERNAL_CALL(LightSourceComponent_SetColor);
		SP_ADD_INTERNAL_CALL(LightSourceComponent_GetDirection);
		SP_ADD_INTERNAL_CALL(LightSourceComponent_SetDirection);

		SP_ADD_INTERNAL_CALL(TextMeshComponent_GetTextString);
		SP_ADD_INTERNAL_CALL(TextMeshComponent_SetTextString);
		SP_ADD_INTERNAL_CALL(TextMeshComponent_GetColor);
		SP_ADD_INTERNAL_CALL(TextMeshComponent_SetColor);
		SP_ADD_INTERNAL_CALL(TextMeshComponent_GetLineSpacing);
		SP_ADD_INTERNAL_CALL(TextMeshComponent_SetLineSpacing);
		SP_ADD_INTERNAL_CALL(TextMeshComponent_GetKerning);
		SP_ADD_INTERNAL_CALL(TextMeshComponent_SetKerning);
		SP_ADD_INTERNAL_CALL(TextMeshComponent_GetMaxWidth);
		SP_ADD_INTERNAL_CALL(TextMeshComponent_SetMaxWidth);

		SP_ADD_INTERNAL_CALL(MeshRendererComponent_GetMeshType);
		SP_ADD_INTERNAL_CALL(MeshRendererComponent_SetMeshType);
		SP_ADD_INTERNAL_CALL(MeshRendererComponent_GetScale);
		SP_ADD_INTERNAL_CALL(MeshRendererComponent_SetScale);

		SP_ADD_INTERNAL_CALL(Material_GetAlbedo);
		SP_ADD_INTERNAL_CALL(Material_SetAlbedo);
		SP_ADD_INTERNAL_CALL(Material_GetMetallic);
		SP_ADD_INTERNAL_CALL(Material_SetMetallic);
		SP_ADD_INTERNAL_CALL(Material_GetRoughness);
		SP_ADD_INTERNAL_CALL(Material_SetRoughness);

		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTexture);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_GetScale);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_SetScale);

		SP_ADD_INTERNAL_CALL(CircleRendererComponent_GetColor);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_SetColor);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_GetThickness);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_SetThickness);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_GetFade);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_SetFade);

		SP_ADD_INTERNAL_CALL(AudioSourceComponent_GetIsPlaying);
		SP_ADD_INTERNAL_CALL(AudioSourceComponent_Play);
		SP_ADD_INTERNAL_CALL(AudioSourceComponent_Stop);

		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetTranslation);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetTranslation);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetRotation);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetRotation);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_Translate);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_Rotate);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_LookAt);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetBodyType);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetBodyType);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetMass);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetMass);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetLinearVelocity);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetLinearVelocity);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetLinearDrag);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetLinearDrag);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetAngularVelocity);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetAngularVelocity);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetAngularDrag);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetAngularDrag);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetDisableGravity);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetDisableGravity);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_GetIsKinematic);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_SetIsKinematic);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_AddForce);
		SP_ADD_INTERNAL_CALL(RigidBodyComponent_AddTorque);

		SP_ADD_INTERNAL_CALL(CharacterControllerComponent_Move);
		SP_ADD_INTERNAL_CALL(CharacterControllerComponent_Jump); 
		SP_ADD_INTERNAL_CALL(CharacterControllerComponent_IsGrounded);

		SP_ADD_INTERNAL_CALL(BoxColliderComponent_GetHalfSize);
		SP_ADD_INTERNAL_CALL(BoxColliderComponent_SetHalfSize);
		SP_ADD_INTERNAL_CALL(BoxColliderComponent_GetOffset);
		SP_ADD_INTERNAL_CALL(BoxColliderComponent_SetOffset);
		SP_ADD_INTERNAL_CALL(BoxColliderComponent_GetIsTrigger);
		SP_ADD_INTERNAL_CALL(BoxColliderComponent_SetIsTrigger);

		SP_ADD_INTERNAL_CALL(SphereColliderComponent_GetRadius);
		SP_ADD_INTERNAL_CALL(SphereColliderComponent_SetRadius);
		SP_ADD_INTERNAL_CALL(SphereColliderComponent_GetOffset);
		SP_ADD_INTERNAL_CALL(SphereColliderComponent_SetOffset);
		SP_ADD_INTERNAL_CALL(SphereColliderComponent_GetIsTrigger);
		SP_ADD_INTERNAL_CALL(SphereColliderComponent_SetIsTrigger);

		SP_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetRadius);
		SP_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetRadius);
		SP_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetHeight);
		SP_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetHeight);
		SP_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetOffset);
		SP_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetOffset);
		SP_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetIsTrigger);
		SP_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetIsTrigger);

		SP_ADD_INTERNAL_CALL(Physics_Raycast);

		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_GetBodyType);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_SetBodyType);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyForce);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyForceToCenter);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulseToCenter);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_GetVelocity);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_SetVelocity);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_GetDrag);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_SetDrag);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_GetFixedRotation);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_SetFixedRotation);

		SP_ADD_INTERNAL_CALL(Physics2D_Raycast);

		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetOffset);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetOffset);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetSize);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetSize);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetDensity);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetDensity);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetFriction);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetFriction);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitution);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitution);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitutionThreshold);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitutionThreshold);

		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetOffset);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetOffset);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRadius);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRadius);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetDensity);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetDensity);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetFriction);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetFriction);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRestitution);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRestitution);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRestitutionThreshold);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRestitutionThreshold);

		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetVelocity);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetVelocity);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetVelocityVariation);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetVelocityVariation);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetOffset);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetOffset);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetSizeBegin);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetSizeBegin);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetSizeEnd);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetSizeEnd);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetSizeVariation);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetSizeVariation);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetColorBegin);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetColorBegin);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetColorEnd);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetColorEnd);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetRotation);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetRotation);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_GetLifeTime);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_SetLifeTime);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_Start);
		SP_ADD_INTERNAL_CALL(ParticleEmitterComponent_Stop);

		SP_ADD_INTERNAL_CALL(RandomDevice_RangedInt32);
		SP_ADD_INTERNAL_CALL(RandomDevice_RangedFloat);

		SP_ADD_INTERNAL_CALL(Mathf_GetPI);
		SP_ADD_INTERNAL_CALL(Mathf_GetPI_D);
		SP_ADD_INTERNAL_CALL(Mathf_Sqrt);
		SP_ADD_INTERNAL_CALL(Mathf_Sin);
		SP_ADD_INTERNAL_CALL(Mathf_Cos);
		SP_ADD_INTERNAL_CALL(Mathf_Acos);
		SP_ADD_INTERNAL_CALL(Mathf_Tan);
		SP_ADD_INTERNAL_CALL(Mathf_Max);
		SP_ADD_INTERNAL_CALL(Mathf_Max);
		SP_ADD_INTERNAL_CALL(Mathf_Deg2Rad);
		SP_ADD_INTERNAL_CALL(Mathf_Rad2Deg);
		SP_ADD_INTERNAL_CALL(Mathf_Deg2RadVector3);
		SP_ADD_INTERNAL_CALL(Mathf_Rad2DegVector3);

		SP_ADD_INTERNAL_CALL(Vector3_CrossProductVec3);
		SP_ADD_INTERNAL_CALL(Vector3_DotProductVec3);

		SP_ADD_INTERNAL_CALL(Time_GetElapsed);
		SP_ADD_INTERNAL_CALL(Time_GetDeltaTime);

		SP_ADD_INTERNAL_CALL(Input_IsKeyDown);
		SP_ADD_INTERNAL_CALL(Input_IsKeyUp);
		SP_ADD_INTERNAL_CALL(Input_IsMouseButtonDown);
		SP_ADD_INTERNAL_CALL(Input_IsMouseButtonUp);
		SP_ADD_INTERNAL_CALL(Input_GetMousePosition);
		SP_ADD_INTERNAL_CALL(Input_GetMouseScrollOffset);
		SP_ADD_INTERNAL_CALL(Input_IsGamepadButtonDown);
		SP_ADD_INTERNAL_CALL(Input_IsGamepadButtonUp);
		SP_ADD_INTERNAL_CALL(Input_GetGamepadAxis);
		SP_ADD_INTERNAL_CALL(Input_ShowMouseCursor);

		SP_ADD_INTERNAL_CALL(Gui_Begin);
		SP_ADD_INTERNAL_CALL(Gui_BeginWithPosition);
		SP_ADD_INTERNAL_CALL(Gui_BeginWithSize);
		SP_ADD_INTERNAL_CALL(Gui_BeginWithPositionAndSize);
		SP_ADD_INTERNAL_CALL(Gui_End);
		SP_ADD_INTERNAL_CALL(Gui_Separator);
		SP_ADD_INTERNAL_CALL(Gui_Spacing);
		SP_ADD_INTERNAL_CALL(Gui_Text);
		SP_ADD_INTERNAL_CALL(Gui_Button);

		SP_ADD_INTERNAL_CALL(Debug_Log);
		SP_ADD_INTERNAL_CALL(Debug_Info);
		SP_ADD_INTERNAL_CALL(Debug_Warn);
		SP_ADD_INTERNAL_CALL(Debug_Error);
		SP_ADD_INTERNAL_CALL(Debug_Critical);
	}

}
