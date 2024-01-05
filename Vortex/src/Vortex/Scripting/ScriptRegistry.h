#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"

#include "Vortex/Math/Math.h"
#include "Vortex/Math/AABB.h"
#include "Vortex/Math/Ray.h"

#include "Vortex/Scene/Components.h"

extern "C"
{
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoReflectionType MonoReflectionType;
}

namespace Vortex {

	class Actor;
	struct RaycastHit;
	struct RaycastHit2D;
	enum class KeyCode : uint16_t;
	enum class MouseButton : uint16_t;
	enum class CursorMode : uint16_t;
	enum class GamepadButton : uint16_t;
	enum class GamepadAxis : uint16_t;
	enum class AttenuationModel : uint8_t;
	enum class PanMode : uint8_t;
	enum class PositioningMode : uint8_t;
	enum class MaterialFlag;
	class Texture2D;
	class Noise;
	enum class NoiseType;

	class VORTEX_API ScriptRegistry
	{
	public:
		static void RegisterInternalCalls();
		static void RegisterComponents();
		static void SetSceneStartTime(float currentTime);
		static bool SavePlayerPrefs();
		static bool LoadPlayerPrefs();
	};

	namespace InternalCalls {

#pragma region Application

		void Application_Quit();

#pragma endregion

#pragma region Window

		void Window_GetSize(Math::vec2* outSize);
		void Window_GetPosition(Math::vec2* outPosition);
		bool Window_IsMaximized();
		void Window_SetMaximized(bool maximized);
		bool Window_IsResizeable();
		void Window_SetResizeable(bool resizeable);
		bool Window_IsDecorated();
		void Window_SetDecorated(bool decorated);
		bool Window_IsVSyncEnabled();
		void Window_SetVSync(bool use);

#pragma endregion

#pragma region SceneRenderer

		float SceneRenderer_GetExposure();
		void SceneRenderer_SetExposure(float exposure);
		float SceneRenderer_GetGamma();
		void SceneRenderer_SetGamma(float gamma);

#pragma endregion

#pragma region DebugRenderer
		
		void DebugRenderer_DrawLine(const Math::vec3* startPoint, const Math::vec3* endPoint, const Math::vec4* color);
		void DebugRenderer_SetLineWidth(float width);
		void DebugRenderer_DrawQuadBillboard(const Math::vec3* translation, const Math::vec2* size, const Math::vec4* color);
		void DebugRenderer_DrawCircleVec2(const Math::vec2* translation, const Math::vec2* size, const Math::vec4* color, float thickness, float fade);
		void DebugRenderer_DrawCircleVec3(const Math::vec3* translation, const Math::vec3* size, const Math::vec4* color, float thickness, float fade);
		void DebugRenderer_DrawBoundingBox(const Math::vec3* worldPosition, const Math::vec3* size, const Math::vec4* color);
		void DebugRenderer_DrawBoundingBoxFromTransform(UUID actorUUID, const Math::vec4* color);
		void DebugRenderer_Flush();

#pragma endregion
		
#pragma region Scene
		
		uint64_t Scene_GetPrimaryCamera();
		bool Scene_FindActorByID(UUID actorUUID);
		uint64_t Scene_FindActorByName(MonoString* name);
		uint64_t Scene_FindChildByName(UUID actorUUID, MonoString* childName);
		uint64_t Scene_CreateActor(MonoString* name);
		uint64_t Scene_Instantiate(UUID actorUUID);
		uint64_t Scene_InstantiateAsChild(UUID actorUUID, UUID parentUUID);
		bool Scene_IsPaused();
		void Scene_Pause();
		void Scene_Resume();

#pragma endregion

#pragma region SceneManager

		void SceneManager_LoadScene(MonoString* sceneName);

#pragma endregion

#pragma region Actor

		void Actor_AddComponent(UUID actorUUID, MonoReflectionType* componentType);
		bool Actor_HasComponent(UUID actorUUID, MonoReflectionType* componentType);
		void Actor_RemoveComponent(UUID actorUUID, MonoReflectionType* componentType);
		MonoArray* Actor_GetChildren(UUID actorUUID);
		uint64_t Actor_GetChild(UUID actorUUID, uint32_t index);
		MonoString* Actor_GetTag(UUID actorUUID);
		void Actor_SetTag(UUID actorUUID, MonoString* tag);
		MonoString* Actor_GetMarker(UUID actorUUID);
		void Actor_SetMarker(UUID actorUUID, MonoString* marker);
		bool Actor_AddChild(UUID parentUUID, UUID childUUID);
		bool Actor_RemoveChild(UUID parentUUID, UUID childUUID);
		MonoObject* Actor_GetScriptInstance(UUID actorUUID);
		void Actor_Destroy(UUID actorUUID, bool excludeChildren);
		void Actor_DestroyWithDelay(UUID actorUUID, float delay, bool excludeChildren);
		void Actor_Invoke(UUID actorUUID, MonoString* methodName);
		void Actor_InvokeWithDelay(UUID actorUUID, MonoString* methodName, float delay);
		bool Actor_IsActive(UUID actorUUID);
		void Actor_SetActive(UUID actorUUID, bool isActive);
		void Actor_AddTimer(UUID actorUUID, MonoString* name, float delay);
		bool Actor_IsValid(UUID actorUUID);

#pragma endregion

#pragma region AssetHandle

		bool AssetHandle_IsValid(AssetHandle* assetHandle);

#pragma endregion

#pragma region Timer

		float Timer_GetTimeLeft(UUID actorUUID, MonoString* name);
		bool Timer_IsStarted(UUID actorUUID, MonoString* name);
		bool Timer_IsFinished(UUID actorUUID, MonoString* name);
		void Timer_Start(UUID actorUUID, MonoString* name);

#pragma endregion

#pragma region Transform Component

