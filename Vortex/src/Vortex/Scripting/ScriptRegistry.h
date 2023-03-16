#pragma once

#include "Vortex/Core/UUID.h"
#include "Vortex/Scene/Entity.h"

#include "Vortex/Core/Math/Math.h"

extern "C"
{
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoReflectionType MonoReflectionType;
}

namespace Vortex {

	class Entity;
	struct RaycastHit;
	struct RaycastHit2D;
	enum class KeyCode : uint16_t;
	enum class MouseButton : uint16_t;
	enum class CursorMode : uint16_t;
	enum class GamepadButton : uint16_t;
	enum class GamepadAxis : uint16_t;
	class Texture2D;
	class Noise;
	enum class NoiseType;

	class ScriptRegistry
	{
	public:
		static void RegisterMethods();
		static void RegisterComponents();
		static void SetHoveredEntity(Entity entity);
		static void SetSceneStartTime(float startTime);
		static bool HasPendingTransitionQueued();
		static uint32_t GetNextBuildIndex();
		static void ResetBuildIndex();
	};

	namespace InternalCalls {

#pragma region Application

		void Application_Quit();
		void Application_GetSize(Math::vec2* outSize);
		void Application_GetPosition(Math::vec2* outPosition);
		bool Application_IsMaximized();

#pragma endregion

#pragma region SceneRenderer

		float SceneRenderer_GetBloomThreshold();
		void SceneRenderer_SetBloomThreshold(float threshold);
		float SceneRenderer_GetBloomSoftKnee();
		void SceneRenderer_SetBloomSoftKnee(float softKnee);
		float SceneRenderer_GetBloomUnknown();
		void SceneRenderer_SetBloomUnknown(float unknown);
		float SceneRenderer_GetExposure();
		void SceneRenderer_SetExposure(float exposure);
		float SceneRenderer_GetGamma();
		void SceneRenderer_SetGamma(float gamma);

#pragma endregion

#pragma region DebugRenderer
		
		void DebugRenderer_BeginScene();
		void DebugRenderer_SetClearColor(Math::vec3* color);
		void DebugRenderer_DrawLine(Math::vec3* p1, Math::vec3* p2, Math::vec4* color);
		void DebugRenderer_DrawQuadBillboard(Math::vec3* translation, Math::vec2* size, Math::vec4* color);
		void DebugRenderer_DrawCircleVec2(Math::vec2* translation, Math::vec2* size, Math::vec4* color, float thickness, float fade);
		void DebugRenderer_DrawCircleVec3(Math::vec3* translation, Math::vec3* size, Math::vec4* color, float thickness, float fade);
		void DebugRenderer_DrawBoundingBox(Math::vec3* worldPosition, Math::vec3* size, Math::vec4* color);
		void DebugRenderer_DrawBoundingBoxFromTransform(UUID entityUUID, Math::vec4* color);
		void DebugRenderer_Flush();

#pragma endregion
		
#pragma region Scene
		
		bool Scene_FindEntityByID(UUID entityUUID);
		uint64_t Scene_FindEntityByName(MonoString* name);
		uint64_t Scene_FindChildByName(UUID entityUUID, MonoString* childName);
		uint64_t Scene_CreateEntity(MonoString* name);
		uint64_t Scene_Instantiate(UUID entityUUID);
		uint64_t Scene_InstantiateAsChild(UUID entityUUID, UUID parentUUID);
		bool Scene_IsPaused();
		void Scene_Pause();
		void Scene_Resume();
		uint64_t Scene_GetHoveredEntity();
		uint32_t Scene_GetCurrentBuildIndex();

#pragma endregion

#pragma region SceneManager

		void SceneManager_LoadScene(MonoString* sceneName);
		void SceneManager_LoadSceneFromBuildIndex(uint32_t buildIndex);
		MonoString* SceneManager_GetActiveScene();

#pragma endregion

#pragma region Entity

		void Entity_AddComponent(UUID entityUUID, MonoReflectionType* componentType);
		bool Entity_HasComponent(UUID entityUUID, MonoReflectionType* componentType);
		void Entity_RemoveComponent(UUID entityUUID, MonoReflectionType* componentType);
		MonoArray* Entity_GetChildren(UUID entityUUID);
		uint64_t Entity_GetChild(UUID entityUUID, uint32_t index);
		MonoString* Entity_GetTag(UUID entityUUID);
		MonoString* Entity_GetMarker(UUID entityUUID);
		void Entity_SetMarker(UUID entityUUID, MonoString* monoString);
		bool Entity_AddChild(UUID parentUUID, UUID childUUID);
		bool Entity_RemoveChild(UUID parentUUID, UUID childUUID);
		MonoObject* Entity_GetScriptInstance(UUID entityUUID);
		void Entity_Destroy(UUID entityUUID, bool excludeChildren);
		void Entity_DestroyTimed(UUID entityUUID, float waitTime, bool excludeChildren);
		void Entity_SetActive(UUID entityUUID, bool isActive);

#pragma endregion

#pragma region AssetHandle

