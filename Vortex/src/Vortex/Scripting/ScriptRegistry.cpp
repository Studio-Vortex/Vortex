#include "vxpch.h"
#include "ScriptRegistry.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Core/Input.h"
#include "Vortex/Core/Log.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scripting/ScriptUtils.h"
#include "Vortex/Scripting/ScriptEngine.h"

#include "Vortex/Audio/AudioSource.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/3D/PhysXTypes.h"
#include "Vortex/Physics/3D/PhysXAPIHelpers.h"
#include "Vortex/Physics/2D/Physics2D.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/RenderCommand.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"

#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"

#include "Vortex/UI/UI.h"
#include "Vortex/Utils/PlatformUtils.h"

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

#define VX_ADD_INTERNAL_CALL(icall) mono_add_internal_call("Vortex.InternalCalls::" #icall, InternalCalls::icall)

	struct ScriptingData
	{
		std::unordered_map<MonoType*, std::function<void(Entity)>> EntityAddComponentFuncs;
		std::unordered_map<MonoType*, std::function<bool(Entity)>> EntityHasComponentFuncs;
		std::unordered_map<MonoType*, std::function<void(Entity)>> EntityRemoveComponentFuncs;

		Entity HoveredEntity = Entity{};

		float SceneStartTime = 0.0f;

		std::string ActiveSceneName = "";
		std::string SceneToBeLoaded = "";

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

			Application::Get().Quit();
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
			return Renderer::GetBloomSettings().x;
		}

		void SceneRenderer_SetBloomThreshold(float threshold)
		{
			Renderer::SetBloomThreshold(threshold);
		}

		float SceneRenderer_GetBloomSoftKnee()
		{
			return Renderer::GetBloomSettings().y;
		}

		void SceneRenderer_SetBloomSoftKnee(float softKnee)
		{
			Renderer::SetBloomSoftKnee(softKnee);
		}

		float SceneRenderer_GetBloomUnknown()
		{
			return Renderer::GetBloomSettings().z;
		}

		void SceneRenderer_SetBloomUnknown(float unknown)
		{
			Renderer::SetBloomUnknown(unknown);
		}

		float SceneRenderer_GetExposure()
		{
			return Renderer::GetSceneExposure();
		}

		void SceneRenderer_SetExposure(float exposure)
		{
			Renderer::SetSceneExposure(exposure);
		}

		float SceneRenderer_GetGamma()
		{
			return Renderer::GetSceneGamma();
		}

		void SceneRenderer_SetGamma(float gamma)
		{
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
			Renderer2D::DrawCircle(*translation, *size, 0.0f, *color, thickness, fade);
		}

		void DebugRenderer_DrawCircleVec3(Math::vec3* translation, Math::vec3* size, Math::vec4* color, float thickness, float fade)
		{
			Renderer2D::DrawCircle(*translation, *size, 0.0f, *color, thickness, fade);
		}

		void DebugRenderer_DrawBoundingBox(Math::vec3* worldPosition, Math::vec3* size, Math::vec4* color)
		{
			Math::AABB aabb{
				-Math::vec3(0.5f),
				+Math::vec3(0.5f),
			};

			Math::mat4 transform = Math::Identity() * Math::Translate(*worldPosition) * Math::Scale(*size);
			Renderer2D::DrawAABB(aabb, transform, *color);
		}

		void DebugRenderer_Flush()
		{
			Scene* contextScene = GetContextScene();

			Renderer2D::EndScene();
		}

#pragma endregion

#pragma region Scene

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

		uint64_t Scene_InstantiateAtWorldPosition(UUID entityUUID, Math::vec3* worldPosition)
		{
			return 0;
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
			if (!s_Data.HoveredEntity)
				return 0;

			return s_Data.HoveredEntity.GetUUID();
		}

#pragma endregion

#pragma region SceneManager

		void SceneManager_LoadScene(MonoString* sceneName)
		{
			Scene* contextScene = GetContextScene();

			char* sceneNameCStr = mono_string_to_utf8(sceneName);
			s_Data.SceneToBeLoaded = std::string(sceneNameCStr);
			mono_free(sceneNameCStr);
		}

		MonoString* SceneManager_GetActiveScene()
		{
			Scene* contextScene = GetContextScene();

			return mono_string_new(mono_domain_get(), s_Data.ActiveSceneName.c_str());
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

			contextScene->DestroyEntity(entity, excludeChildren);
		}

		void Entity_SetActive(UUID entityUUID, bool isActive)
		{
			Entity entity = GetEntity(entityUUID);

			entity.SetActive(isActive);
		}

#pragma endregion

#pragma region Transform Component

		void TransformComponent_GetTranslation(UUID entityUUID, Math::vec3* outTranslation)
		{
			Entity entity = GetEntity(entityUUID);

			*outTranslation = entity.GetComponent<TransformComponent>().Translation;
		}

		void TransformComponent_SetTranslation(UUID entityUUID, Math::vec3* translation)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<TransformComponent>().Translation = *translation;
		}

		void TransformComponent_GetRotation(UUID entityUUID, Math::vec3* outRotation)
		{
			Entity entity = GetEntity(entityUUID);

			*outRotation = entity.GetComponent<TransformComponent>().GetRotationEuler();

			// Since we store rotation in radians we must convert to degrees here
			outRotation->x = Math::Rad2Deg(outRotation->x);
			outRotation->y = Math::Rad2Deg(outRotation->y);
			outRotation->z = Math::Rad2Deg(outRotation->z);
		}

		void TransformComponent_SetRotation(UUID entityUUID, Math::vec3* rotation)
		{
			Entity entity = GetEntity(entityUUID);

			// Since we store rotation in radians we must convert to radians here
			rotation->x = Math::Deg2Rad(rotation->x);
			rotation->y = Math::Deg2Rad(rotation->y);
			rotation->z = Math::Deg2Rad(rotation->z);

			entity.GetComponent<TransformComponent>().SetRotationEuler(*rotation);
		}

		void TransformComponent_SetTranslationAndRotation(UUID entityUUID, Math::vec3* translation, Math::vec3* rotation)
		{
			Entity entity = GetEntity(entityUUID);

			TransformComponent& transform = entity.GetTransform();
			transform.Translation = *translation;
			transform.SetRotationEuler(*rotation);
		}

		void TransformComponent_GetRotationQuaternion(UUID entityUUID, Math::quaternion* outOrientation)
		{
			Entity entity = GetEntity(entityUUID);

			*outOrientation = entity.GetTransform().GetRotation();
		}

		void TransformComponent_SetRotationQuaternion(UUID entityUUID, Math::quaternion* orientation)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetTransform().SetRotation(*orientation);
		}

		void TransformComponent_GetScale(UUID entityUUID, Math::vec3* outScale)
		{
			Entity entity = GetEntity(entityUUID);

			*outScale = entity.GetComponent<TransformComponent>().Scale;
		}

		void TransformComponent_SetScale(UUID entityUUID, Math::vec3* scale)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<TransformComponent>().Scale = *scale;
		}

		void TransformComponent_GetWorldSpaceTransform(UUID entityUUID, Math::vec3* outTranslation, Math::vec3* outRotationEuler, Math::vec3* outScale)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			TransformComponent worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);
			*outTranslation = worldSpaceTransform.Translation;
			*outRotationEuler = worldSpaceTransform.GetRotationEuler();
			*outScale = worldSpaceTransform.Scale;
		}

		void TransformComponent_GetForwardDirection(UUID entityUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

			Math::vec3 rotation = worldSpaceTransform.GetRotationEuler();

			*outDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), Math::vec3(0.0f, 0.0f, -1.0f));
		}

		void TransformComponent_GetUpDirection(UUID entityUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

			Math::vec3 rotation = worldSpaceTransform.GetRotationEuler();

			*outDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), Math::vec3(0.0f, 1.0f, 0.0f));
		}

		void TransformComponent_GetRightDirection(UUID entityUUID, Math::vec3* outDirection)
		{
			Scene* contextScene = GetContextScene();
			Entity entity = GetEntity(entityUUID);

			const auto& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

			Math::vec3 rotation = worldSpaceTransform.GetRotationEuler();

			*outDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), Math::vec3(1.0f, 0.0f, 0.0f));
		}

		void TransformComponent_LookAt(UUID entityUUID, Math::vec3* worldPoint)
		{
			Entity entity = GetEntity(entityUUID);

			TransformComponent& transform = entity.GetTransform();
			Math::vec3 upDirection(0.0f, 1.0f, 0.0f);
			Math::mat4 result = Math::LookAt(transform.Translation, *worldPoint, upDirection);
			Math::vec3 translation, rotation, scale;
			Math::DecomposeTransform(Math::Inverse(result), translation, rotation, scale);
			transform.SetRotationEuler(rotation);
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
			Math::quaternion orientation;
			Math::DecomposeTransform(transform, out.Translation, orientation, out.Scale);
			outTransform->SetRotation(orientation);
		}