		void TransformComponent_GetTranslation(UUID actorUUID, Math::vec3* outTranslation);
		void TransformComponent_SetTranslation(UUID actorUUID, Math::vec3* translation);
		void TransformComponent_GetRotation(UUID actorUUID, Math::quaternion* outRotation);
		void TransformComponent_SetRotation(UUID actorUUID, Math::quaternion* rotation);
		void TransformComponent_GetEulerAngles(UUID actorUUID, Math::vec3* outEulerAngles);
		void TransformComponent_SetEulerAngles(UUID actorUUID, Math::vec3* eulerAngles);
		void TransformComponent_Rotate(UUID actorUUID, Math::vec3* eulers, Space relativeTo);
		void TransformComponent_RotateAround(UUID actorUUID, Math::vec3* worldPoint, Math::vec3* axis, float angle);
		void TransformComponent_SetTranslationAndRotation(UUID actorUUID, Math::vec3* translation, Math::vec3* rotation);
		void TransformComponent_GetScale(UUID actorUUID, Math::vec3* outScale);
		void TransformComponent_SetScale(UUID actorUUID, Math::vec3* scale);
		void TransformComponent_GetWorldSpaceTransform(UUID actorUUID, Math::vec3* outTranslation, Math::quaternion* outRotation, Math::vec3* outEulers, Math::vec3* outScale);
		void TransformComponent_GetTransformMatrix(UUID actorUUID, Math::mat4* outTransform);
		void TransformComponent_SetTransformMatrix(UUID actorUUID, Math::mat4* transform);
		void TransformComponent_GetForwardDirection(UUID actorUUID, Math::vec3* outDirection);
		void TransformComponent_GetBackwardDirection(UUID actorUUID, Math::vec3* outDirection);
		void TransformComponent_GetUpDirection(UUID actorUUID, Math::vec3* outDirection);
		void TransformComponent_GetDownDirection(UUID actorUUID, Math::vec3* outDirection);
		void TransformComponent_GetRightDirection(UUID actorUUID, Math::vec3* outDirection);
		void TransformComponent_GetLeftDirection(UUID actorUUID, Math::vec3* outDirection);
		void TransformComponent_LookAt(UUID actorUUID, Math::vec3* worldPoint);
		uint64_t TransformComponent_GetParent(UUID actorUUID);
		void TransformComponent_SetParent(UUID childUUID, UUID parentUUID);
		void TransformComponent_Unparent(UUID actorUUID);
		void TransformComponent_Multiply(TransformComponent* a, TransformComponent* b, TransformComponent* outTransform);

#pragma endregion

#pragma region Camera Component

		SceneCamera::ProjectionType CameraComponent_GetProjectionType(UUID actorUUID);
		void CameraComponent_SetProjectionType(UUID actorUUID, SceneCamera::ProjectionType type);
		void CameraComponent_GetPrimary(UUID actorUUID, bool* outPrimary);
		void CameraComponent_SetPrimary(UUID actorUUID, bool primary);
		float CameraComponent_GetPerspectiveVerticalFOV(UUID actorUUID);
		void CameraComponent_SetPerspectiveVerticalFOV(UUID actorUUID, float perspectiveVerticalFOV);
		float CameraComponent_GetNearClip(UUID actorUUID);
		void CameraComponent_SetNearClip(UUID actorUUID, float nearClip);
		float CameraComponent_GetFarClip(UUID actorUUID);
		void CameraComponent_SetFarClip(UUID actorUUID, float farClip);
		float CameraComponent_GetOrthographicSize(UUID actorUUID);
		void CameraComponent_SetOrthographicSize(UUID actorUUID, float orthographicSize);
		float CameraComponent_GetOrthographicNear(UUID actorUUID);
		void CameraComponent_SetOrthographicNear(UUID actorUUID, float orthographicNear);
		float CameraComponent_GetOrthographicFar(UUID actorUUID);
		void CameraComponent_SetOrthographicFar(UUID actorUUID, float orthographicFar);
		void CameraComponent_GetFixedAspectRatio(UUID actorUUID, bool* outFixedAspectRatio);
		void CameraComponent_SetFixedAspectRatio(UUID actorUUID, bool fixedAspectRatio);
		void CameraComponent_GetClearColor(UUID actorUUID, Math::vec3* outColor);
		void CameraComponent_SetClearColor(UUID actorUUID, Math::vec3* color);
		void CameraComponent_Raycast(UUID actorUUID, Math::vec3* position, float maxDistance, Math::Ray* outRay);
		void CameraComponent_ScreenToWorldPoint(UUID actorUUID, Math::vec2* position, float maxDistance, Math::vec3* outWorldPoint);
		void CameraComponent_ScreenToViewportPoint(UUID actorUUID, Math::vec2* position, Math::vec2* outViewportPoint);

#pragma endregion

#pragma region PostProcessInfo

		bool PostProcessInfo_GetEnabled(UUID actorUUID);
		void PostProcessInfo_SetEnabled(UUID actorUUID, bool enabled);

#pragma endregion

#pragma region BloomInfo

		float BloomInfo_GetThreshold(UUID actorUUID);
		void BloomInfo_SetThreshold(UUID actorUUID, float threshold);
		float BloomInfo_GetKnee(UUID actorUUID);
		void BloomInfo_SetKnee(UUID actorUUID, float knee);
		float BloomInfo_GetIntensity(UUID actorUUID);
		void BloomInfo_SetIntensity(UUID actorUUID, float intensity);
		bool BloomInfo_GetEnabled(UUID actorUUID);
		void BloomInfo_SetEnabled(UUID actorUUID, bool enabled);

#pragma endregion

#pragma region Light Source Component