		bool AssetHandle_IsValid(AssetHandle assetHandle);

#pragma endregion

#pragma region Transform Component

		void TransformComponent_GetTranslation(UUID entityUUID, Math::vec3* outTranslation);
		void TransformComponent_SetTranslation(UUID entityUUID, Math::vec3* translation);
		void TransformComponent_GetRotation(UUID entityUUID, Math::quaternion* outRotation);
		void TransformComponent_SetRotation(UUID entityUUID, Math::quaternion* rotation);
		void TransformComponent_GetEulerAngles(UUID entityUUID, Math::vec3* outEulerAngles);
		void TransformComponent_SetEulerAngles(UUID entityUUID, Math::vec3* eulerAngles);
		void TransformComponent_Rotate(UUID entityUUID, Math::vec3* eulers, Space relativeTo);
		void TransformComponent_RotateAround(UUID entityUUID, Math::vec3* worldPoint, Math::vec3* axis, float angle);
		void TransformComponent_SetTranslationAndRotation(UUID entityUUID, Math::vec3* translation, Math::vec3* rotation);
		void TransformComponent_GetScale(UUID entityUUID, Math::vec3* outScale);
		void TransformComponent_SetScale(UUID entityUUID, Math::vec3* scale);
		void TransformComponent_GetWorldSpaceTransform(UUID entityUUID, Math::vec3* outTranslation, Math::quaternion* outRotation, Math::vec3* outEulers, Math::vec3* outScale);
		void TransformComponent_GetTransformMatrix(UUID entityUUID, Math::mat4* outTransform);
		void TransformComponent_SetTransformMatrix(UUID entityUUID, Math::mat4* transform);
		void TransformComponent_GetForwardDirection(UUID entityUUID, Math::vec3* outDirection);
		void TransformComponent_GetUpDirection(UUID entityUUID, Math::vec3* outDirection);
		void TransformComponent_GetRightDirection(UUID entityUUID, Math::vec3* outDirection);
		void TransformComponent_LookAt(UUID entityUUID, Math::vec3* worldPoint);
		uint64_t TransformComponent_GetParent(UUID entityUUID);
		void TransformComponent_SetParent(UUID childUUID, UUID parentUUID);
		void TransformComponent_Unparent(UUID entityUUID);
		void TransformComponent_Multiply(TransformComponent* a, TransformComponent* b, TransformComponent* outTransform);

#pragma endregion

#pragma region Camera Component

		SceneCamera::ProjectionType CameraComponent_GetProjectionType(UUID entityUUID);
		void CameraComponent_SetProjectionType(UUID entityUUID, SceneCamera::ProjectionType type);
		void CameraComponent_GetPrimary(UUID entityUUID, bool* outPrimary);
		void CameraComponent_SetPrimary(UUID entityUUID, bool primary);
		float CameraComponent_GetPerspectiveVerticalFOV(UUID entityUUID);
		void CameraComponent_SetPerspectiveVerticalFOV(UUID entityUUID, float perspectiveVerticalFOV);
		float CameraComponent_GetNearClip(UUID entityUUID);
		void CameraComponent_SetNearClip(UUID entityUUID, float nearClip);
		float CameraComponent_GetFarClip(UUID entityUUID);
		void CameraComponent_SetFarClip(UUID entityUUID, float farClip);
		float CameraComponent_GetOrthographicSize(UUID entityUUID);
		void CameraComponent_SetOrthographicSize(UUID entityUUID, float orthographicSize);
		float CameraComponent_GetOrthographicNear(UUID entityUUID);
		void CameraComponent_SetOrthographicNear(UUID entityUUID, float orthographicNear);
		float CameraComponent_GetOrthographicFar(UUID entityUUID);
		void CameraComponent_SetOrthographicFar(UUID entityUUID, float orthographicFar);
		void CameraComponent_GetFixedAspectRatio(UUID entityUUID, bool* outFixedAspectRatio);
		void CameraComponent_SetFixedAspectRatio(UUID entityUUID, bool fixedAspectRatio);
		void CameraComponent_GetClearColor(UUID entityUUID, Math::vec3* outColor);
		void CameraComponent_SetClearColor(UUID entityUUID, Math::vec3* color);

#pragma endregion

#pragma region Light Source Component

