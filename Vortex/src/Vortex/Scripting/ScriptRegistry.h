#pragma once

#include "Vortex/Core/UUID.h"
#include "Vortex/Scene/Entity.h"
#include <Vortex/Core/KeyCodes.h>

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

	class ScriptRegistry
	{
	public:
		static void RegisterMethods();
		static void RegisterComponents();
		static void SetHoveredEntity(Entity entity);
		static void SetSceneStartTime(float startTime);
		static void SetActiveSceneName(const std::string& sceneName);
		static const char* GetSceneToBeLoaded();
		static void ResetSceneToBeLoaded();
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
		void DebugRenderer_Flush();

#pragma endregion
		
#pragma region Scene
		
		bool Scene_FindEntityByID(UUID entityUUID);
		uint64_t Scene_FindEntityByName(MonoString* name);
		uint64_t Scene_FindChildByName(UUID entityUUID, MonoString* childName);
		uint64_t Scene_CreateEntity(MonoString* name);
		uint64_t Scene_Instantiate(UUID entityUUID);
		uint64_t Scene_InstantiateAtWorldPosition(UUID entityUUID, Math::vec3* worldPosition);
		bool Scene_IsPaused();
		void Scene_Pause();
		void Scene_Resume();
		uint64_t Scene_GetHoveredEntity();

#pragma endregion

#pragma region SceneManager

		void SceneManager_LoadScene(MonoString* sceneName);
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
		void Entity_SetActive(UUID entityUUID, bool isActive);

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
		void TransformComponent_GetWorldSpaceTransform(UUID entityUUID, Math::vec3* outTranslation, Math::vec3* outRotationEuler, Math::vec3* outScale);
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

		void CameraComponent_GetPrimary(UUID entityUUID, bool* outPrimary);
		void CameraComponent_SetPrimary(UUID entityUUID, bool primary);
		float CameraComponent_GetPerspectiveVerticalFOV(UUID entityUUID);
		void CameraComponent_SetPerspectiveVerticalFOV(UUID entityUUID, float perspectiveVerticalFOV);
		void CameraComponent_GetFixedAspectRatio(UUID entityUUID, bool* outFixedAspectRatio);
		void CameraComponent_SetFixedAspectRatio(UUID entityUUID, bool fixedAspectRatio);

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

		void SpriteRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor);
		void SpriteRendererComponent_SetColor(UUID entityUUID, Math::vec4* color);
		void SpriteRendererComponent_GetScale(UUID entityUUID, Math::vec2* outScale);
		MonoString* SpriteRendererComponent_GetTexture(UUID entityUUID);
		void SpriteRendererComponent_SetTexture(UUID entityUUID, MonoString* texturePathString);
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

		bool AudioSourceComponent_GetIsPlaying(UUID entityUUID);
		void AudioSourceComponent_Play(UUID entityUUID);
		void AudioSourceComponent_PlayOneShot(UUID entityUUID);
		void AudioSourceComponent_Restart(UUID entityUUID);
		void AudioSourceComponent_Stop(UUID entityUUID);
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
		float RigidBodyComponent_GetLinearDrag(UUID entityUUID);
		void RigidBodyComponent_SetLinearDrag(UUID entityUUID, float drag);
		void RigidBodyComponent_GetAngularVelocity(UUID entityUUID, Math::vec3* outVelocity);
		void RigidBodyComponent_SetAngularVelocity(UUID entityUUID, Math::vec3* velocity);
		float RigidBodyComponent_GetAngularDrag(UUID entityUUID);
		void RigidBodyComponent_SetAngularDrag(UUID entityUUID, float drag);
		bool RigidBodyComponent_GetDisableGravity(UUID entityUUID);
		void RigidBodyComponent_SetDisableGravity(UUID entityUUID, bool disabled);
		bool RigidBodyComponent_GetIsKinematic(UUID entityUUID);
		void RigidBodyComponent_SetIsKinematic(UUID entityUUID, bool isKinematic);
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
		void RigidBody2DComponent_ApplyForce(UUID entityUUID, Math::vec2* force, Math::vec2* point, bool wake);
		void RigidBody2DComponent_ApplyForceToCenter(UUID entityUUID, Math::vec2* force, bool wake);
		void RigidBody2DComponent_ApplyLinearImpulse(UUID entityUUID, Math::vec2* impulse, Math::vec2* point, bool wake);
		void RigidBody2DComponent_ApplyLinearImpulseToCenter(UUID entityUUID, Math::vec2* impulse, bool wake);
		void RigidBody2DComponent_GetVelocity(UUID entityUUID, Math::vec2* outVelocity);
		void RigidBody2DComponent_SetVelocity(UUID entityUUID, Math::vec2* velocity);
		float RigidBody2DComponent_GetDrag(UUID entityUUID);
		void RigidBody2DComponent_SetDrag(UUID entityUUID, float drag);
		bool RigidBody2DComponent_GetFixedRotation(UUID entityUUID);
		void RigidBody2DComponent_SetFixedRotation(UUID entityUUID, bool freeze);
		float RigidBody2DComponent_GetGravityScale(UUID entityUUID);
		void RigidBody2DComponent_SetGravityScale(UUID entityUUID, float gravityScale);

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

#pragma region Random

		int Random_RangedInt32(int min, int max);
		float Random_RangedFloat(float min, float max);
		float Random_Float();

#pragma endregion

#pragma region Mathf

		float Mathf_GetPI();
		double Mathf_GetPI_D();
		float Mathf_Abs(float in);
		float Mathf_Sqrt(float in);
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

#pragma endregion

#pragma region Quaternion



#pragma endregion

#pragma region Vector3

		void Vector3_CrossProductVec3(Math::vec3* left, Math::vec3* right, Math::vec3* outResult);
		float Vector3_DotProductVec3(Math::vec3* left, Math::vec3* right);

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
		void Input_GetMouseScrollOffset(Math::vec2* outMouseScrollOffset);
		bool Input_IsGamepadButtonDown(Gamepad button);
		bool Input_IsGamepadButtonUp(Gamepad button);
		float Input_GetGamepadAxis(Gamepad axis);
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
		void Gui_Separator();
		void Gui_Spacing();
		void Gui_Text(MonoString* text);
		bool Gui_Button(MonoString* text);

#pragma endregion

		void Log_Print(MonoString* message);
		void Log_Info(MonoString* message);
		void Log_Warn(MonoString* message);
		void Log_Error(MonoString* message);
		void Log_Fatal(MonoString* message);

	}

}