		LightType LightSourceComponent_GetLightType(UUID actorUUID);
		void LightSourceComponent_SetLightType(UUID actorUUID, LightType type);
		void LightSourceComponent_GetRadiance(UUID actorUUID, Math::vec3* outRadiance);
		void LightSourceComponent_SetRadiance(UUID actorUUID, Math::vec3* radiance);
		float LightSourceComponent_GetIntensity(UUID actorUUID);
		void LightSourceComponent_SetIntensity(UUID actorUUID, float intensity);
		float LightSourceComponent_GetCutoff(UUID actorUUID);
		void LightSourceComponent_SetCutoff(UUID actorUUID, float cutoff);
		float LightSourceComponent_GetOuterCutoff(UUID actorUUID);
		void LightSourceComponent_SetOuterCutoff(UUID actorUUID, float outerCutoff);
		float LightSourceComponent_GetShadowBias(UUID actorUUID);
		void LightSourceComponent_SetShadowBias(UUID actorUUID, float shadowBias);
		bool LightSourceComponent_GetCastShadows(UUID actorUUID);
		void LightSourceComponent_SetCastShadows(UUID actorUUID, bool castShadows);
		bool LightSourceComponent_GetSoftShadows(UUID actorUUID);
		void LightSourceComponent_SetSoftShadows(UUID actorUUID, bool softShadows);
		bool LightSourceComponent_IsVisible(UUID actorUUID);
		void LightSourceComponent_SetVisible(UUID actorUUID, bool visible);

#pragma endregion

#pragma region TextMesh Component

		MonoString* TextMeshComponent_GetTextString(UUID actorUUID);
		void TextMeshComponent_SetTextString(UUID actorUUID, MonoString* textString);
		void TextMeshComponent_GetColor(UUID actorUUID, Math::vec4* outColor);
		void TextMeshComponent_SetColor(UUID actorUUID, Math::vec4* color);
		void TextMeshComponent_GetOutlineColor(UUID actorUUID, Math::vec4* outOutlineColor);
		void TextMeshComponent_SetOutlineColor(UUID actorUUID, Math::vec4* outlineColor);
		float TextMeshComponent_GetLineSpacing(UUID actorUUID);
		void TextMeshComponent_SetLineSpacing(UUID actorUUID, float lineSpacing);
		float TextMeshComponent_GetKerning(UUID actorUUID);
		void TextMeshComponent_SetKerning(UUID actorUUID, float kerning);
		float TextMeshComponent_GetMaxWidth(UUID actorUUID);
		void TextMeshComponent_SetMaxWidth(UUID actorUUID, float maxWidth);
		bool TextMeshComponent_IsVisible(UUID actorUUID);
		void TextMeshComponent_SetVisible(UUID actorUUID, bool visible);

#pragma endregion

#pragma region Animator Component

		bool AnimatorComponent_IsPlaying(UUID actorUUID);
		void AnimatorComponent_Play(UUID actorUUID);
		void AnimatorComponent_Stop(UUID actorUUID);

#pragma endregion

#pragma region Mesh Renderer Component

		bool MeshRendererComponent_GetMaterialHandle(uint32_t submeshIndex, UUID actorUUID, AssetHandle* outHandle);
		bool MeshRendererComponent_IsVisible(UUID actorUUID);
		void MeshRendererComponent_SetVisible(UUID actorUUID, bool visible);

#pragma endregion

#pragma region Static Mesh Renderer Component

		MeshType StaticMeshRendererComponent_GetMeshType(UUID actorUUID);
		void StaticMeshRendererComponent_SetMeshType(UUID actorUUID, MeshType meshType);
		void StaticMeshRendererComponent_SetMaterialHandle(uint32_t submeshIndex, UUID actorUUID, AssetHandle* materialHandle);
		bool StaticMeshRendererComponent_IsVisible(UUID actorUUID);
		void StaticMeshRendererComponent_SetVisible(UUID actorUUID, bool visible);
		bool StaticMeshRendererComponent_GetMaterialHandle(uint32_t submeshIndex, UUID actorUUID, AssetHandle* outHandle);

#pragma endregion

#pragma region Material

		void Material_GetAlbedo(AssetHandle* assetHandle, Math::vec3* outAlbedo);
		void Material_SetAlbedo(AssetHandle* assetHandle, Math::vec3* albedo);
		bool Material_GetAlbedoMap(AssetHandle* assetHandle, AssetHandle* outHandle);
		void Material_SetAlbedoMap(AssetHandle* assetHandle, AssetHandle* textureHandle);
		bool Material_GetNormalMap(AssetHandle* assetHandle, AssetHandle* outHandle);
		void Material_SetNormalMap(AssetHandle* assetHandle, AssetHandle* textureHandle);
		float Material_GetMetallic(AssetHandle* assetHandle);
		void Material_SetMetallic(AssetHandle* assetHandle, float metallic);
		bool Material_GetMetallicMap(AssetHandle* assetHandle, AssetHandle* outHandle);
		void Material_SetMetallicMap(AssetHandle* assetHandle, AssetHandle* textureHandle);
		float Material_GetRoughness(AssetHandle* assetHandle);
		void Material_SetRoughness(AssetHandle* assetHandle, float roughness);
		bool Material_GetRoughnessMap(AssetHandle* assetHandle, AssetHandle* outHandle);
		void Material_SetRoughnessMap(AssetHandle* assetHandle, AssetHandle* textureHandle);
		float Material_GetEmission(AssetHandle* assetHandle);
		void Material_SetEmission(AssetHandle* assetHandle, float emission);
		bool Material_GetEmissionMap(AssetHandle* assetHandle, AssetHandle* outHandle);
		void Material_SetEmissionMap(AssetHandle* assetHandle, AssetHandle* textureHandle);
		bool Material_GetAmbientOcclusionMap(AssetHandle* assetHandle, AssetHandle* outHandle);
		void Material_SetAmbientOcclusionMap(AssetHandle* assetHandle, AssetHandle* textureHandle);
		void Material_GetUV(AssetHandle* assetHandle, Math::vec2* outUV);
		void Material_SetUV(AssetHandle* assetHandle, Math::vec2* uv);
		float Material_GetOpacity(AssetHandle* assetHandle);
		void Material_SetOpacity(AssetHandle* assetHandle, float opacity);
		bool Material_IsFlagSet(AssetHandle* assetHandle, MaterialFlag flag);
		void Material_SetFlag(AssetHandle* assetHandle, MaterialFlag flag, bool value);

#pragma endregion

#pragma region Sprite Renderer Component