		LightType LightSourceComponent_GetLightType(UUID entityUUID);
		void LightSourceComponent_SetLightType(UUID entityUUID, LightType type);
		void LightSourceComponent_GetRadiance(UUID entityUUID, Math::vec3* outRadiance);
		void LightSourceComponent_SetRadiance(UUID entityUUID, Math::vec3* radiance);
		float LightSourceComponent_GetIntensity(UUID entityUUID);
		void LightSourceComponent_SetIntensity(UUID entityUUID, float intensity);
		float LightSourceComponent_GetCutoff(UUID entityUUID);
		void LightSourceComponent_SetCutoff(UUID entityUUID, float cutoff);
		float LightSourceComponent_GetOuterCutoff(UUID entityUUID);
		void LightSourceComponent_SetOuterCutoff(UUID entityUUID, float outerCutoff);
		float LightSourceComponent_GetShadowBias(UUID entityUUID);
		void LightSourceComponent_SetShadowBias(UUID entityUUID, float shadowBias);
		bool LightSourceComponent_GetCastShadows(UUID entityUUID);
		void LightSourceComponent_SetCastShadows(UUID entityUUID, bool castShadows);
		bool LightSourceComponent_GetSoftShadows(UUID entityUUID);
		void LightSourceComponent_SetSoftShadows(UUID entityUUID, bool softShadows);

#pragma endregion

#pragma region TextMesh Component

		MonoString* TextMeshComponent_GetTextString(UUID entityUUID);
		void TextMeshComponent_SetTextString(UUID entityUUID, MonoString* textString);
		void TextMeshComponent_GetColor(UUID entityUUID, Math::vec4* outColor);
		void TextMeshComponent_SetColor(UUID entityUUID, Math::vec4* color);
		void TextMeshComponent_GetBackgroundColor(UUID entityUUID, Math::vec4* outBackgroundColor);
		void TextMeshComponent_SetBackgroundColor(UUID entityUUID, Math::vec4* backgroundcolor);
		float TextMeshComponent_GetLineSpacing(UUID entityUUID);
		void TextMeshComponent_SetLineSpacing(UUID entityUUID, float lineSpacing);
		float TextMeshComponent_GetKerning(UUID entityUUID);
		void TextMeshComponent_SetKerning(UUID entityUUID, float kerning);
		float TextMeshComponent_GetMaxWidth(UUID entityUUID);
		void TextMeshComponent_SetMaxWidth(UUID entityUUID, float maxWidth);

#pragma endregion

#pragma region Animator Component

		bool AnimatorComponent_IsPlaying(UUID entityUUID);
		void AnimatorComponent_Play(UUID entityUUID);
		void AnimatorComponent_Stop(UUID entityUUID);

#pragma endregion

#pragma region Mesh Renderer Component



#pragma endregion

#pragma region Static Mesh Renderer Component

		MeshType StaticMeshRendererComponent_GetMeshType(UUID entityUUID);
		void StaticMeshRendererComponent_SetMeshType(UUID entityUUID, MeshType meshType);

#pragma endregion

#pragma region Material

		void Material_GetAlbedo(UUID entityUUID, uint32_t submeshIndex, Math::vec3* outAlbedo);
		void Material_SetAlbedo(UUID entityUUID, uint32_t submeshIndex, Math::vec3* albedo);
		float Material_GetMetallic(UUID entityUUID, uint32_t submeshIndex);
		void Material_SetMetallic(UUID entityUUID, uint32_t submeshIndex, float metallic);
		float Material_GetRoughness(UUID entityUUID, uint32_t submeshIndex);
		void Material_SetRoughness(UUID entityUUID, uint32_t submeshIndex, float roughness);
		float Material_GetEmission(UUID entityUUID, uint32_t submeshIndex);
		void Material_SetEmission(UUID entityUUID, uint32_t submeshIndex, float emission);
		void Material_GetUV(UUID entityUUID, uint32_t submeshIndex, Math::vec2* outUV);
		void Material_SetUV(UUID entityUUID, uint32_t submeshIndex, Math::vec2* uv);
		float Material_GetOpacity(UUID entityUUID, uint32_t submeshIndex);
		void Material_SetOpacity(UUID entityUUID, uint32_t submeshIndex, float opacity);

#pragma endregion

#pragma region Sprite Renderer Component

		Texture2D* SpriteRendererComponent_GetTexture(UUID entityUUID);
		void SpriteRendererComponent_SetTexture(UUID entityUUID, Texture2D* unmanagedInstance);
		void SpriteRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor);
		void SpriteRendererComponent_SetColor(UUID entityUUID, Math::vec4* color);
		void SpriteRendererComponent_GetScale(UUID entityUUID, Math::vec2* outScale);
		void SpriteRendererComponent_SetScale(UUID entityUUID, Math::vec2* scale);

#pragma endregion

#pragma region Circle Renderer Component

		void CircleRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor);
		void CircleRendererComponent_SetColor(UUID entityUUID, Math::vec4* color);
		void CircleRendererComponent_GetThickness(UUID entityUUID, float* outThickness);
		void CircleRendererComponent_SetThickness(UUID entityUUID, float thickness);
		void CircleRendererComponent_GetFade(UUID entityUUID, float* outFade);
		void CircleRendererComponent_SetFade(UUID entityUUID, float fade);