#pragma endregion

#pragma region Camera Component

		void CameraComponent_GetPrimary(UUID entityUUID, bool* outPrimary)
		{
			Entity entity = GetEntity(entityUUID);

			*outPrimary = entity.GetComponent<CameraComponent>().Primary;
		}

		void CameraComponent_SetPrimary(UUID entityUUID, bool primary)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CameraComponent>().Primary = primary;
		}

		float CameraComponent_GetPerspectiveVerticalFOV(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return Math::Rad2Deg(entity.GetComponent<CameraComponent>().Camera.GetPerspectiveVerticalFOVRad());
		}

		void CameraComponent_SetPerspectiveVerticalFOV(UUID entityUUID, float perspectiveVerticalFOV)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CameraComponent>().Camera.SetPerspectiveVerticalFOVRad(Math::Deg2Rad(perspectiveVerticalFOV));
		}

		void CameraComponent_GetFixedAspectRatio(UUID entityUUID, bool* outFixedAspectRatio)
		{
			Entity entity = GetEntity(entityUUID);

			*outFixedAspectRatio = entity.GetComponent<CameraComponent>().FixedAspectRatio;
		}

		void CameraComponent_SetFixedAspectRatio(UUID entityUUID, bool fixedAspectRatio)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CameraComponent>().FixedAspectRatio = fixedAspectRatio;
		}

#pragma endregion

#pragma region Light Source Component

		LightType LightSourceComponent_GetLightType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return LightType::Directional;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			return lsc.Type;
		}

		void LightSourceComponent_SetLightType(UUID entityUUID, LightType type)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return;
			}

			LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			lsc.Type = type;
		}

		void LightSourceComponent_GetRadiance(UUID entityUUID, Math::vec3* outRadiance)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			*outRadiance = lsc.Source->GetRadiance();
		}

		void LightSourceComponent_SetRadiance(UUID entityUUID, Math::vec3* radiance)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			lsc.Source->SetRadiance(*radiance);
		}

		float LightSourceComponent_GetIntensity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			return lsc.Source->GetIntensity();
		}

		void LightSourceComponent_SetIntensity(UUID entityUUID, float intensity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			lsc.Source->SetIntensity(intensity);
		}

		float LightSourceComponent_GetCutoff(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			return lsc.Source->GetCutOff();
		}

		void LightSourceComponent_SetCutoff(UUID entityUUID, float cutoff)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			lsc.Source->SetCutOff(cutoff);
		}

		float LightSourceComponent_GetOuterCutoff(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			return lsc.Source->GetOuterCutOff();
		}

		void LightSourceComponent_SetOuterCutoff(UUID entityUUID, float outerCutoff)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			lsc.Source->SetOuterCutOff(outerCutoff);
		}

		float LightSourceComponent_GetShadowBias(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return 0.0f;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			return lsc.Source->GetShadowBias();
		}

		void LightSourceComponent_SetShadowBias(UUID entityUUID, float shadowBias)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			lsc.Source->SetShadowBias(shadowBias);
		}

		bool LightSourceComponent_GetCastShadows(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return false;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			return lsc.Source->GetCastShadows();
		}

		void LightSourceComponent_SetCastShadows(UUID entityUUID, bool castShadows)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			lsc.Source->SetCastShadows(castShadows);
		}

		bool LightSourceComponent_GetSoftShadows(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return false;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			return lsc.Source->GetSoftShadows();
		}

		void LightSourceComponent_SetSoftShadows(UUID entityUUID, bool softShadows)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<LightSourceComponent>())
			{
				VX_CONSOLE_LOG_ERROR("Entity doesn't have Light Source!");
				return;
			}

			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();
			lsc.Source->SetSoftShadows(softShadows);
		}

#pragma endregion

#pragma region TextMesh Component

		MonoString* TextMeshComponent_GetTextString(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return mono_string_new(mono_domain_get(), entity.GetComponent<TextMeshComponent>().TextString.c_str());
		}

		void TextMeshComponent_SetTextString(UUID entityUUID, MonoString* textString)
		{
			Entity entity = GetEntity(entityUUID);

			char* textCStr = mono_string_to_utf8(textString);

			entity.GetComponent<TextMeshComponent>().TextString = std::string(textCStr);
			mono_free(textCStr);
		}

		void TextMeshComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
		{
			Entity entity = GetEntity(entityUUID);

			*outColor = entity.GetComponent<TextMeshComponent>().Color;
		}

		void TextMeshComponent_SetColor(UUID entityUUID, Math::vec4* color)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<TextMeshComponent>().Color = *color;
		}

		float TextMeshComponent_GetLineSpacing(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<TextMeshComponent>().LineSpacing;
		}

		void TextMeshComponent_SetLineSpacing(UUID entityUUID, float lineSpacing)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<TextMeshComponent>().LineSpacing = lineSpacing;
		}

		float TextMeshComponent_GetKerning(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<TextMeshComponent>().Kerning;
		}

		void TextMeshComponent_SetKerning(UUID entityUUID, float kerning)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<TextMeshComponent>().Kerning = kerning;
		}

		float TextMeshComponent_GetMaxWidth(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<TextMeshComponent>().MaxWidth;
		}

		void TextMeshComponent_SetMaxWidth(UUID entityUUID, float maxWidth)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<TextMeshComponent>().MaxWidth = maxWidth;
		}

#pragma endregion

#pragma region Animator Component

		bool AnimatorComponent_IsPlaying(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<AnimatorComponent>().Animator->IsPlaying();
		}

		void AnimatorComponent_Play(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			SharedRef<Animation> animation = entity.GetComponent<AnimationComponent>().Animation;

			if (!animation)
			{
				VX_CORE_WARN_TAG("Scripting", "Animation was invalid! exiting early");
				return;
			}

			SharedRef<Animator> animator = entity.GetComponent<AnimatorComponent>().Animator;
			animator->PlayAnimation();
		}

#pragma endregion

#pragma region Mesh Renderer Component



#pragma endregion

#pragma region Static Mesh Renderer Component

		MeshType StaticMeshRendererComponent_GetMeshType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<StaticMeshRendererComponent>().Type;
		}

		void StaticMeshRendererComponent_SetMeshType(UUID entityUUID, MeshType meshType)
		{
			Entity entity = GetEntity(entityUUID);

			if (meshType != MeshType::Custom)
			{
				StaticMeshRendererComponent& meshRenderer = entity.GetComponent<StaticMeshRendererComponent>();
				meshRenderer.Type = meshType;
				meshRenderer.StaticMesh = StaticMesh::Create(StaticMesh::DefaultMeshSourcePaths[static_cast<uint32_t>(meshType)], entity.GetTransform(), MeshImportOptions(), (int)(entt::entity)entity);
			}
		}