		bool SpriteRendererComponent_GetTextureHandle(UUID actorUUID, AssetHandle* outHandle);
		void SpriteRendererComponent_SetTextureHandle(UUID actorUUID, AssetHandle* textureHandle);
		void SpriteRendererComponent_GetColor(UUID actorUUID, Math::vec4* outColor);
		void SpriteRendererComponent_SetColor(UUID actorUUID, Math::vec4* color);
		void SpriteRendererComponent_GetUV(UUID actorUUID, Math::vec2* outScale);
		void SpriteRendererComponent_SetUV(UUID actorUUID, Math::vec2* scale);
		bool SpriteRendererComponent_IsVisible(UUID actorUUID);
		void SpriteRendererComponent_SetVisible(UUID actorUUID, bool visible);

#pragma endregion

#pragma region Circle Renderer Component

		void CircleRendererComponent_GetColor(UUID actorUUID, Math::vec4* outColor);
		void CircleRendererComponent_SetColor(UUID actorUUID, Math::vec4* color);
		void CircleRendererComponent_GetThickness(UUID actorUUID, float* outThickness);
		void CircleRendererComponent_SetThickness(UUID actorUUID, float thickness);
		void CircleRendererComponent_GetFade(UUID actorUUID, float* outFade);
		void CircleRendererComponent_SetFade(UUID actorUUID, float fade);
		bool CircleRendererComponent_IsVisible(UUID actorUUID);
		void CircleRendererComponent_SetVisible(UUID actorUUID, bool visible);

#pragma endregion
		
#pragma region Particle Emitter Component

		void ParticleEmitterComponent_GetVelocity(UUID actorUUID, Math::vec3* outVelocity);
		void ParticleEmitterComponent_SetVelocity(UUID actorUUID, Math::vec3* velocity);
		void ParticleEmitterComponent_GetVelocityVariation(UUID actorUUID, Math::vec3* outVelocityVariation);
		void ParticleEmitterComponent_SetVelocityVariation(UUID actorUUID, Math::vec3* velocityVariation);
		void ParticleEmitterComponent_GetOffset(UUID actorUUID, Math::vec3* outOffset);
		void ParticleEmitterComponent_SetOffset(UUID actorUUID, Math::vec3* offset);
		void ParticleEmitterComponent_GetSizeBegin(UUID actorUUID, Math::vec2* outSizeBegin);
		void ParticleEmitterComponent_SetSizeBegin(UUID actorUUID, Math::vec2* sizeBegin);
		void ParticleEmitterComponent_GetSizeEnd(UUID actorUUID, Math::vec2* outSizeEnd);
		void ParticleEmitterComponent_SetSizeEnd(UUID actorUUID, Math::vec2* sizeEnd);
		void ParticleEmitterComponent_GetSizeVariation(UUID actorUUID, Math::vec2* outSizeVariation);
		void ParticleEmitterComponent_SetSizeVariation(UUID actorUUID, Math::vec2* sizeVariation);
		void ParticleEmitterComponent_GetColorBegin(UUID actorUUID, Math::vec4* outColorBegin);
		void ParticleEmitterComponent_SetColorBegin(UUID actorUUID, Math::vec4* colorBegin);
		void ParticleEmitterComponent_GetColorEnd(UUID actorUUID, Math::vec4* outColorEnd);
		void ParticleEmitterComponent_SetColorEnd(UUID actorUUID, Math::vec4* colorEnd);
		void ParticleEmitterComponent_GetRotation(UUID actorUUID, float* outRotation);
		void ParticleEmitterComponent_SetRotation(UUID actorUUID, float colorEnd);
		void ParticleEmitterComponent_GetLifeTime(UUID actorUUID, float* outLifeTime);
		void ParticleEmitterComponent_SetLifeTime(UUID actorUUID, float lifetime);
		void ParticleEmitterComponent_Start(UUID actorUUID);
		void ParticleEmitterComponent_Stop(UUID actorUUID);
		bool ParticleEmitterComponent_IsActive(UUID actorUUID);

#pragma endregion
		
#pragma region Audio Source Component