#pragma endregion
		
#pragma region Particle Emitter Component

		void ParticleEmitterComponent_GetVelocity(UUID entityUUID, Math::vec3* outVelocity);
		void ParticleEmitterComponent_SetVelocity(UUID entityUUID, Math::vec3* velocity);
		void ParticleEmitterComponent_GetVelocityVariation(UUID entityUUID, Math::vec3* outVelocityVariation);
		void ParticleEmitterComponent_SetVelocityVariation(UUID entityUUID, Math::vec3* velocityVariation);
		void ParticleEmitterComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset);
		void ParticleEmitterComponent_SetOffset(UUID entityUUID, Math::vec3* offset);
		void ParticleEmitterComponent_GetSizeBegin(UUID entityUUID, Math::vec2* outSizeBegin);
		void ParticleEmitterComponent_SetSizeBegin(UUID entityUUID, Math::vec2* sizeBegin);
		void ParticleEmitterComponent_GetSizeEnd(UUID entityUUID, Math::vec2* outSizeEnd);
		void ParticleEmitterComponent_SetSizeEnd(UUID entityUUID, Math::vec2* sizeEnd);
		void ParticleEmitterComponent_GetSizeVariation(UUID entityUUID, Math::vec2* outSizeVariation);
		void ParticleEmitterComponent_SetSizeVariation(UUID entityUUID, Math::vec2* sizeVariation);
		void ParticleEmitterComponent_GetColorBegin(UUID entityUUID, Math::vec4* outColorBegin);
		void ParticleEmitterComponent_SetColorBegin(UUID entityUUID, Math::vec4* colorBegin);
		void ParticleEmitterComponent_GetColorEnd(UUID entityUUID, Math::vec4* outColorEnd);
		void ParticleEmitterComponent_SetColorEnd(UUID entityUUID, Math::vec4* colorEnd);
		void ParticleEmitterComponent_GetRotation(UUID entityUUID, float* outRotation);
		void ParticleEmitterComponent_SetRotation(UUID entityUUID, float colorEnd);
		void ParticleEmitterComponent_GetLifeTime(UUID entityUUID, float* outLifeTime);
		void ParticleEmitterComponent_SetLifeTime(UUID entityUUID, float lifetime);
		void ParticleEmitterComponent_Start(UUID entityUUID);
		void ParticleEmitterComponent_Stop(UUID entityUUID);

#pragma endregion
		
#pragma region Audio Source Component

		void AudioSourceComponent_GetPosition(UUID entityUUID, Math::vec3* outPosition);
		void AudioSourceComponent_SetPosition(UUID entityUUID, Math::vec3* position);
		void AudioSourceComponent_GetDirection(UUID entityUUID, Math::vec3* outPosition);
		void AudioSourceComponent_SetDirection(UUID entityUUID, Math::vec3* position);
		void AudioSourceComponent_GetVelocity(UUID entityUUID, Math::vec3* velocity);
		void AudioSourceComponent_SetVelocity(UUID entityUUID, Math::vec3* velocity);
		float AudioSourceComponent_GetConeInnerAngle(UUID entityUUID);
		void AudioSourceComponent_SetConeInnerAngle(UUID entityUUID, float innerAngle);
		float AudioSourceComponent_GetConeOuterAngle(UUID entityUUID);
		void AudioSourceComponent_SetConeOuterAngle(UUID entityUUID, float outerAngle);
		float AudioSourceComponent_GetConeOuterGain(UUID entityUUID);
		void AudioSourceComponent_SetConeOuterGain(UUID entityUUID, float outerGain);
		float AudioSourceComponent_GetMinDistance(UUID entityUUID);
		void AudioSourceComponent_SetMinDistance(UUID entityUUID, float minDistance);
		float AudioSourceComponent_GetMaxDistance(UUID entityUUID);
		void AudioSourceComponent_SetMaxDistance(UUID entityUUID, float maxDistance);
		float AudioSourceComponent_GetPitch(UUID entityUUID);
		void AudioSourceComponent_SetPitch(UUID entityUUID, float pitch);
		float AudioSourceComponent_GetDopplerFactor(UUID entityUUID);
		void AudioSourceComponent_SetDopplerFactor(UUID entityUUID, float dopplerFactor);
		float AudioSourceComponent_GetVolume(UUID entityUUID);
		void AudioSourceComponent_SetVolume(UUID entityUUID, float volume);
		bool AudioSourceComponent_GetPlayOnStart(UUID entityUUID);
		void AudioSourceComponent_SetPlayOnStart(UUID entityUUID, bool playOnStart);
		bool AudioSourceComponent_GetIsSpacialized(UUID entityUUID);
		void AudioSourceComponent_SetIsSpacialized(UUID entityUUID, bool spacialized);
		bool AudioSourceComponent_GetIsLooping(UUID entityUUID);
		void AudioSourceComponent_SetIsLooping(UUID entityUUID, bool loop);
		bool AudioSourceComponent_GetIsPlaying(UUID entityUUID);
		void AudioSourceComponent_Play(UUID entityUUID);
		void AudioSourceComponent_PlayOneShot(UUID entityUUID);
		void AudioSourceComponent_Restart(UUID entityUUID);
		void AudioSourceComponent_Stop(UUID entityUUID);