#pragma endregion

#pragma region Material

		void Material_GetAlbedo(UUID entityUUID, uint32_t submeshIndex, Math::vec3* outAlbedo)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				*outAlbedo = submeshes[submeshIndex].GetMaterial()->GetAlbedo();
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				*outAlbedo = submeshes[submeshIndex].GetMaterial()->GetAlbedo();
			}
		}

		void Material_SetAlbedo(UUID entityUUID, uint32_t submeshIndex, Math::vec3* albedo)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetAlbedo(*albedo);
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetAlbedo(*albedo);
			}
		}

		float Material_GetMetallic(UUID entityUUID, uint32_t submeshIndex)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				return submeshes[submeshIndex].GetMaterial()->GetMetallic();
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				return submeshes[submeshIndex].GetMaterial()->GetMetallic();
			}

			return 0.0f;
		}

		void Material_SetMetallic(UUID entityUUID, uint32_t submeshIndex, float metallic)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetMetallic(metallic);
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetMetallic(metallic);
			}
		}

		float Material_GetRoughness(UUID entityUUID, uint32_t submeshIndex)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				return submeshes[submeshIndex].GetMaterial()->GetRoughness();
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				return submeshes[submeshIndex].GetMaterial()->GetRoughness();
			}

			return 0.0f;
		}

		void Material_SetRoughness(UUID entityUUID, uint32_t submeshIndex, float roughness)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetRoughness(roughness);
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetRoughness(roughness);
			}
		}

		float Material_GetEmission(UUID entityUUID, uint32_t submeshIndex)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				return submeshes[submeshIndex].GetMaterial()->GetEmission();
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				return submeshes[submeshIndex].GetMaterial()->GetEmission();
			}

			return 0.0f;
		}

		void Material_SetEmission(UUID entityUUID, uint32_t submeshIndex, float emission)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetEmission(emission);
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetEmission(emission);
			}
		}

		void Material_GetUV(UUID entityUUID, uint32_t submeshIndex, Math::vec2* outUV)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				*outUV = submeshes[submeshIndex].GetMaterial()->GetUV();
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				*outUV = submeshes[submeshIndex].GetMaterial()->GetUV();
			}
		}

		void Material_SetUV(UUID entityUUID, uint32_t submeshIndex, Math::vec2* uv)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetUV(*uv);
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetUV(*uv);
			}
		}

		float Material_GetOpacity(UUID entityUUID, uint32_t submeshIndex)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				return submeshes[submeshIndex].GetMaterial()->GetOpacity();
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				return submeshes[submeshIndex].GetMaterial()->GetOpacity();
			}

			return 0.0f;
		}

		void Material_SetOpacity(UUID entityUUID, uint32_t submeshIndex, float opacity)
		{
			Entity entity = GetEntity(entityUUID);

			if (entity.HasComponent<MeshRendererComponent>())
			{
				SharedRef<Mesh> mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
				const auto& submeshes = mesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetOpacity(opacity);
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				SharedRef<StaticMesh> staticMesh = entity.GetComponent<StaticMeshRendererComponent>().StaticMesh;
				const auto& submeshes = staticMesh->GetSubmeshes();
				VX_CORE_ASSERT(submeshIndex < submeshes.size(), "Index out of bounds!");
				submeshes[submeshIndex].GetMaterial()->SetOpacity(opacity);
			}
		}

#pragma endregion

#pragma region Sprite Renderer Component

		void SpriteRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
		{
			Entity entity = GetEntity(entityUUID);

			*outColor = entity.GetComponent<SpriteRendererComponent>().SpriteColor;
		}

		void SpriteRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<SpriteRendererComponent>().SpriteColor = *color;
		}

		void SpriteRendererComponent_GetScale(UUID entityUUID, Math::vec2* outScale)
		{
			Entity entity = GetEntity(entityUUID);

			*outScale = entity.GetComponent<SpriteRendererComponent>().Scale;
		}

		MonoString* SpriteRendererComponent_GetTexture(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			const std::string& texturePath = entity.GetComponent<SpriteRendererComponent>().Texture->GetPath();

			return mono_string_new(mono_domain_get(), texturePath.c_str());
		}

		void SpriteRendererComponent_SetTexture(UUID entityUUID, MonoString* texturePathString)
		{
			char* texturePathCStr = mono_string_to_utf8(texturePathString);

			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<SpriteRendererComponent>().Texture = Texture2D::Create(std::string(texturePathCStr));

			mono_free(texturePathCStr);
		}

		void SpriteRendererComponent_SetScale(UUID entityUUID, Math::vec2* scale)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<SpriteRendererComponent>().Scale = *scale;
		}

#pragma endregion

#pragma region Circle Renderer Component

		void CircleRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
		{
			Entity entity = GetEntity(entityUUID);

			*outColor = entity.GetComponent<CircleRendererComponent>().Color;
		}

		void CircleRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CircleRendererComponent>().Color = *color;
		}

		void CircleRendererComponent_GetThickness(UUID entityUUID, float* outThickness)
		{
			Entity entity = GetEntity(entityUUID);

			*outThickness = entity.GetComponent<CircleRendererComponent>().Thickness;
		}

		void CircleRendererComponent_SetThickness(UUID entityUUID, float thickness)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CircleRendererComponent>().Thickness = thickness;
		}

		void CircleRendererComponent_GetFade(UUID entityUUID, float* outFade)
		{
			Entity entity = GetEntity(entityUUID);

			*outFade = entity.GetComponent<CircleRendererComponent>().Fade;
		}

		void CircleRendererComponent_SetFade(UUID entityUUID, float fade)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CircleRendererComponent>().Fade = fade;
		}

#pragma endregion

#pragma region Particle Emitter Component

		void ParticleEmitterComponent_GetVelocity(UUID entityUUID, Math::vec3* outVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			*outVelocity = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Velocity;
		}

		void ParticleEmitterComponent_SetVelocity(UUID entityUUID, Math::vec3* velocity)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Velocity = *velocity;
		}

		void ParticleEmitterComponent_GetVelocityVariation(UUID entityUUID, Math::vec3* outVelocityVariation)
		{
			Entity entity = GetEntity(entityUUID);

			*outVelocityVariation = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().VelocityVariation;
		}

		void ParticleEmitterComponent_SetVelocityVariation(UUID entityUUID, Math::vec3* velocityVariation)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().VelocityVariation = *velocityVariation;
		}

		void ParticleEmitterComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			*outOffset = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Offset;
		}

		void ParticleEmitterComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Offset = *offset;
		}

		void ParticleEmitterComponent_GetSizeBegin(UUID entityUUID, Math::vec2* outSizeBegin)
		{
			Entity entity = GetEntity(entityUUID);

			*outSizeBegin = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeBegin;
		}

		void ParticleEmitterComponent_SetSizeBegin(UUID entityUUID, Math::vec2* sizeBegin)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeBegin = *sizeBegin;
		}

		void ParticleEmitterComponent_GetSizeEnd(UUID entityUUID, Math::vec2* outSizeEnd)
		{
			Entity entity = GetEntity(entityUUID);

			*outSizeEnd = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeEnd;
		}

		void ParticleEmitterComponent_SetSizeEnd(UUID entityUUID, Math::vec2* sizeEnd)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeEnd = *sizeEnd;
		}

		void ParticleEmitterComponent_GetSizeVariation(UUID entityUUID, Math::vec2* outSizeVariation)
		{
			Entity entity = GetEntity(entityUUID);

			*outSizeVariation = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeVariation;
		}

		void ParticleEmitterComponent_SetSizeVariation(UUID entityUUID, Math::vec2* sizeVariation)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().SizeVariation = *sizeVariation;
		}

		void ParticleEmitterComponent_GetColorBegin(UUID entityUUID, Math::vec4* outColorBegin)
		{
			Entity entity = GetEntity(entityUUID);

			*outColorBegin = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorBegin;
		}

		void ParticleEmitterComponent_SetColorBegin(UUID entityUUID, Math::vec4* colorBegin)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorBegin = *colorBegin;
		}

		void ParticleEmitterComponent_GetColorEnd(UUID entityUUID, Math::vec4* outColorEnd)
		{
			Entity entity = GetEntity(entityUUID);

			*outColorEnd = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorEnd;
		}

		void ParticleEmitterComponent_SetColorEnd(UUID entityUUID, Math::vec4* colorEnd)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().ColorEnd = *colorEnd;
		}

		void ParticleEmitterComponent_GetRotation(UUID entityUUID, float* outRotation)
		{
			Entity entity = GetEntity(entityUUID);

			*outRotation = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Rotation;
		}

		void ParticleEmitterComponent_SetRotation(UUID entityUUID, float colorEnd)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().Rotation = colorEnd;
		}

		void ParticleEmitterComponent_GetLifeTime(UUID entityUUID, float* outLifeTime)
		{
			Entity entity = GetEntity(entityUUID);

			*outLifeTime = entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().LifeTime;
		}

		void ParticleEmitterComponent_SetLifeTime(UUID entityUUID, float lifetime)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->GetProperties().LifeTime = lifetime;
		}

		void ParticleEmitterComponent_Start(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->Start();
		}

		void ParticleEmitterComponent_Stop(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<ParticleEmitterComponent>().Emitter->Stop();
		}