		void AudioSourceComponent_GetPosition(UUID actorUUID, Math::vec3* outPosition);
		void AudioSourceComponent_SetPosition(UUID actorUUID, Math::vec3* position);
		void AudioSourceComponent_GetDirection(UUID actorUUID, Math::vec3* outPosition);
		void AudioSourceComponent_SetDirection(UUID actorUUID, Math::vec3* position);
		void AudioSourceComponent_GetVelocity(UUID actorUUID, Math::vec3* velocity);
		void AudioSourceComponent_SetVelocity(UUID actorUUID, Math::vec3* velocity);
		float AudioSourceComponent_GetMinGain(UUID actorUUID);
		void AudioSourceComponent_SetMinGain(UUID actorUUID, float minGain);
		float AudioSourceComponent_GetMaxGain(UUID actorUUID);
		void AudioSourceComponent_SetMaxGain(UUID actorUUID, float maxGain);
		float AudioSourceComponent_GetDirectionalAttenuationFactor(UUID actorUUID);
		void AudioSourceComponent_SetDirectionalAttenuationFactor(UUID actorUUID, float factor);
		AttenuationModel AudioSourceComponent_GetAttenuationModel(UUID actorUUID);
		void AudioSourceComponent_SetAttenuationModel(UUID actorUUID, AttenuationModel model);
		float AudioSourceComponent_GetPan(UUID actorUUID);
		void AudioSourceComponent_SetPan(UUID actorUUID, float pan);
		PanMode AudioSourceComponent_GetPanMode(UUID actorUUID);
		void AudioSourceComponent_SetPanMode(UUID actorUUID, PanMode mode);
		PositioningMode AudioSourceComponent_GetPositioningMode(UUID actorUUID);
		void AudioSourceComponent_SetPositioningMode(UUID actorUUID, PositioningMode mode);
		float AudioSourceComponent_GetFalloff(UUID actorUUID);
		void AudioSourceComponent_SetFalloff(UUID actorUUID, float falloff);
		float AudioSourceComponent_GetMinDistance(UUID actorUUID);
		void AudioSourceComponent_SetMinDistance(UUID actorUUID, float minDistance);
		float AudioSourceComponent_GetMaxDistance(UUID actorUUID);
		void AudioSourceComponent_SetMaxDistance(UUID actorUUID, float maxDistance);
		float AudioSourceComponent_GetPitch(UUID actorUUID);
		void AudioSourceComponent_SetPitch(UUID actorUUID, float pitch);
		float AudioSourceComponent_GetDopplerFactor(UUID actorUUID);
		void AudioSourceComponent_SetDopplerFactor(UUID actorUUID, float dopplerFactor);
		float AudioSourceComponent_GetVolume(UUID actorUUID);
		void AudioSourceComponent_SetVolume(UUID actorUUID, float volume);
		void AudioSourceComponent_GetDirectionToListener(UUID actorUUID, Math::vec3* outDirection);
		bool AudioSourceComponent_GetPlayOnStart(UUID actorUUID);
		void AudioSourceComponent_SetPlayOnStart(UUID actorUUID, bool playOnStart);
		bool AudioSourceComponent_GetIsSpacialized(UUID actorUUID);
		void AudioSourceComponent_SetIsSpacialized(UUID actorUUID, bool spacialized);
		bool AudioSourceComponent_GetIsLooping(UUID actorUUID);
		void AudioSourceComponent_SetIsLooping(UUID actorUUID, bool loop);
		bool AudioSourceComponent_GetIsPlaying(UUID actorUUID);
		bool AudioSourceComponent_GetIsPaused(UUID actorUUID);
		uint64_t AudioSourceComponent_GetCursorInMilliseconds(UUID actorUUID);
		uint32_t AudioSourceComponent_GetPinnedListenerIndex(UUID actorUUID);
		void AudioSourceComponent_SetPinnedListenerIndex(UUID actorUUID, uint32_t listenerIndex);
		void AudioSourceComponent_Play(UUID actorUUID);
		void AudioSourceComponent_SetStartTimeInMilliseconds(UUID actorUUID, uint64_t millis);
		void AudioSourceComponent_SetStartTimeInPCMFrames(UUID actorUUID, uint64_t frames);
		void AudioSourceComponent_SetFadeInMilliseconds(UUID actorUUID, float volumeStart, float volumeEnd, uint64_t lengthInMillis);
		void AudioSourceComponent_SetFadeStartInMilliseconds(UUID actorUUID, float volumeStart, float volumeEnd, uint64_t lengthInMillis, uint64_t absoluteGlobalTime);
		void AudioSourceComponent_SetFadeInPCMFrames(UUID actorUUID, float volumeStart, float volumeEnd, uint64_t lengthInFrames);
		void AudioSourceComponent_SetFadeStartInPCMFrames(UUID actorUUID, float volumeStart, float volumeEnd, uint64_t lengthInFrames, uint64_t absoluteGlobalTime);
		float AudioSourceComponent_GetCurrentFadeVolume(UUID actorUUID);
		void AudioSourceComponent_PlayOneShot(UUID actorUUID);
		void AudioSourceComponent_Pause(UUID actorUUID);
		void AudioSourceComponent_Restart(UUID actorUUID);
		void AudioSourceComponent_Stop(UUID actorUUID);
		void AudioSourceComponent_SetStopTimeInMilliseconds(UUID actorUUID, uint64_t millis);
		void AudioSourceComponent_SetStopTimeInPCMFrames(UUID actorUUID, uint64_t frames);
		void AudioSourceComponent_SetStopTimeWithFadeInMilliseconds(UUID actorUUID, uint64_t stopTimeInMillis, uint64_t fadeLengthInMillis);
		void AudioSourceComponent_SetStopTimeWithFadeInPCMFrames(UUID actorUUID, uint64_t stopTimeInFrames, uint64_t fadeLengthInFrames);
		bool AudioSourceComponent_SeekToPCMFrame(UUID actorUUID, uint64_t frameIndex);

#pragma endregion

#pragma region Audio Clip

		MonoString* AudioClip_GetName(UUID actorUUID);
		float AudioClip_GetLength(UUID actorUUID);

#pragma endregion

#pragma region Audio Cone

		float AudioCone_GetInnerAngle(UUID actorUUID);
		void AudioCone_SetInnerAngle(UUID actorUUID, float innerAngle);
		float AudioCone_GetOuterAngle(UUID actorUUID);
		void AudioCone_SetOuterAngle(UUID actorUUID, float outerAngle);
		float AudioCone_GetOuterGain(UUID actorUUID);
		void AudioCone_SetOuterGain(UUID actorUUID, float outerGain);

#pragma endregion

#pragma region RigidBody Component