#pragma endregion

#pragma region Audio Clip

		MonoString* AudioClip_GetName(UUID entityUUID);
		float AudioClip_GetLength(UUID entityUUID);

#pragma endregion

#pragma region RigidBody Component

		RigidBodyType RigidBodyComponent_GetBodyType(UUID entityUUID);
		void RigidBodyComponent_SetBodyType(UUID entityUUID, RigidBodyType bodyType);
		CollisionDetectionType RigidBodyComponent_GetCollisionDetectionType(UUID entityUUID);
		void RigidBodyComponent_SetCollisionDetectionType(UUID entityUUID, CollisionDetectionType collisionDetectionType);
		float RigidBodyComponent_GetMass(UUID entityUUID);
		void RigidBodyComponent_SetMass(UUID entityUUID, float mass);
		void RigidBodyComponent_GetLinearVelocity(UUID entityUUID, Math::vec3* outVelocity);
		void RigidBodyComponent_SetLinearVelocity(UUID entityUUID, Math::vec3* velocity);
		float RigidBodyComponent_GetMaxLinearVelocity(UUID entityUUID);
		void RigidBodyComponent_SetMaxLinearVelocity(UUID entityUUID, float maxLinearVelocity);
		float RigidBodyComponent_GetLinearDrag(UUID entityUUID);
		void RigidBodyComponent_SetLinearDrag(UUID entityUUID, float drag);
		void RigidBodyComponent_GetAngularVelocity(UUID entityUUID, Math::vec3* outVelocity);
		void RigidBodyComponent_SetAngularVelocity(UUID entityUUID, Math::vec3* velocity);
		float RigidBodyComponent_GetMaxAngularVelocity(UUID entityUUID);
		void RigidBodyComponent_SetMaxAngularVelocity(UUID entityUUID, float maxAngularVelocity);
		float RigidBodyComponent_GetAngularDrag(UUID entityUUID);
		void RigidBodyComponent_SetAngularDrag(UUID entityUUID, float drag);
		bool RigidBodyComponent_GetDisableGravity(UUID entityUUID);
		void RigidBodyComponent_SetDisableGravity(UUID entityUUID, bool disabled);
		bool RigidBodyComponent_GetIsKinematic(UUID entityUUID);
		void RigidBodyComponent_SetIsKinematic(UUID entityUUID, bool isKinematic);
		void RigidBodyComponent_GetKinematicTargetTranslation(UUID entityUUID, Math::vec3* outTranslation);
		void RigidBodyComponent_SetKinematicTargetTranslation(UUID entityUUID, Math::vec3* translation);
		void RigidBodyComponent_GetKinematicTargetRotation(UUID entityUUID, Math::quaternion* outRotation);
		void RigidBodyComponent_SetKinematicTargetRotation(UUID entityUUID, Math::quaternion* rotation);
		uint32_t RigidBodyComponent_GetLockFlags(UUID entityUUID);
		void RigidBodyComponent_SetLockFlag(UUID entityUUID, ActorLockFlag flag, bool value, bool forceWake);
		bool RigidBodyComponent_IsLockFlagSet(UUID entityUUID, ActorLockFlag flag);
		bool RigidBodyComponent_IsSleeping(UUID entityUUID);
		void RigidBodyComponent_WakeUp(UUID entityUUID);
		void RigidBodyComponent_AddForce(UUID entityUUID, Math::vec3* force, ForceMode mode);
		void RigidBodyComponent_AddForceAtPosition(UUID entityUUID, Math::vec3* force, Math::vec3* position, ForceMode mode);
		void RigidBodyComponent_AddTorque(UUID entityUUID, Math::vec3* torque, ForceMode mode);
		void RigidBodyComponent_ClearTorque(UUID entityUUID, ForceMode mode);
		void RigidBodyComponent_ClearForce(UUID entityUUID, ForceMode mode);

#pragma endregion

#pragma region Physics

		bool Physics_Raycast(Math::vec3* origin, Math::vec3* direction, float maxDistance, RaycastHit* outHit);
		void Physics_GetSceneGravity(Math::vec3* outGravity);
		void Physics_SetSceneGravity(Math::vec3* gravity);
		uint32_t Physics_GetScenePositionIterations();
		void Physics_SetScenePositionIterations(uint32_t positionIterations);
		uint32_t Physics_GetSceneVelocityIterations();
		void Physics_SetSceneVelocityIterations(uint32_t velocityIterations);