#pragma endregion

#pragma region AudioSource Component

		bool AudioSourceComponent_GetIsPlaying(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<AudioSourceComponent>().Source->IsPlaying();
		}

		void AudioSourceComponent_Play(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<AudioSourceComponent>().Source->Play();
		}

		void AudioSourceComponent_Stop(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<AudioSourceComponent>().Source->Stop();
		}

#pragma endregion

#pragma region RigidBody Component

		void RigidBodyComponent_GetTranslation(UUID entityUUID, Math::vec3* outTranslation)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Cannot get translation of Static actor");
				return;
			}

			physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)Physics::GetActor(entityUUID);
			Math::vec3 translation = FromPhysXVector(actor->getGlobalPose().p);

			*outTranslation = translation;
		}

		void RigidBodyComponent_SetTranslation(UUID entityUUID, Math::vec3* translation)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Cannot set translation of Static actor");
				return;
			}

			physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)Physics::GetActor(entityUUID);

			const auto& transformComponent = entity.GetTransform();
			Math::vec3 rotation = transformComponent.GetRotationEuler();
			auto entityTransform = TransformComponent{ *translation, rotation, transformComponent.Scale }.GetTransform();
			auto physxTransform = ToPhysXTransform(entityTransform);

			actor->setGlobalPose(physxTransform);
		}

		void RigidBodyComponent_GetRotation(UUID entityUUID, Math::vec3* outRotation)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Cannot get rotation of Static actor");
				return;
			}

			physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)Physics::GetActor(entityUUID);
			Math::quaternion orientation = FromPhysXQuat(actor->getGlobalPose().q);

			*outRotation = Math::EulerAngles(orientation);
		}

		void RigidBodyComponent_SetRotation(UUID entityUUID, Math::vec3* rotation)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Cannot set rotation of Static actor");
				return;
			}

			physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)Physics::GetActor(entityUUID);
			physx::PxTransform physxTransform = actor->getGlobalPose();
			physxTransform.q = ToPhysXQuat(*rotation);

			actor->setGlobalPose(physxTransform);
		}

		void RigidBodyComponent_Translate(UUID entityUUID, Math::vec3* translation)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Cannot translate Static actor");
				return;
			}

			physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)Physics::GetActor(entityUUID);
			physx::PxTransform physxTransform = actor->getGlobalPose();
			physxTransform.p += ToPhysXVector(*translation);

			actor->setGlobalPose(physxTransform);
		}

		void RigidBodyComponent_Rotate(UUID entityUUID, Math::vec3* rotation)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Cannot rotate Static actor");
				return;
			}

			physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)Physics::GetActor(entityUUID);
			physx::PxTransform physxTransform = actor->getGlobalPose();
			physxTransform.q *= ToPhysXQuat(*rotation);

			actor->setGlobalPose(physxTransform);
		}

		void RigidBodyComponent_LookAt(UUID entityUUID, Math::vec3* worldPoint)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Calling RigidBody.LookAt with Static actor");
				return;
			}

			physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)Physics::GetActor(entityUUID);
			physx::PxTransform physxTransform = actor->getGlobalPose();

			Math::mat4 transform = FromPhysXTransform(physxTransform);

			Math::vec3 upDirection(0.0f, 1.0f, 0.0f);
			Math::mat4 result = Math::LookAt(FromPhysXVector(physxTransform.p), *worldPoint, upDirection);
			Math::vec3 translation, rotation, scale;
			Math::DecomposeTransform(Math::Inverse(result), translation, rotation, scale);
			physxTransform.q = ToPhysXQuat(Math::quaternion(rotation));

			actor->setGlobalPose(physxTransform);
		}

		RigidBodyType RigidBodyComponent_GetBodyType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBodyComponent>().Type;
		}

		void RigidBodyComponent_SetBodyType(UUID entityUUID, RigidBodyType bodyType)
		{
			Entity entity = GetEntity(entityUUID);

			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			const bool consistentBodyType = bodyType == rigidbody.Type;

			if (consistentBodyType)
				return;

			rigidbody.Type = bodyType;
			Physics::ReCreateActor(entity);
		}

		CollisionDetectionType RigidBodyComponent_GetCollisionDetectionType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBodyComponent>().CollisionDetection;
		}

		void RigidBodyComponent_SetCollisionDetectionType(UUID entityUUID, CollisionDetectionType collisionDetectionType)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBodyComponent>().CollisionDetection = collisionDetectionType;
		}

		float RigidBodyComponent_GetMass(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBodyComponent>().Mass;
		}

		void RigidBodyComponent_SetMass(UUID entityUUID, float mass)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBodyComponent>().Mass = mass;
		}

		void RigidBodyComponent_GetLinearVelocity(UUID entityUUID, Math::vec3* outVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			*outVelocity = entity.GetComponent<RigidBodyComponent>().LinearVelocity;
		}

		void RigidBodyComponent_SetLinearVelocity(UUID entityUUID, Math::vec3* velocity)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBodyComponent>().LinearVelocity = *velocity;
		}

		float RigidBodyComponent_GetLinearDrag(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBodyComponent>().LinearDrag;
		}

		void RigidBodyComponent_SetLinearDrag(UUID entityUUID, float drag)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBodyComponent>().LinearDrag = drag;
		}

		void RigidBodyComponent_GetAngularVelocity(UUID entityUUID, Math::vec3* outVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			*outVelocity = entity.GetComponent<RigidBodyComponent>().AngularVelocity;
		}

		void RigidBodyComponent_SetAngularVelocity(UUID entityUUID, Math::vec3* velocity)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBodyComponent>().AngularVelocity = *velocity;
		}

		float RigidBodyComponent_GetAngularDrag(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBodyComponent>().AngularDrag;
		}

		void RigidBodyComponent_SetAngularDrag(UUID entityUUID, float drag)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBodyComponent>().AngularDrag = drag;
		}

		bool RigidBodyComponent_GetDisableGravity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBodyComponent>().DisableGravity;
		}

		void RigidBodyComponent_SetDisableGravity(UUID entityUUID, bool disabled)
		{
			Entity entity = GetEntity(entityUUID);

			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
			rigidbody.DisableGravity = disabled;

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();

			actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, rigidbody.DisableGravity);
		}

		static bool RigidBodyComponent_GetIsKinematic(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBodyComponent>().IsKinematic;
		}

		static void RigidBodyComponent_SetIsKinematic(UUID entityUUID, bool isKinematic)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBodyComponent>().IsKinematic = isKinematic;
		}

		static uint32_t RigidBodyComponent_GetLockFlags(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Static actors cannot be woken up");
				return 0;
			}

			return (uint32_t)rb.LockFlags;
		}

		static void RigidBodyComponent_SetLockFlag(UUID entityUUID, ActorLockFlag flag, bool value, bool forceWake)
		{
			Entity entity = GetEntity(entityUUID);

			RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Cannot set actor lock flag on a Static actor");
				return;
			}

			if (value)
			{
				rb.LockFlags |= (uint8_t)flag;
			}
			else
			{
				rb.LockFlags &= ~(uint8_t)flag;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();

			actor->setRigidDynamicLockFlag((physx::PxRigidDynamicLockFlag::Enum)flag, value);

			if (forceWake)
			{
				actor->wakeUp();
			}
		}

		static bool RigidBodyComponent_IsLockFlagSet(UUID entityUUID, ActorLockFlag flag)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Cannot access actor lock flags of a Static actor");
				return false;
			}

			uint8_t lockFlags = rb.LockFlags;

			return lockFlags & (uint8_t)flag;
		}

		static bool RigidBodyComponent_IsSleeping(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();
			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Static actors are always sleeping");
				return false;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			return actor->isSleeping();
		}

		static void RigidBodyComponent_WakeUp(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rb = entity.GetComponent<RigidBodyComponent>();

			if (rb.Type != RigidBodyType::Dynamic)
			{
				VX_CONSOLE_LOG_WARN("Static actors cannot be woken up");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			actor->wakeUp();
		}

		static void RigidBodyComponent_AddForce(UUID entityUUID, Math::vec3* force, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling Rigidbody.AddForce with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			actor->addForce(ToPhysXVector(*force), (physx::PxForceMode::Enum)mode);
		}

		static void RigidBodyComponent_AddForceAtPosition(UUID entityUUID, Math::vec3* force, Math::vec3* position, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling Rigidbody.AddForceAtPosition with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			physx::PxRigidBodyExt::addForceAtPos(*actor, ToPhysXVector(*force), ToPhysXVector(*position), static_cast<physx::PxForceMode::Enum>(mode));
		}

		static void RigidBodyComponent_AddTorque(UUID entityUUID, Math::vec3* torque, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CONSOLE_LOG_WARN("Calling Rigidbody.AddTorque with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			actor->addTorque(ToPhysXVector(*torque), (physx::PxForceMode::Enum)mode);
		}

		static void RigidBodyComponent_ClearTorque(UUID entityUUID, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type != RigidBodyType::Dynamic || rigidbody.IsKinematic)
			{
				VX_CORE_ASSERT(false, "Calling RigidBody.ClearTorque with a non-dynamic Rigidbody!");
				return;
			}

			physx::PxRigidDynamic* actor = Physics::GetActor(entityUUID)->is<physx::PxRigidDynamic>();
			actor->clearTorque((physx::PxForceMode::Enum)mode);
		}

		static void RigidBodyComponent_ClearForce(UUID entityUUID, ForceMode mode)
		{
			Entity entity = GetEntity(entityUUID);

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

#pragma region Character Controller Component

		void CharacterControllerComponent_Move(UUID entityUUID, Math::vec3* displacement)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Calling CharacterController.Move without a Character Controller!");
				return;
			}

			CharacterControllerComponent& characterControllerComponent = entity.GetComponent<CharacterControllerComponent>();

			physx::PxControllerFilters filters; // TODO
			physx::PxController* controller = Physics::GetController(entityUUID);

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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return false;
			}

			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			physx::PxController* controller = Physics::GetController(entityUUID);

			physx::PxControllerState state;
			controller->getState(state);

			// test if grounded
			if (state.collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
			{
				return true;
			}

			return false;
		}

		float CharacterControllerComponent_GetSpeedDown(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return 0.0f;
			}

			return entity.GetComponent<CharacterControllerComponent>().SpeedDown;
		}

		float CharacterControllerComponent_GetSlopeLimit(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return 0.0f;
			}

			return entity.GetComponent<CharacterControllerComponent>().SlopeLimitDegrees;
		}

		void CharacterControllerComponent_SetSlopeLimit(UUID entityUUID, float slopeLimit)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return;
			}

			entity.GetComponent<CharacterControllerComponent>().SlopeLimitDegrees = slopeLimit;
			Physics::GetController(entityUUID)->setSlopeLimit(Math::Max(0.0f, cosf(Math::Deg2Rad(slopeLimit))));
		}

		float CharacterControllerComponent_GetStepOffset(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return 0.0f;
			}

			return entity.GetComponent<CharacterControllerComponent>().StepOffset;
		}

		void CharacterControllerComponent_SetStepOffset(UUID entityUUID, float stepOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return;
			}

			entity.GetComponent<CharacterControllerComponent>().StepOffset = stepOffset;
			Physics::GetController(entityUUID)->setStepOffset(stepOffset);
		}

		float CharacterControllerComponent_GetContactOffset(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return 0.0f;
			}

			return entity.GetComponent<CharacterControllerComponent>().ContactOffset;
		}

		void CharacterControllerComponent_SetContactOffset(UUID entityUUID, float contactOffset)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return;
			}

			entity.GetComponent<CharacterControllerComponent>().ContactOffset = contactOffset;
			Physics::GetController(entityUUID)->setContactOffset(contactOffset);
		}

		NonWalkableMode CharacterControllerComponent_GetNonWalkableMode(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return NonWalkableMode::PreventClimbing;
			}

			return entity.GetComponent<CharacterControllerComponent>().NonWalkMode;
		}

		void CharacterControllerComponent_SetNonWalkableMode(UUID entityUUID, NonWalkableMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return;
			}

			entity.GetComponent<CharacterControllerComponent>().NonWalkMode = mode;
			Physics::GetController(entityUUID)->setNonWalkableMode((physx::PxControllerNonWalkableMode::Enum)mode);
		}

		CapsuleClimbMode CharacterControllerComponent_GetClimbMode(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return CapsuleClimbMode::Easy;
			}

			return entity.GetComponent<CharacterControllerComponent>().ClimbMode;
		}

		void CharacterControllerComponent_SetClimbMode(UUID entityUUID, CapsuleClimbMode mode)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return;
			}

			entity.GetComponent<CharacterControllerComponent>().ClimbMode = mode;
			// TODO any way to set capsule climbing mode during runtime?
		}

		bool CharacterControllerComponent_GetDisableGravity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return false;
			}

			return entity.GetComponent<CharacterControllerComponent>().DisableGravity;
		}

		void CharacterControllerComponent_SetDisableGravity(UUID entityUUID, bool disableGravity)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<CharacterControllerComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Character Controller!");
				return;
			}

			entity.GetComponent<CharacterControllerComponent>().DisableGravity = disableGravity;
		}