		RigidBodyType RigidBodyComponent_GetBodyType(UUID actorUUID);
		void RigidBodyComponent_SetBodyType(UUID actorUUID, RigidBodyType bodyType);
		CollisionDetectionType RigidBodyComponent_GetCollisionDetectionType(UUID actorUUID);
		void RigidBodyComponent_SetCollisionDetectionType(UUID actorUUID, CollisionDetectionType collisionDetectionType);
		float RigidBodyComponent_GetMass(UUID actorUUID);
		void RigidBodyComponent_SetMass(UUID actorUUID, float mass);
		void RigidBodyComponent_GetLinearVelocity(UUID actorUUID, Math::vec3* outVelocity);
		void RigidBodyComponent_SetLinearVelocity(UUID actorUUID, Math::vec3* velocity);
		float RigidBodyComponent_GetMaxLinearVelocity(UUID actorUUID);
		void RigidBodyComponent_SetMaxLinearVelocity(UUID actorUUID, float maxLinearVelocity);
		float RigidBodyComponent_GetLinearDrag(UUID actorUUID);
		void RigidBodyComponent_SetLinearDrag(UUID actorUUID, float drag);
		void RigidBodyComponent_GetAngularVelocity(UUID actorUUID, Math::vec3* outVelocity);
		void RigidBodyComponent_SetAngularVelocity(UUID actorUUID, Math::vec3* velocity);
		float RigidBodyComponent_GetMaxAngularVelocity(UUID actorUUID);
		void RigidBodyComponent_SetMaxAngularVelocity(UUID actorUUID, float maxAngularVelocity);
		float RigidBodyComponent_GetAngularDrag(UUID actorUUID);
		void RigidBodyComponent_SetAngularDrag(UUID actorUUID, float drag);
		bool RigidBodyComponent_GetDisableGravity(UUID actorUUID);
		void RigidBodyComponent_SetDisableGravity(UUID actorUUID, bool disabled);
		bool RigidBodyComponent_GetIsKinematic(UUID actorUUID);
		void RigidBodyComponent_SetIsKinematic(UUID actorUUID, bool isKinematic);
		void RigidBodyComponent_GetKinematicTargetTranslation(UUID actorUUID, Math::vec3* outTranslation);
		void RigidBodyComponent_SetKinematicTargetTranslation(UUID actorUUID, Math::vec3* translation);
		void RigidBodyComponent_GetKinematicTargetRotation(UUID actorUUID, Math::quaternion* outRotation);
		void RigidBodyComponent_SetKinematicTargetRotation(UUID actorUUID, Math::quaternion* rotation);
		uint32_t RigidBodyComponent_GetLockFlags(UUID actorUUID);
		void RigidBodyComponent_SetLockFlag(UUID actorUUID, ActorLockFlag flag, bool value, bool forceWake);
		bool RigidBodyComponent_IsLockFlagSet(UUID actorUUID, ActorLockFlag flag);
		bool RigidBodyComponent_IsSleeping(UUID actorUUID);
		void RigidBodyComponent_WakeUp(UUID actorUUID);
		void RigidBodyComponent_AddForce(UUID actorUUID, Math::vec3* force, ForceMode mode);
		void RigidBodyComponent_AddForceAtPosition(UUID actorUUID, Math::vec3* force, Math::vec3* position, ForceMode mode);
		void RigidBodyComponent_AddTorque(UUID actorUUID, Math::vec3* torque, ForceMode mode);
		void RigidBodyComponent_ClearTorque(UUID actorUUID, ForceMode mode);
		void RigidBodyComponent_ClearForce(UUID actorUUID, ForceMode mode);

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

#pragma region PhysicsMaterial

	float PhysicsMaterial_GetStaticFriction(AssetHandle* assetHandle);
	void PhysicsMaterial_SetStaticFriction(AssetHandle* assetHandle, float staticFriction);
	float PhysicsMaterial_GetDynamicFriction(AssetHandle* assetHandle);
	void PhysicsMaterial_SetDynamicFriction(AssetHandle* assetHandle, float dynamicFriction);
	float PhysicsMaterial_GetBounciness(AssetHandle* assetHandle);
	void PhysicsMaterial_SetBounciness(AssetHandle* assetHandle, float bounciness);
	CombineMode PhysicsMaterial_GetFrictionCombineMode(AssetHandle* assetHandle);
	void PhysicsMaterial_SetFrictionCombineMode(AssetHandle* assetHandle, CombineMode frictionCombineMode);
	CombineMode PhysicsMaterial_GetBouncinessCombineMode(AssetHandle* assetHandle);
	void PhysicsMaterial_SetBouncinessCombineMode(AssetHandle* assetHandle, CombineMode bouncinessCombineMode);

#pragma endregion

#pragma region Character Controller Component

		void CharacterControllerComponent_Move(UUID actorUUID, Math::vec3* displacement);
		void CharacterControllerComponent_Jump(UUID actorUUID, float jumpForce);
		bool CharacterControllerComponent_IsGrounded(UUID actorUUID);
		void CharacterControllerComponent_GetFootPosition(UUID actorUUID, Math::vec3* outFootPos);
		float CharacterControllerComponent_GetSpeedDown(UUID actorUUID);
		float CharacterControllerComponent_GetSlopeLimit(UUID actorUUID);
		void CharacterControllerComponent_SetSlopeLimit(UUID actorUUID, float slopeLimit);
		float CharacterControllerComponent_GetStepOffset(UUID actorUUID);
		void CharacterControllerComponent_SetStepOffset(UUID actorUUID, float stepOffset);
		float CharacterControllerComponent_GetContactOffset(UUID actorUUID);
		void CharacterControllerComponent_SetContactOffset(UUID actorUUID, float contactOffset);
		NonWalkableMode CharacterControllerComponent_GetNonWalkableMode(UUID actorUUID);
		void CharacterControllerComponent_SetNonWalkableMode(UUID actorUUID, NonWalkableMode mode);
		CapsuleClimbMode CharacterControllerComponent_GetClimbMode(UUID actorUUID);
		void CharacterControllerComponent_SetClimbMode(UUID actorUUID, CapsuleClimbMode mode);
		bool CharacterControllerComponent_GetDisableGravity(UUID actorUUID);
		void CharacterControllerComponent_SetDisableGravity(UUID actorUUID, bool disableGravity);
		
#pragma endregion
		
#pragma region FixedJoint Component