#pragma endregion

#pragma region Character Controller Component

		void CharacterControllerComponent_Move(UUID entityUUID, Math::vec3* displacement);
		void CharacterControllerComponent_Jump(UUID entityUUID, float jumpForce);
		bool CharacterControllerComponent_IsGrounded(UUID entityUUID);
		void CharacterControllerComponent_GetFootPosition(UUID entityUUID, Math::vec3* outFootPos);
		float CharacterControllerComponent_GetSpeedDown(UUID entityUUID);
		float CharacterControllerComponent_GetSlopeLimit(UUID entityUUID);
		void CharacterControllerComponent_SetSlopeLimit(UUID entityUUID, float slopeLimit);
		float CharacterControllerComponent_GetStepOffset(UUID entityUUID);
		void CharacterControllerComponent_SetStepOffset(UUID entityUUID, float stepOffset);
		float CharacterControllerComponent_GetContactOffset(UUID entityUUID);
		void CharacterControllerComponent_SetContactOffset(UUID entityUUID, float contactOffset);
		NonWalkableMode CharacterControllerComponent_GetNonWalkableMode(UUID entityUUID);
		void CharacterControllerComponent_SetNonWalkableMode(UUID entityUUID, NonWalkableMode mode);
		CapsuleClimbMode CharacterControllerComponent_GetClimbMode(UUID entityUUID);
		void CharacterControllerComponent_SetClimbMode(UUID entityUUID, CapsuleClimbMode mode);
		bool CharacterControllerComponent_GetDisableGravity(UUID entityUUID);
		void CharacterControllerComponent_SetDisableGravity(UUID entityUUID, bool disableGravity);
		
#pragma endregion
		
#pragma region FixedJoint Component

		uint64_t FixedJointComponent_GetConnectedEntity(UUID entityUUID);
		void FixedJointComponent_SetConnectedEntity(UUID entityUUID, UUID connectedEntityUUID);
		float FixedJointComponent_GetBreakForce(UUID entityUUID);
		void FixedJointComponent_SetBreakForce(UUID entityUUID, float breakForce);
		float FixedJointComponent_GetBreakTorque(UUID entityUUID);
		void FixedJointComponent_SetBreakTorque(UUID entityUUID, float breakTorque);
		void FixedJointComponent_SetBreakForceAndTorque(UUID entityUUID, float breakForce, float breakTorque);
		bool FixedJointComponent_GetEnableCollision(UUID entityUUID);
		void FixedJointComponent_SetCollisionEnabled(UUID entityUUID, bool enableCollision);
		bool FixedJointComponent_GetPreProcessingEnabled(UUID entityUUID);
		void FixedJointComponent_SetPreProcessingEnabled(UUID entityUUID, bool enablePreProcessing);
		bool FixedJointComponent_IsBroken(UUID entityUUID);
		bool FixedJointComponent_GetIsBreakable(UUID entityUUID);
		void FixedJointComponent_SetIsBreakable(UUID entityUUID, bool isBreakable);
		void FixedJointComponent_Break(UUID entityUUID);

#pragma endregion

#pragma region BoxCollider Component

		void BoxColliderComponent_GetHalfSize(UUID entityUUID, Math::vec3* outHalfSize);
		void BoxColliderComponent_SetHalfSize(UUID entityUUID, Math::vec3* halfSize);
		void BoxColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset);
		void BoxColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset);
		bool BoxColliderComponent_GetIsTrigger(UUID entityUUID);
		void BoxColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger);

#pragma endregion

#pragma region SphereCollider Component

		float SphereColliderComponent_GetRadius(UUID entityUUID);
		void SphereColliderComponent_SetRadius(UUID entityUUID, float radius);
		void SphereColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset);
		void SphereColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset);
		bool SphereColliderComponent_GetIsTrigger(UUID entityUUID);
		void SphereColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger);

#pragma endregion

#pragma region CapsuleCollider Component

		float CapsuleColliderComponent_GetRadius(UUID entityUUID);
		void CapsuleColliderComponent_SetRadius(UUID entityUUID, float radius);
		float CapsuleColliderComponent_GetHeight(UUID entityUUID);
		void CapsuleColliderComponent_SetHeight(UUID entityUUID, float height);
		void CapsuleColliderComponent_GetOffset(UUID entityUUID, Math::vec3* outOffset);
		void CapsuleColliderComponent_SetOffset(UUID entityUUID, Math::vec3* offset);
		bool CapsuleColliderComponent_GetIsTrigger(UUID entityUUID);
		void CapsuleColliderComponent_SetIsTrigger(UUID entityUUID, bool isTrigger);

#pragma endregion