#pragma endregion

#pragma region FixedJoint Component

		uint64_t FixedJointComponent_GetConnectedEntity(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
				return false;
			}

			return Physics::IsConstraintBroken(entityUUID);
		}

		bool FixedJointComponent_GetIsBreakable(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			if (!entity.HasComponent<FixedJointComponent>())
			{
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
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
				VX_CONSOLE_LOG_WARN("Entity doesn't have Fixed Joint!");
				return;
			}

			Physics::BreakJoint(entityUUID);
		}

#pragma endregion

#pragma region BoxCollider Component

		void BoxColliderComponent_GetHalfSize(UUID entityUUID, Math::vec3* outHalfSize)
		{
			Entity entity = GetEntity(entityUUID);

			*outHalfSize = entity.GetComponent<BoxColliderComponent>().HalfSize;
		}

		void BoxColliderComponent_SetHalfSize(UUID entityUUID, Math::vec3* halfSize)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<BoxColliderComponent>().HalfSize = *halfSize;
		}

		void BoxColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			*outOffset = entity.GetComponent<BoxColliderComponent>().Offset;
		}

		void BoxColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<BoxColliderComponent>().Offset = *offset;
		}

		bool BoxColliderComponent_GetIsTrigger(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<BoxColliderComponent>().IsTrigger;
		}

		void BoxColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<BoxColliderComponent>().IsTrigger = isTrigger;
		}