		uint64_t FixedJointComponent_GetConnectedActor(UUID actorUUID);
		void FixedJointComponent_SetConnectedActor(UUID actorUUID, UUID connectedActorUUID);
		float FixedJointComponent_GetBreakForce(UUID actorUUID);
		void FixedJointComponent_SetBreakForce(UUID actorUUID, float breakForce);
		float FixedJointComponent_GetBreakTorque(UUID actorUUID);
		void FixedJointComponent_SetBreakTorque(UUID actorUUID, float breakTorque);
		void FixedJointComponent_SetBreakForceAndTorque(UUID actorUUID, float breakForce, float breakTorque);
		bool FixedJointComponent_GetEnableCollision(UUID actorUUID);
		void FixedJointComponent_SetCollisionEnabled(UUID actorUUID, bool enableCollision);
		bool FixedJointComponent_GetPreProcessingEnabled(UUID actorUUID);
		void FixedJointComponent_SetPreProcessingEnabled(UUID actorUUID, bool enablePreProcessing);
		bool FixedJointComponent_IsBroken(UUID actorUUID);
		bool FixedJointComponent_GetIsBreakable(UUID actorUUID);
		void FixedJointComponent_SetIsBreakable(UUID actorUUID, bool isBreakable);
		void FixedJointComponent_Break(UUID actorUUID);

#pragma endregion

#pragma region BoxCollider Component

		void BoxColliderComponent_GetHalfSize(UUID actorUUID, Math::vec3* outHalfSize);
		void BoxColliderComponent_SetHalfSize(UUID actorUUID, Math::vec3* halfSize);
		void BoxColliderComponent_GetOffset(UUID actorUUID, Math::vec3* outOffset);
		void BoxColliderComponent_SetOffset(UUID actorUUID, Math::vec3* offset);
		bool BoxColliderComponent_GetIsTrigger(UUID actorUUID);
		void BoxColliderComponent_SetIsTrigger(UUID actorUUID, bool isTrigger);
		bool BoxColliderComponent_GetMaterialHandle(UUID actorUUID, AssetHandle* outHandle);

#pragma endregion

#pragma region SphereCollider Component

		float SphereColliderComponent_GetRadius(UUID actorUUID);
		void SphereColliderComponent_SetRadius(UUID actorUUID, float radius);
		void SphereColliderComponent_GetOffset(UUID actorUUID, Math::vec3* outOffset);
		void SphereColliderComponent_SetOffset(UUID actorUUID, Math::vec3* offset);
		bool SphereColliderComponent_GetIsTrigger(UUID actorUUID);
		void SphereColliderComponent_SetIsTrigger(UUID actorUUID, bool isTrigger);
		bool SphereColliderComponent_GetMaterialHandle(UUID actorUUID, AssetHandle* outHandle);

#pragma endregion

#pragma region CapsuleCollider Component

		float CapsuleColliderComponent_GetRadius(UUID actorUUID);
		void CapsuleColliderComponent_SetRadius(UUID actorUUID, float radius);
		float CapsuleColliderComponent_GetHeight(UUID actorUUID);
		void CapsuleColliderComponent_SetHeight(UUID actorUUID, float height);
		void CapsuleColliderComponent_GetOffset(UUID actorUUID, Math::vec3* outOffset);
		void CapsuleColliderComponent_SetOffset(UUID actorUUID, Math::vec3* offset);
		bool CapsuleColliderComponent_GetIsTrigger(UUID actorUUID);
		void CapsuleColliderComponent_SetIsTrigger(UUID actorUUID, bool isTrigger);
		bool CapsuleColliderComponent_GetMaterialHandle(UUID actorUUID, AssetHandle* outHandle);

#pragma endregion

#pragma region MeshCollider Component
		
		bool MeshColliderComponent_IsStaticMesh(UUID actorUUID);
		bool MeshColliderComponent_IsColliderMeshValid(UUID actorUUID, AssetHandle* assetHandle);
		bool MeshColliderComponent_GetColliderMesh(UUID actorUUID, AssetHandle* outHandle);
		void MeshColliderComponent_SetColliderMesh(UUID actorUUID, AssetHandle assetHandle);
		bool MeshColliderComponent_GetIsTrigger(UUID actorUUID);
		void MeshColliderComponent_SetIsTrigger(UUID actorUUID, bool isTrigger);
		bool MeshColliderComponent_GetMaterialHandle(UUID actorUUID, AssetHandle* outHandle);

#pragma endregion

#pragma region RigidBody2D Component

		RigidBody2DType RigidBody2DComponent_GetBodyType(UUID actorUUID);
		void RigidBody2DComponent_SetBodyType(UUID actorUUID, RigidBody2DType bodyType);
		void RigidBody2DComponent_GetVelocity(UUID actorUUID, Math::vec2* outVelocity);
		void RigidBody2DComponent_SetVelocity(UUID actorUUID, Math::vec2* velocity);
		float RigidBody2DComponent_GetDrag(UUID actorUUID);
		void RigidBody2DComponent_SetDrag(UUID actorUUID, float drag);
		float RigidBody2DComponent_GetAngularVelocity(UUID actorUUID);
		void RigidBody2DComponent_SetAngularVelocity(UUID actorUUID, float angularVelocity);
		float RigidBody2DComponent_GetAngularDrag(UUID actorUUID);
		void RigidBody2DComponent_SetAngularDrag(UUID actorUUID, float angularDrag);
		bool RigidBody2DComponent_GetFixedRotation(UUID actorUUID);
		void RigidBody2DComponent_SetFixedRotation(UUID actorUUID, bool freeze);
		float RigidBody2DComponent_GetGravityScale(UUID actorUUID);
		void RigidBody2DComponent_SetGravityScale(UUID actorUUID, float gravityScale);
		void RigidBody2DComponent_ApplyForce(UUID actorUUID, Math::vec2* force, Math::vec2* point, bool wake);
		void RigidBody2DComponent_ApplyForceToCenter(UUID actorUUID, Math::vec2* force, bool wake);
		void RigidBody2DComponent_ApplyLinearImpulse(UUID actorUUID, Math::vec2* impulse, Math::vec2* point, bool wake);
		void RigidBody2DComponent_ApplyLinearImpulseToCenter(UUID actorUUID, Math::vec2* impulse, bool wake);

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