#pragma region RigidBody2D Component

		RigidBody2DType RigidBody2DComponent_GetBodyType(UUID entityUUID);
		void RigidBody2DComponent_SetBodyType(UUID entityUUID, RigidBody2DType bodyType);
		void RigidBody2DComponent_GetVelocity(UUID entityUUID, Math::vec2* outVelocity);
		void RigidBody2DComponent_SetVelocity(UUID entityUUID, Math::vec2* velocity);
		float RigidBody2DComponent_GetDrag(UUID entityUUID);
		void RigidBody2DComponent_SetDrag(UUID entityUUID, float drag);
		float RigidBody2DComponent_GetAngularVelocity(UUID entityUUID);
		void RigidBody2DComponent_SetAngularVelocity(UUID entityUUID, float angularVelocity);
		float RigidBody2DComponent_GetAngularDrag(UUID entityUUID);
		void RigidBody2DComponent_SetAngularDrag(UUID entityUUID, float angularDrag);
		bool RigidBody2DComponent_GetFixedRotation(UUID entityUUID);
		void RigidBody2DComponent_SetFixedRotation(UUID entityUUID, bool freeze);
		float RigidBody2DComponent_GetGravityScale(UUID entityUUID);
		void RigidBody2DComponent_SetGravityScale(UUID entityUUID, float gravityScale);
		void RigidBody2DComponent_ApplyForce(UUID entityUUID, Math::vec2* force, Math::vec2* point, bool wake);
		void RigidBody2DComponent_ApplyForceToCenter(UUID entityUUID, Math::vec2* force, bool wake);
		void RigidBody2DComponent_ApplyLinearImpulse(UUID entityUUID, Math::vec2* impulse, Math::vec2* point, bool wake);
		void RigidBody2DComponent_ApplyLinearImpulseToCenter(UUID entityUUID, Math::vec2* impulse, bool wake);

#pragma endregion

#pragma region Physics2D

		uint64_t Physics2D_Raycast(Math::vec2* start, Math::vec2* end, RaycastHit2D* outResult, bool drawDebugLine);
		void Physics2D_GetWorldGravity(Math::vec2* outGravity);
		void Physics2D_SetWorldGravity(Math::vec2* gravity);
		uint32_t Physics2D_GetWorldPositionIterations();
		void Physics2D_SetWorldPositionIterations(uint32_t positionIterations);
		uint32_t Physics2D_GetWorldVelocityIterations();
		void Physics2D_SetWorldVelocityIterations(uint32_t velocityIterations);

#pragma endregion

#pragma region BoxCollider2D Component

		void BoxCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset);
		void BoxCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset);
		void BoxCollider2DComponent_GetSize(UUID entityUUID, Math::vec2* outSize);
		void BoxCollider2DComponent_SetSize(UUID entityUUID, Math::vec2* size);
		void BoxCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity);
		void BoxCollider2DComponent_SetDensity(UUID entityUUID, float density);
		void BoxCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction);
		void BoxCollider2DComponent_SetFriction(UUID entityUUID, float friction);
		void BoxCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution);
		void BoxCollider2DComponent_SetRestitution(UUID entityUUID, float restitution);
		void BoxCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold);
		void BoxCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold);

#pragma endregion

#pragma region CircleCollider2D Component

		void CircleCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset);
		void CircleCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset);
		void CircleCollider2DComponent_GetRadius(UUID entityUUID, float* outRadius);
		void CircleCollider2DComponent_SetRadius(UUID entityUUID, float radius);
		void CircleCollider2DComponent_GetDensity(UUID entityUUID, float* outDensity);
		void CircleCollider2DComponent_SetDensity(UUID entityUUID, float density);
		void CircleCollider2DComponent_GetFriction(UUID entityUUID, float* outFriction);
		void CircleCollider2DComponent_SetFriction(UUID entityUUID, float friction);
		void CircleCollider2DComponent_GetRestitution(UUID entityUUID, float* outRestitution);
		void CircleCollider2DComponent_SetRestitution(UUID entityUUID, float restitution);
		void CircleCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float* outRestitutionThreshold);
		void CircleCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold);

#pragma endregion

#pragma region Texture2D

		Texture2D* Texture2D_LoadFromPath(MonoString* filepath);
		Texture2D* Texture2D_Constructor(uint32_t width, uint32_t height);
		uint32_t Texture2D_GetWidth(Texture2D* _this);
		uint32_t Texture2D_GetHeight(Texture2D* _this);
		void Texture2D_SetPixel(Texture2D* _this, uint32_t x, uint32_t y, Math::vec4* color);

#pragma endregion

#pragma region Random

		int Random_RangedInt32(int min, int max);
		float Random_RangedFloat(float min, float max);
		float Random_Float();

#pragma endregion

#pragma region Matrix4
		
		void Matrix4_Rotate(float angleDeg, Math::vec3* axis, Math::mat4* outResult);
		void Matrix4_LookAt(Math::vec3* eyePos, Math::vec3* worldPoint, Math::vec3* up, Math::mat4* outResult);
		void Matrix4_Multiply(Math::mat4* matrix, Math::mat4* other, Math::mat4* outResult);