#pragma endregion

#pragma region SphereCollider Component

		float SphereColliderComponent_GetRadius(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<SphereColliderComponent>().Radius;
		}

		void SphereColliderComponent_SetRadius(UUID entityUUID, float radius)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<SphereColliderComponent>().Radius = radius;
		}

		void SphereColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			*outOffset = entity.GetComponent<SphereColliderComponent>().Offset;
		}

		void SphereColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<SphereColliderComponent>().Offset = *offset;
		}

		bool SphereColliderComponent_GetIsTrigger(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<SphereColliderComponent>().IsTrigger;
		}

		void SphereColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<SphereColliderComponent>().IsTrigger = isTrigger;
		}

#pragma endregion

#pragma region CapsuleCollider Component

		float CapsuleColliderComponent_GetRadius(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<CapsuleColliderComponent>().Radius;
		}

		void CapsuleColliderComponent_SetRadius(UUID entityUUID, float radius)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CapsuleColliderComponent>().Radius = radius;
		}

		float CapsuleColliderComponent_GetHeight(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<CapsuleColliderComponent>().Height;
		}

		void CapsuleColliderComponent_SetHeight(UUID entityUUID, float height)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CapsuleColliderComponent>().Height = height;
		}

		void CapsuleColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			*outOffset = entity.GetComponent<CapsuleColliderComponent>().Offset;
		}

		void CapsuleColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CapsuleColliderComponent>().Offset = *offset;
		}

		bool CapsuleColliderComponent_GetIsTrigger(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<CapsuleColliderComponent>().IsTrigger;
		}

		void CapsuleColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CapsuleColliderComponent>().IsTrigger = isTrigger;
		}

#pragma endregion

#pragma region RigidBody2D Component

		void RigidBody2DComponent_GetTranslation(UUID entityUUID, Math::vec2* outTranslation)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			const auto& position = body->GetPosition();
			*outTranslation = Math::vec2{ position.x, position.y };
		}

		void RigidBody2DComponent_SetTranslation(UUID entityUUID, Math::vec2* translation)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetTransform({ translation->x, translation->y }, body->GetAngle());
		}

		float RigidBody2DComponent_GetAngle(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			return Math::Rad2Deg(body->GetAngle());
		}

		void RigidBody2DComponent_SetAngle(UUID entityUUID, float angle)
		{
			Entity entity = GetEntity(entityUUID);

			const RigidBody2DComponent& rigidbody = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rigidbody.RuntimeBody;

			body->SetTransform(body->GetPosition(), Math::Deg2Rad(angle));
		}

		RigidBody2DType RigidBody2DComponent_GetBodyType(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBody2DComponent>().Type;
		}

		void RigidBody2DComponent_SetBodyType(UUID entityUUID, RigidBody2DType bodyType)
		{
			Entity entity = GetEntity(entityUUID);

			RigidBody2DComponent& rb2d = entity.GetComponent<RigidBody2DComponent>();

			if (bodyType != rb2d.Type)
			{
				Physics2D::DestroyPhysicsBody(entity);
				rb2d.Type = bodyType;
				rb2d.RuntimeBody = nullptr;
				Physics2D::CreatePhysicsBody(entity, entity.GetTransform(), rb2d);
			}
		}

		void RigidBody2DComponent_ApplyForce(UUID entityUUID, Math::vec2* force, Math::vec2* point, bool wake)
		{
			Entity entity = GetEntity(entityUUID);

			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->ApplyForce(b2Vec2(force->x, force->y), b2Vec2(point->x, point->y), wake);
		}

		void RigidBody2DComponent_ApplyForceToCenter(UUID entityUUID, Math::vec2* force, bool wake)
		{
			Entity entity = GetEntity(entityUUID);

			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
		}

		void RigidBody2DComponent_ApplyLinearImpulse(UUID entityUUID, Math::vec2* impulse, Math::vec2* point, bool wake)
		{
			Entity entity = GetEntity(entityUUID);

			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
		}

		void RigidBody2DComponent_ApplyLinearImpulseToCenter(UUID entityUUID, Math::vec2* impulse, bool wake)
		{
			Entity entity = GetEntity(entityUUID);

			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
		}

		void RigidBody2DComponent_GetVelocity(UUID entityUUID, Math::vec2* outVelocity)
		{
			Entity entity = GetEntity(entityUUID);

			*outVelocity = entity.GetComponent<RigidBody2DComponent>().Velocity;
		}

		void RigidBody2DComponent_SetVelocity(UUID entityUUID, Math::vec2* velocity)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBody2DComponent>().Velocity = *velocity;
		}

		float RigidBody2DComponent_GetDrag(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBody2DComponent>().Drag;
		}

		void RigidBody2DComponent_SetDrag(UUID entityUUID, float drag)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBody2DComponent>().Drag = drag;
		}

		bool RigidBody2DComponent_GetFixedRotation(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			return entity.GetComponent<RigidBody2DComponent>().FixedRotation;
		}

		void RigidBody2DComponent_SetFixedRotation(UUID entityUUID, bool freeze)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<RigidBody2DComponent>().FixedRotation = freeze;
		}

		float RigidBody2DComponent_GetGravityScale(UUID entityUUID)
		{
			Entity entity = GetEntity(entityUUID);

			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;

			return body->GetGravityScale();
		}

		void RigidBody2DComponent_SetGravityScale(UUID entityUUID, float gravityScale)
		{
			Entity entity = GetEntity(entityUUID);

			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;

			entity.GetComponent<RigidBody2DComponent>().GravityScale = gravityScale;
			body->SetGravityScale(gravityScale);
		}

#pragma endregion