		void BoxCollider2DComponent_GetOffset(UUID actorUUID, Math::vec2* outOffset);
		void BoxCollider2DComponent_SetOffset(UUID actorUUID, Math::vec2* offset);
		void BoxCollider2DComponent_GetSize(UUID actorUUID, Math::vec2* outSize);
		void BoxCollider2DComponent_SetSize(UUID actorUUID, Math::vec2* size);
		void BoxCollider2DComponent_GetDensity(UUID actorUUID, float* outDensity);
		void BoxCollider2DComponent_SetDensity(UUID actorUUID, float density);
		void BoxCollider2DComponent_GetFriction(UUID actorUUID, float* outFriction);
		void BoxCollider2DComponent_SetFriction(UUID actorUUID, float friction);
		void BoxCollider2DComponent_GetRestitution(UUID actorUUID, float* outRestitution);
		void BoxCollider2DComponent_SetRestitution(UUID actorUUID, float restitution);
		void BoxCollider2DComponent_GetRestitutionThreshold(UUID actorUUID, float* outRestitutionThreshold);
		void BoxCollider2DComponent_SetRestitutionThreshold(UUID actorUUID, float restitutionThreshold);

#pragma endregion

#pragma region CircleCollider2D Component

		void CircleCollider2DComponent_GetOffset(UUID actorUUID, Math::vec2* outOffset);
		void CircleCollider2DComponent_SetOffset(UUID actorUUID, Math::vec2* offset);
		void CircleCollider2DComponent_GetRadius(UUID actorUUID, float* outRadius);
		void CircleCollider2DComponent_SetRadius(UUID actorUUID, float radius);
		void CircleCollider2DComponent_GetDensity(UUID actorUUID, float* outDensity);
		void CircleCollider2DComponent_SetDensity(UUID actorUUID, float density);
		void CircleCollider2DComponent_GetFriction(UUID actorUUID, float* outFriction);
		void CircleCollider2DComponent_SetFriction(UUID actorUUID, float friction);
		void CircleCollider2DComponent_GetRestitution(UUID actorUUID, float* outRestitution);
		void CircleCollider2DComponent_SetRestitution(UUID actorUUID, float restitution);
		void CircleCollider2DComponent_GetRestitutionThreshold(UUID actorUUID, float* outRestitutionThreshold);
		void CircleCollider2DComponent_SetRestitutionThreshold(UUID actorUUID, float restitutionThreshold);

#pragma endregion

#pragma region Texture2D

		bool Texture2D_LoadFromPath(MonoString* filepath, AssetHandle* outHandle);
		void Texture2D_Constructor(uint32_t width, uint32_t height, AssetHandle* outHandle);
		uint32_t Texture2D_GetWidth(AssetHandle* textureHandle);
		uint32_t Texture2D_GetHeight(AssetHandle* textureHandle);
		void Texture2D_SetPixel(AssetHandle* textureHandle, uint32_t x, uint32_t y, Math::vec4* color);

#pragma endregion

#pragma region Random

		int Random_RangedInt32(int min, int max);
		float Random_RangedFloat(float min, float max);
		float Random_Float();

#pragma endregion

#pragma region Quaternion

		void Quaternion_Inverse(Math::quaternion* quat, Math::quaternion* result);

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

#pragma region PlayerPrefs

		bool PlayerPrefs_HasKey(MonoString* key);
		bool PlayerPrefs_RemoveKey(MonoString* key);
		void PlayerPrefs_WriteInt(MonoString* key, int32_t value);
		int32_t PlayerPrefs_ReadInt(MonoString* key);
		int32_t PlayerPrefs_ReadIntWithDefault(MonoString* key, int32_t defaultValue);
		void PlayerPrefs_WriteString(MonoString* key, MonoString* value);
		MonoString* PlayerPrefs_ReadString(MonoString* key);
		MonoString* PlayerPrefs_ReadStringWithDefault(MonoString* key, MonoString* defaultValue);

#pragma endregion

#pragma region Gui

		void Gui_Begin(MonoString* label);
		void Gui_End();
		void Gui_Underline();
		void Gui_Spacing(unsigned int count);
		void Gui_Text(MonoString* text);
		bool Gui_Button(MonoString* label);
		bool Gui_ButtonWithSize(MonoString* label, const Math::vec2* size);
		void Gui_BeginPropertyGrid();
		void Gui_EndPropertyGrid();
		bool Gui_PropertyGridHeader(MonoString* label, bool defaultOpen);
		void Gui_EndGridHeader();
		bool Gui_PropertyBool(MonoString* label, bool* outValue);
		bool Gui_PropertyInt(MonoString* label, int* outValue);
		bool Gui_PropertyULong(MonoString* label, unsigned int* outValue);
		bool Gui_PropertyFloat(MonoString* label, float* outValue);
		bool Gui_PropertyDouble(MonoString* label, double* outValue);
		bool Gui_PropertyVec2(MonoString* label, Math::vec2* outValue);
		bool Gui_PropertyVec3(MonoString* label, Math::vec3* outValue);
		bool Gui_PropertyVec4(MonoString* label, Math::vec4* outValue);
		bool Gui_PropertyColor3(MonoString* label, Math::vec3* outValue);
		bool Gui_PropertyColor4(MonoString* label, Math::vec4* outValue);

#pragma endregion

		void Log_Message(MonoString* message, Log::LogLevel type);

	}

}