#pragma endregion

#pragma region Mathf

		float Mathf_GetPI();
		double Mathf_GetPI_D();
		float Mathf_Round(float value);
		float Mathf_Abs(float in);
		float Mathf_Sqrt(float in);
		float Mathf_Pow(float base, float power);
		float Mathf_Sin(float in);
		float Mathf_Cos(float in);
		float Mathf_Acos(float in);
		float Mathf_Tan(float in);
		float Mathf_Max(float x, float y);
		float Mathf_Min(float x, float y);
		float Mathf_Deg2Rad(float degrees);
		float Mathf_Rad2Deg(float radians);
		void Mathf_Deg2RadVector3(Math::vec3* value, Math::vec3* outResult);
		void Mathf_Rad2DegVector3(Math::vec3* value, Math::vec3* outResult);
		void Mathf_LookAt(Math::vec3* eyePos, Math::vec3* worldPoint, Math::vec3* outRotation);
		void Mathf_InverseQuat(Math::quaternion* rotation, Math::quaternion* result);

#pragma endregion

#pragma region Noise

		Noise* Noise_Constructor(int seed, NoiseType type);
		void Noise_Destructor(Noise* _this);
		float Noise_GetFrequency(Noise* _this);
		void Noise_SetFrequency(Noise* _this, float frequency);
		int Noise_GetFractalOctaves(Noise* _this);
		void Noise_SetFractalOctaves(Noise* _this, int octaves);
		float Noise_GetFractalLacunarity(Noise* _this);
		void Noise_SetFractalLacunarity(Noise* _this, float lacunarity);
		float Noise_GetFractalGain(Noise* _this);
		void Noise_SetFractalGain(Noise* _this, float gain);
		float Noise_GetVec2(Noise* _this, float x, float y);
		float Noise_GetVec3(Noise* _this, float x, float y, float z);
		void Noise_SetSeed(int seed);
		float Noise_PerlinNoiseVec2(float x, float y);
		float Noise_PerlinNoiseVec3(float x, float y, float z);

#pragma endregion

#pragma region Time

		float Time_GetElapsed();
		float Time_GetDeltaTime();

#pragma endregion

#pragma region Input

		bool Input_IsKeyPressed(KeyCode key);
		bool Input_IsKeyReleased(KeyCode key);
		bool Input_IsKeyDown(KeyCode key);
		bool Input_IsKeyUp(KeyCode key);
		bool Input_IsMouseButtonPressed(MouseButton mouseButton);
		bool Input_IsMouseButtonReleased(MouseButton mouseButton);
		bool Input_IsMouseButtonDown(MouseButton mouseButton);
		bool Input_IsMouseButtonUp(MouseButton mouseButton);
		void Input_GetMousePosition(Math::vec2* outPosition);
		void Input_SetMousePosition(Math::vec2* position);
		void Input_GetMouseWheelMovement(Math::vec2* outMouseScrollOffset);
		bool Input_IsGamepadButtonDown(GamepadButton gamepadButton);
		bool Input_IsGamepadButtonUp(GamepadButton gamepadButton);
		float Input_GetGamepadAxis(GamepadAxis gamepadAxis);
		CursorMode Input_GetCursorMode();
		void Input_SetCursorMode(CursorMode cursorMode);

#pragma endregion

#pragma region Gui

		void BeginWindow(char* text, uint32_t flags = 0);
		void Gui_Begin(MonoString* text);
		void Gui_BeginWithPosition(MonoString* text, Math::vec2* position);
		void Gui_BeginWithSize(MonoString* text, float width, float height);
		void Gui_BeginWithPositionAndSize(MonoString* text, Math::vec2* position, Math::vec2* size);
		void Gui_End();
		void Gui_Underline();
		void Gui_Spacing(unsigned int count);
		void Gui_Text(MonoString* text);
		bool Gui_Button(MonoString* text);
		bool Gui_PropertyBool(MonoString* label, bool* value);
		bool Gui_PropertyInt(MonoString* label, int* value);
		bool Gui_PropertyULong(MonoString* label, unsigned int* value);
		bool Gui_PropertyFloat(MonoString* label, float* value);
		bool Gui_PropertyDouble(MonoString* label, double* value);
		bool Gui_PropertyVec2(MonoString* label, Math::vec2* value);
		bool Gui_PropertyVec3(MonoString* label, Math::vec3* value);
		bool Gui_PropertyVec4(MonoString* label, Math::vec4* value);
		bool Gui_PropertyColor3(MonoString* label, Math::vec3* value);
		bool Gui_PropertyColor4(MonoString* label, Math::vec4* value);

#pragma endregion

		void Log_Message(MonoString* message, Log::LogLevel type);

	}

}