#pragma region Physics2D

		uint64_t Physics2D_Raycast(Math::vec2* start, Math::vec2* end, RaycastHit2D* outResult, bool drawDebugLine)
		{
			

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

			*outOffset = entity.GetComponent<BoxCollider2DComponent>().Offset;
		}

		void BoxCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<BoxCollider2DComponent>().Offset = *offset;
		}

		void BoxCollider2DComponent_GetSize(UUID entityUUID, Math::vec2* outSize)
		{
			Entity entity = GetEntity(entityUUID);

			*outSize = entity.GetComponent<BoxCollider2DComponent>().Size;
		}

		void BoxCollider2DComponent_SetSize(UUID entityUUID, Math::vec2* size)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<BoxCollider2DComponent>().Size = *size;
		}

		void BoxCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity)
		{
			Entity entity = GetEntity(entityUUID);

			*outDensity = entity.GetComponent<BoxCollider2DComponent>().Density;
		}

		void BoxCollider2DComponent_SetDensity(UUID entityUUID, float density)
		{
			Entity entity = GetEntity(entityUUID);

			b2Fixture* fixture = ((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture);
			fixture->SetDensity(density);

			// Since we changed the density we must recalculate the mass data according to box2d
			fixture->GetBody()->ResetMassData();
		}

		void BoxCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction)
		{
			Entity entity = GetEntity(entityUUID);

			*outFriction = entity.GetComponent<BoxCollider2DComponent>().Friction;
		}

		void BoxCollider2DComponent_SetFriction(UUID entityUUID, float friction)
		{
			Entity entity = GetEntity(entityUUID);

			b2Fixture* fixture = ((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture);
			fixture->SetFriction(friction);
		}

		void BoxCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution)
		{
			Entity entity = GetEntity(entityUUID);

			*outRestitution = entity.GetComponent<BoxCollider2DComponent>().Restitution;
		}

		void BoxCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
		{
			Entity entity = GetEntity(entityUUID);

			b2Fixture* fixture = ((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture);
			fixture->SetRestitution(restitution);
		}

		void BoxCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold)
		{
			Entity entity = GetEntity(entityUUID);

			*outRestitutionThreshold = entity.GetComponent<BoxCollider2DComponent>().RestitutionThreshold;
		}

		void BoxCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
		{
			Entity entity = GetEntity(entityUUID);

			b2Fixture* fixture = ((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture);
			fixture->SetRestitutionThreshold(restitutionThreshold);
		}

#pragma endregion

#pragma region Circle Collider2D Component

		void CircleCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset)
		{
			Entity entity = GetEntity(entityUUID);

			*outOffset = entity.GetComponent<CircleCollider2DComponent>().Offset;
		}

		void CircleCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CircleCollider2DComponent>().Offset = *offset;
		}

		void CircleCollider2DComponent_GetRadius(UUID entityUUID, float* outRadius)
		{
			Entity entity = GetEntity(entityUUID);

			*outRadius = entity.GetComponent<CircleCollider2DComponent>().Radius;
		}

		void CircleCollider2DComponent_SetRadius(UUID entityUUID, float radius)
		{
			Entity entity = GetEntity(entityUUID);

			entity.GetComponent<CircleCollider2DComponent>().Radius = radius;
		}

		void CircleCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity)
		{
			Entity entity = GetEntity(entityUUID);

			*outDensity = entity.GetComponent<CircleCollider2DComponent>().Density;
		}

		void CircleCollider2DComponent_SetDensity(UUID entityUUID, float density)
		{
			Entity entity = GetEntity(entityUUID);

			b2Fixture* fixture = (b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture;
			fixture->SetDensity(density);

			// Since we changed the density we must recalculate the mass data according to box2d
			fixture->GetBody()->ResetMassData();
		}

		void CircleCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction)
		{
			Entity entity = GetEntity(entityUUID);

			*outFriction = entity.GetComponent<CircleCollider2DComponent>().Friction;
		}

		void CircleCollider2DComponent_SetFriction(UUID entityUUID, float friction)
		{
			Entity entity = GetEntity(entityUUID);

			((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetFriction(friction);
		}

		void CircleCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution)
		{
			Entity entity = GetEntity(entityUUID);

			*outRestitution = entity.GetComponent<CircleCollider2DComponent>().Restitution;
		}

		void CircleCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
		{
			Entity entity = GetEntity(entityUUID);

			((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetRestitution(restitution);
		}

		void CircleCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold)
		{
			Entity entity = GetEntity(entityUUID);

			*outRestitutionThreshold = entity.GetComponent<CircleCollider2DComponent>().RestitutionThreshold;
		}

		void CircleCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
		{
			Entity entity = GetEntity(entityUUID);

			((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetRestitution(restitutionThreshold);
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

#pragma region Mathf

		float Mathf_GetPI()
		{
			return Math::PI;
		}

		double Mathf_GetPI_D()
		{
			return Math::PI_D;
		}

		float Mathf_Abs(float in)
		{
			return Math::Abs(in);
		}

		float Mathf_Sqrt(float in)
		{
			return Math::Sqrt(in);
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
			Math::vec3 translation, rotation, scale;
			Math::DecomposeTransform(transform, translation, rotation, scale);
			*outRotation = rotation;
		}

#pragma endregion

#pragma region Quaternion



#pragma endregion

#pragma region Vector3

		void Vector3_CrossProductVec3(Math::vec3* left, Math::vec3* right, Math::vec3* outResult)
		{
			*outResult = Math::Cross(*left, *right);
		}

		float Vector3_DotProductVec3(Math::vec3* left, Math::vec3* right)
		{
			return Math::Dot(*left, *right);
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

		void Input_GetMouseScrollOffset(Math::vec2* outMouseScrollOffset)
		{
			*outMouseScrollOffset = Input::GetMouseScrollOffset();
		}

		bool Input_IsGamepadButtonDown(Gamepad button)
		{
			return Input::IsGamepadButtonDown(button);
		}

		bool Input_IsGamepadButtonUp(Gamepad button)
		{
			return Input::IsGamepadButtonUp(button);
		}

		float Input_GetGamepadAxis(Gamepad axis)
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

		namespace Gui = ImGui;

		uint32_t defaultWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking;

		void BeginWindow(char* text, uint32_t flags)
		{
			ImGuiIO& io = Gui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];
			auto largeFont = io.Fonts->Fonts[1];

			Gui::Begin(text, nullptr, defaultWindowFlags | flags);
			Gui::PushFont(largeFont);
			Gui::TextCentered(text);
			Gui::PopFont();
			UI::Draw::Underline();
			Gui::Spacing();
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
			Gui::End();
		}

		void Gui_Separator()
		{
			UI::Draw::Underline();
		}

		void Gui_Spacing()
		{
			Gui::Spacing();
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

#pragma endregion

#pragma region Log

		void Log_Print(MonoString* message)
		{
			char* managedString = mono_string_to_utf8(message);
			VX_CONSOLE_LOG_TRACE("{}", managedString);
			mono_free(managedString);
		}

		void Log_Info(MonoString* message)
		{
			char* managedString = mono_string_to_utf8(message);
			VX_CONSOLE_LOG_INFO("{}", managedString);
			mono_free(managedString);
		}

		void Log_Warn(MonoString* message)
		{
			char* managedString = mono_string_to_utf8(message);
			VX_CONSOLE_LOG_WARN("{}", managedString);
			mono_free(managedString);
		}

		void Log_Error(MonoString* message)
		{
			char* managedString = mono_string_to_utf8(message);
			VX_CONSOLE_LOG_ERROR("{}", managedString);
			mono_free(managedString);
		}

		void Log_Fatal(MonoString* message)
		{
			char* managedString = mono_string_to_utf8(message);
			VX_CONSOLE_LOG_FATAL("{}", managedString);
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

	void ScriptRegistry::SetActiveSceneName(const std::string& sceneName)
	{
		s_Data.ActiveSceneName = sceneName;
	}

	const char* ScriptRegistry::GetSceneToBeLoaded()
	{
		const char* sceneName = s_Data.SceneToBeLoaded.c_str();
		return sceneName;
	}

	void ScriptRegistry::ResetSceneToBeLoaded()
	{
		s_Data.SceneToBeLoaded.clear();
	}

	void ScriptRegistry::RegisterMethods()
	{
		VX_ADD_INTERNAL_CALL(Application_Quit);
		VX_ADD_INTERNAL_CALL(Application_GetSize);
		VX_ADD_INTERNAL_CALL(Application_GetPosition);
		VX_ADD_INTERNAL_CALL(Application_IsMaximized);

		VX_ADD_INTERNAL_CALL(SceneRenderer_GetBloomThreshold);
		VX_ADD_INTERNAL_CALL(SceneRenderer_SetBloomThreshold);
		VX_ADD_INTERNAL_CALL(SceneRenderer_GetBloomSoftKnee);
		VX_ADD_INTERNAL_CALL(SceneRenderer_SetBloomSoftKnee);
		VX_ADD_INTERNAL_CALL(SceneRenderer_GetBloomUnknown);
		VX_ADD_INTERNAL_CALL(SceneRenderer_SetBloomUnknown);
		VX_ADD_INTERNAL_CALL(SceneRenderer_GetExposure);
		VX_ADD_INTERNAL_CALL(SceneRenderer_SetExposure);
		VX_ADD_INTERNAL_CALL(SceneRenderer_GetGamma);
		VX_ADD_INTERNAL_CALL(SceneRenderer_SetGamma);

		VX_ADD_INTERNAL_CALL(DebugRenderer_BeginScene);
		VX_ADD_INTERNAL_CALL(DebugRenderer_SetClearColor);
		VX_ADD_INTERNAL_CALL(DebugRenderer_DrawLine);
		VX_ADD_INTERNAL_CALL(DebugRenderer_DrawQuadBillboard);
		VX_ADD_INTERNAL_CALL(DebugRenderer_DrawCircleVec2);
		VX_ADD_INTERNAL_CALL(DebugRenderer_DrawCircleVec3);
		VX_ADD_INTERNAL_CALL(DebugRenderer_DrawBoundingBox);
		VX_ADD_INTERNAL_CALL(DebugRenderer_Flush);

		VX_ADD_INTERNAL_CALL(Scene_FindEntityByID);
		VX_ADD_INTERNAL_CALL(Scene_FindEntityByName);
		VX_ADD_INTERNAL_CALL(Scene_FindChildByName);
		VX_ADD_INTERNAL_CALL(Scene_CreateEntity);
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
		VX_ADD_INTERNAL_CALL(Entity_SetMarker);
		VX_ADD_INTERNAL_CALL(Entity_AddChild);
		VX_ADD_INTERNAL_CALL(Entity_RemoveChild);
		VX_ADD_INTERNAL_CALL(Entity_GetScriptInstance);
		VX_ADD_INTERNAL_CALL(Entity_Destroy);
		VX_ADD_INTERNAL_CALL(Entity_SetActive);

		VX_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		VX_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		VX_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		VX_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		VX_ADD_INTERNAL_CALL(TransformComponent_SetTranslationAndRotation);
		VX_ADD_INTERNAL_CALL(TransformComponent_GetRotationQuaternion);
		VX_ADD_INTERNAL_CALL(TransformComponent_SetRotationQuaternion);
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

		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetLightType);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetLightType);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetRadiance);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetRadiance);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetIntensity);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetIntensity);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetCutoff);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetCutoff);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetOuterCutoff);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetOuterCutoff);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetShadowBias);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetShadowBias);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetCastShadows);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetCastShadows);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_GetSoftShadows);
		VX_ADD_INTERNAL_CALL(LightSourceComponent_SetSoftShadows);

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

		VX_ADD_INTERNAL_CALL(StaticMeshRendererComponent_GetMeshType);
		VX_ADD_INTERNAL_CALL(StaticMeshRendererComponent_SetMeshType);

		VX_ADD_INTERNAL_CALL(Material_GetAlbedo);
		VX_ADD_INTERNAL_CALL(Material_SetAlbedo);
		VX_ADD_INTERNAL_CALL(Material_GetMetallic);
		VX_ADD_INTERNAL_CALL(Material_SetMetallic);
		VX_ADD_INTERNAL_CALL(Material_GetRoughness);
		VX_ADD_INTERNAL_CALL(Material_SetRoughness);
		VX_ADD_INTERNAL_CALL(Material_GetEmission);
		VX_ADD_INTERNAL_CALL(Material_SetEmission);
		VX_ADD_INTERNAL_CALL(Material_GetUV);
		VX_ADD_INTERNAL_CALL(Material_SetUV);
		VX_ADD_INTERNAL_CALL(Material_GetOpacity);
		VX_ADD_INTERNAL_CALL(Material_SetOpacity);

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
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_GetLockFlags);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_SetLockFlag);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_IsLockFlagSet);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_IsSleeping);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_WakeUp);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_AddForce);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_AddForceAtPosition);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_AddTorque);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_ClearTorque);
		VX_ADD_INTERNAL_CALL(RigidBodyComponent_ClearForce);

		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_Move);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_Jump); 
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_IsGrounded);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_GetSpeedDown);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_GetSlopeLimit);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_SetSlopeLimit);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_GetStepOffset);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_SetStepOffset);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_GetContactOffset);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_SetContactOffset);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_GetNonWalkableMode);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_SetNonWalkableMode);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_GetClimbMode);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_SetClimbMode);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_GetDisableGravity);
		VX_ADD_INTERNAL_CALL(CharacterControllerComponent_SetDisableGravity);

		VX_ADD_INTERNAL_CALL(FixedJointComponent_GetConnectedEntity);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_SetConnectedEntity);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_GetBreakForce);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_SetBreakForce);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_GetBreakTorque);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_SetBreakTorque);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_SetBreakForceAndTorque);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_GetEnableCollision);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_SetCollisionEnabled);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_GetPreProcessingEnabled);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_SetPreProcessingEnabled);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_IsBroken);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_GetIsBreakable);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_SetIsBreakable);
		VX_ADD_INTERNAL_CALL(FixedJointComponent_Break);

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

		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_GetTranslation);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_SetTranslation);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_GetAngle);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_SetAngle);
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
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_GetGravityScale);
		VX_ADD_INTERNAL_CALL(RigidBody2DComponent_SetGravityScale);

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

		VX_ADD_INTERNAL_CALL(Random_RangedInt32);
		VX_ADD_INTERNAL_CALL(Random_RangedFloat);
		VX_ADD_INTERNAL_CALL(Random_Float);

		VX_ADD_INTERNAL_CALL(Mathf_GetPI);
		VX_ADD_INTERNAL_CALL(Mathf_GetPI_D);
		VX_ADD_INTERNAL_CALL(Mathf_Abs);
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
		VX_ADD_INTERNAL_CALL(Mathf_LookAt);

		VX_ADD_INTERNAL_CALL(Vector3_CrossProductVec3);
		VX_ADD_INTERNAL_CALL(Vector3_DotProductVec3);

		VX_ADD_INTERNAL_CALL(Time_GetElapsed);
		VX_ADD_INTERNAL_CALL(Time_GetDeltaTime);

		VX_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		VX_ADD_INTERNAL_CALL(Input_IsKeyReleased);
		VX_ADD_INTERNAL_CALL(Input_IsKeyDown);
		VX_ADD_INTERNAL_CALL(Input_IsKeyUp);
		VX_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		VX_ADD_INTERNAL_CALL(Input_IsMouseButtonReleased);
		VX_ADD_INTERNAL_CALL(Input_IsMouseButtonDown);
		VX_ADD_INTERNAL_CALL(Input_IsMouseButtonUp);
		VX_ADD_INTERNAL_CALL(Input_GetMousePosition);
		VX_ADD_INTERNAL_CALL(Input_SetMousePosition);
		VX_ADD_INTERNAL_CALL(Input_GetMouseScrollOffset);
		VX_ADD_INTERNAL_CALL(Input_IsGamepadButtonDown);
		VX_ADD_INTERNAL_CALL(Input_IsGamepadButtonUp);
		VX_ADD_INTERNAL_CALL(Input_GetGamepadAxis);
		VX_ADD_INTERNAL_CALL(Input_GetCursorMode);
		VX_ADD_INTERNAL_CALL(Input_SetCursorMode);

		VX_ADD_INTERNAL_CALL(Gui_Begin);
		VX_ADD_INTERNAL_CALL(Gui_BeginWithPosition);
		VX_ADD_INTERNAL_CALL(Gui_BeginWithSize);
		VX_ADD_INTERNAL_CALL(Gui_BeginWithPositionAndSize);
		VX_ADD_INTERNAL_CALL(Gui_End);
		VX_ADD_INTERNAL_CALL(Gui_Separator);
		VX_ADD_INTERNAL_CALL(Gui_Spacing);
		VX_ADD_INTERNAL_CALL(Gui_Text);
		VX_ADD_INTERNAL_CALL(Gui_Button);

		VX_ADD_INTERNAL_CALL(Log_Print);
		VX_ADD_INTERNAL_CALL(Log_Info);
		VX_ADD_INTERNAL_CALL(Log_Warn);
		VX_ADD_INTERNAL_CALL(Log_Error);
		VX_ADD_INTERNAL_CALL(Log_Fatal);
	}

}
