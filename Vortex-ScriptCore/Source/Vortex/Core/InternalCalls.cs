using System.Runtime.CompilerServices;
using System;

namespace Vortex {

	internal static class InternalCalls
	{
		#region Application

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Application_Quit();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Application_GetSize(out Vector2 size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Application_GetPosition(out Vector2 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Application_IsMaximized();

		#endregion

		#region SceneRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float SceneRenderer_GetBloomThreshold();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SceneRenderer_SetBloomThreshold(float threshold);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float SceneRenderer_GetBloomSoftKnee();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SceneRenderer_SetBloomSoftKnee(float softKnee);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float SceneRenderer_GetBloomUnknown();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SceneRenderer_SetBloomUnknown(float unknown);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float SceneRenderer_GetExposure();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SceneRenderer_SetExposure(float exposure);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float SceneRenderer_GetGamma();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SceneRenderer_SetGamma(float gamma);

		#endregion

		#region DebugRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_SetClearColor(ref Vector3 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_BeginScene();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawLine(ref Vector3 p1, ref Vector3 p2, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawQuadBillboard(ref Vector3 translation, ref Vector2 size, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawCircleVec2(ref Vector2 translation, ref Vector2 size, ref Vector4 color, float thickness, float fade);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawCircleVec3(ref Vector3 translation, ref Vector3 size, ref Vector4 color, float thickness, float fade);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawBoundingBox(ref Vector3 worldPosition, ref Vector3 size, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawBoundingBoxFromTransform(ulong entityID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_Flush();

		#endregion

		#region Scene

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Scene_FindEntityByID(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_FindEntityByName(string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_FindChildByName(ulong entityID, string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_CreateEntity(string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_Instantiate(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_InstantiateAsChild(ulong entityID, ulong parentID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Scene_IsPaused();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Scene_Pause();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Scene_Resume();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_GetHoveredEntity();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static uint Scene_GetCurrentBuildIndex();

		#endregion

		#region SceneManager

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SceneManager_LoadScene(string sceneName);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SceneManager_LoadSceneFromBuildIndex(uint buildIndex);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string SceneManager_GetActiveScene();

		#endregion

		#region Entity

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddComponent(ulong entityID, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveComponent(ulong entityID, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindChildByName(ulong entityID, string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static Entity[] Entity_GetChildren(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_GetChild(ulong entityID, uint index);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string Entity_GetTag(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string Entity_GetMarker(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetMarker(ulong entityID, string value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_AddChild(ulong parentEntityID, ulong childEntityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_RemoveChild(ulong parentEntityID, ulong childEntityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static object Entity_GetScriptInstance(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_Destroy(ulong entityID, bool excludeChildren);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_DestroyTimed(ulong entityID, float waitTime, bool excludeChildren);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetActive(ulong entityID, bool isActive);

		#endregion

		#region Transform

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotation(ulong entityID, out Quaternion result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotation(ulong entityID, ref Quaternion orientation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetEulerAngles(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetEulerAngles(ulong entityID, ref Vector3 eulerAngles);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_Rotate(ulong entityID, ref Vector3 eulers, Space relativeTo);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_RotateAround(ulong entityID, ref Vector3 worldPoint, ref Vector3 axis, float angle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslationAndRotation(ulong entityID, ref Vector3 translation, ref Vector3 eulerAngles);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetWorldSpaceTransform(ulong entityID, out Vector3 translation, out Quaternion rotation, out Vector3 eulers, out Vector3 scale);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTransformMatrix(ulong entityID, out Matrix4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTransformMatrix(ulong entityID, ref Matrix4 transform);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetForwardDirection(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetUpDirection(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRightDirection(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_LookAt(ulong entityID, ref Vector3 worldPoint);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong TransformComponent_GetParent(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetParent(ulong childID, ulong parentID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_Unparent(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_Multiply(ref Transform a, ref Transform b, out Transform result);

		#endregion

		#region Camera

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ProjectionType CameraComponent_GetProjectionType(ulong entityID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetProjectionType(ulong entityID, ProjectionType type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetPrimary(ulong entityID, out bool outPrimary);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetPrimary(ulong entityID, bool primary);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetPerspectiveVerticalFOV(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetPerspectiveVerticalFOV(ulong entityID, float perspectiveVerticalFOV);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetNearClip(ulong entityID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetNearClip(ulong entityID, float nearClip);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetFarClip(ulong entityID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFarClip(ulong entityID, float farClip);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetOrthographicSize(ulong entityID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetOrthographicSize(ulong entityID, float orthographicSize);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetOrthographicNear(ulong entityID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetOrthographicNear(ulong entityID, float orthographicNear);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetOrthographicFar(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetOrthographicFar(ulong entityID, float orthographicFar);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetFixedAspectRatio(ulong entityID, out bool outFixedAspectRatio);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFixedAspectRatio(ulong entityID, bool fixedAspectRatio);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetClearColor(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetClearColor(ulong entityID, ref Color3 clearColor);

		#endregion

		#region LightSource

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static LightType LightSourceComponent_GetLightType(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetLightType(ulong entityID, LightType type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_GetRadiance(ulong entityID, out Vector3 outRadiance);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetRadiance(ulong entityID, ref Vector3 radiance);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float LightSourceComponent_GetIntensity(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetIntensity(ulong entityID, float intensity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float LightSourceComponent_GetCutoff(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetCutoff(ulong entityID, float cutoff);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float LightSourceComponent_GetOuterCutoff(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetOuterCutoff(ulong entityID, float outerCutoff);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float LightSourceComponent_GetShadowBias(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetShadowBias(ulong entityID, float shadowBias);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool LightSourceComponent_GetCastShadows(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetCastShadows(ulong entityID, bool castShadows);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool LightSourceComponent_GetSoftShadows(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetSoftShadows(ulong entityID, bool softShadows);

		#endregion

		#region TextMesh

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string TextMeshComponent_GetTextString(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetTextString(ulong entityID, string text);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_GetColor(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetColor(ulong entityID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_GetBackgroundColor(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetBackgroundColor(ulong entityID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float TextMeshComponent_GetLineSpacing(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetLineSpacing(ulong entityID, float lineSpacing);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float TextMeshComponent_GetKerning(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetKerning(ulong entityID, float kerning);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float TextMeshComponent_GetMaxWidth(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetMaxWidth(ulong entityID, float maxWidth);

		#endregion

		#region Animator

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AnimatorComponent_IsPlaying(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AnimatorComponent_Play(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AnimatorComponent_Stop(ulong entityID);

		#endregion

		#region MeshRenderer

		#endregion

		#region StaticMeshRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static MeshType StaticMeshRendererComponent_GetMeshType(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void StaticMeshRendererComponent_SetMeshType(ulong entityID, MeshType meshType);

		#endregion

		#region Material

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_GetAlbedo(ulong entityID, uint submeshIndex, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetAlbedo(ulong entityID, uint submeshIndex, ref Vector3 albedo);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetMetallic(ulong entityID, uint submeshIndex);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetMetallic(ulong entityID, uint submeshIndex, float metallic);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetRoughness(ulong entityID, uint submeshIndex);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetRoughness(ulong entityID, uint submeshIndex, float roughness);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetEmission(ulong entityID, uint submeshIndex);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetEmission(ulong entityID, uint submeshIndex, float emission);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_GetUV(ulong entityID, uint submeshIndex, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetUV(ulong entityID, uint submeshIndex, ref Vector2 uv);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetOpacity(ulong entityID, uint submeshIndex);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetOpacity(ulong entityID, uint submeshIndex, float opacity);

		#endregion

		#region SpriteRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetColor(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetColor(ulong entityID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static IntPtr SpriteRendererComponent_GetTexture(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetTexture(ulong entityID, IntPtr unmanagedInstance);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetScale(ulong entityID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetScale(ulong entityID, ref Vector2 scale);

		#endregion

		#region CircleRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetColor(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetColor(ulong entityID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetThickness(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetThickness(ulong entityID, float thickness);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetFade(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetFade(ulong entityID, float fade);

		#endregion

		#region ParticleEmitter

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetVelocity(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetVelocity(ulong entityID, ref Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetVelocityVariation(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetVelocityVariation(ulong entityID, ref Vector3 velocityVariation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetOffset(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetOffset(ulong entityID, ref Vector3 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetSizeBegin(ulong entityID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetSizeBegin(ulong entityID, ref Vector2 sizeBegin);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetSizeEnd(ulong entityID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetSizeEnd(ulong entityID, ref Vector2 sizeEnd);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetSizeVariation(ulong entityID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetSizeVariation(ulong entityID, ref Vector2 sizeVariation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetColorBegin(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetColorBegin(ulong entityID, ref Vector4 colorBegin);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetColorEnd(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetColorEnd(ulong entityID, ref Vector4 colorEnd);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetRotation(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetRotation(ulong entityID, float rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetLifeTime(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetLifeTime(ulong entityID, float lifetime);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_Start(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_Stop(ulong entityID);

		#endregion

		#region AudioSource

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_GetIsPlaying(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Play(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_PlayOneShot(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Restart(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Stop(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetPosition(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPosition(ulong entityID, ref Vector3 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetDirection(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetDirection(ulong entityID, ref Vector3 direction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetVelocity(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetVelocity(ulong entityID, ref Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetConeInnerAngle(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetConeInnerAngle(ulong entityID, float innerAngle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetConeOuterAngle(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetConeOuterAngle(ulong entityID, float outerAngle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetConeOuterGain(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetConeOuterGain(ulong entityID, float outerGain);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetMinDistance(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetMinDistance(ulong entityID, float minDistance);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetMaxDistance(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetMaxDistance(ulong entityID, float maxDistance);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetPitch(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPitch(ulong entityID, float pitch);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetDopplerFactor(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetDopplerFactor(ulong entityID, float effect);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetVolume(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetVolume(ulong entityID, float volume);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_GetPlayOnStart(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPlayOnStart(ulong entityID, bool playOnStart);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_GetIsSpacialized(ulong entityID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetIsSpacialized(ulong entityID, bool spacialized);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_GetIsLooping(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetIsLooping(ulong entityID, bool looping);

		#endregion

		#region AudioClip

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string AudioClip_GetName(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioClip_GetLength(ulong entityID);

		#endregion

		#region RigidBody

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static RigidBodyType RigidBodyComponent_GetBodyType(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetBodyType(ulong entityID, RigidBodyType bodyType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static CollisionDetectionType RigidBodyComponent_GetCollisionDetectionType(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetCollisionDetectionType(ulong entityID, CollisionDetectionType collisionDetectionType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetMass(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetMass(ulong entityID, float mass);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetLinearVelocity(ulong entityID, out Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetLinearVelocity(ulong entityID, ref Vector3 velocity);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetMaxLinearVelocity(ulong entityID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetMaxLinearVelocity(ulong entityID, float maxLinearVelocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetLinearDrag(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetLinearDrag(ulong entityID, float drag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetAngularVelocity(ulong entityID, out Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetAngularVelocity(ulong entityID, ref Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetMaxAngularVelocity(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetMaxAngularVelocity(ulong entityID, float maxAngularVelocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetAngularDrag(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetAngularDrag(ulong entityID, float drag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBodyComponent_GetDisableGravity(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetDisableGravity(ulong entityID, bool disabled);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBodyComponent_GetIsKinematic(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetIsKinematic(ulong entityID, bool isKinematic);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetKinematicTargetTranslation(ulong entityID, out Vector3 result);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetKinematicTargetTranslation(ulong entityID, ref Vector3 target);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetKinematicTargetRotation(ulong entityID, out Quaternion result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetKinematicTargetRotation(ulong entityID, ref Quaternion target);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static uint RigidBodyComponent_GetLockFlags(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetLockFlag(ulong entityID, ActorLockFlag flag, bool value, bool forceWake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBodyComponent_IsLockFlagSet(ulong entityID, ActorLockFlag flag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBodyComponent_IsSleeping(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_WakeUp(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_AddForce(ulong entityID, ref Vector3 force, ForceMode forceMode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_AddForceAtPosition(ulong entityID, ref Vector3 force, ref Vector3 position, ForceMode forceMode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_AddTorque(ulong entityID, ref Vector3 torque, ForceMode forceMode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_ClearTorque(ulong entityID, ForceMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_ClearForce(ulong entityID, ForceMode mode);

		#endregion

		#region Physics

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool Physics_Raycast(ref Vector3 origin, ref Vector3 direction, float maxDistance, out RaycastHit hit);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Physics_GetSceneGravity(out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Physics_SetSceneGravity(ref Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern uint Physics_GetScenePositionIterations();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Physics_SetScenePositionIterations(uint positionIterations);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern uint Physics_GetSceneVelocityIterations();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Physics_SetSceneVelocityIterations(uint velocityIterations);

		#endregion

		#region CharacterController

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_Move(ulong entityID, ref Vector3 displacement);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_Jump(ulong entityID, float jumpForce);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool CharacterControllerComponent_IsGrounded(ulong entityID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_GetFootPosition(ulong entityID, out Vector3 footPosition);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float CharacterControllerComponent_GetSpeedDown(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float CharacterControllerComponent_GetSlopeLimit(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetSlopeLimit(ulong entityID, float slopeLimit);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float CharacterControllerComponent_GetStepOffset(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetStepOffset(ulong entityID, float stepOffset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float CharacterControllerComponent_GetContactOffset(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetContactOffset(ulong entityID, float contactOffset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern NonWalkableMode CharacterControllerComponent_GetNonWalkableMode(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetNonWalkableMode(ulong entityID, NonWalkableMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern CapsuleClimbMode CharacterControllerComponent_GetClimbMode(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetClimbMode(ulong entityID, CapsuleClimbMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool CharacterControllerComponent_GetDisableGravity(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetDisableGravity(ulong entityID, bool disableGravity);

		#endregion

		#region FixedJoint

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern ulong FixedJointComponent_GetConnectedEntity(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetConnectedEntity(ulong entityID, ulong connectedEntityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float FixedJointComponent_GetBreakForce(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetBreakForce(ulong entityID, float breakForce);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float FixedJointComponent_GetBreakTorque(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetBreakTorque(ulong entityID, float breakTorque);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetBreakForceAndTorque(ulong entityID, float breakForce, float breakTorque);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool FixedJointComponent_GetEnableCollision(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetCollisionEnabled(ulong entityID, bool enableCollision);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool FixedJointComponent_GetPreProcessingEnabled(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetPreProcessingEnabled(ulong entityID, bool enablePreProcessing);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool FixedJointComponent_IsBroken(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool FixedJointComponent_GetIsBreakable(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetIsBreakable(ulong entityID, bool isBreakable);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_Break(ulong entityID);

		#endregion

		#region PhysicsMaterial



		#endregion

		#region BoxCollider

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_GetHalfSize(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_SetHalfSize(ulong entityID, ref Vector3 halfSize);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_GetOffset(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_SetOffset(ulong entityID, ref Vector3 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool BoxColliderComponent_GetIsTrigger(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_SetIsTrigger(ulong entityID, bool isTrigger);

		#endregion

		#region SphereCollider

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float SphereColliderComponent_GetRadius(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SphereColliderComponent_SetRadius(ulong entityID, float radius);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SphereColliderComponent_GetOffset(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SphereColliderComponent_SetOffset(ulong entityID, ref Vector3 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool SphereColliderComponent_GetIsTrigger(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SphereColliderComponent_SetIsTrigger(ulong entityID, bool isTrigger);

		#endregion

		#region CapsuleCollider

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CapsuleColliderComponent_GetRadius(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_SetRadius(ulong entityID, float radius);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CapsuleColliderComponent_GetHeight(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_SetHeight(ulong entityID, float height);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_GetOffset(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_SetOffset(ulong entityID, ref Vector3 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool CapsuleColliderComponent_GetIsTrigger(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_SetIsTrigger(ulong entityID, bool isTrigger);

		#endregion

		#region MeshCollider

		#endregion

		#region RigidBody2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static RigidBody2DType RigidBody2DComponent_GetBodyType(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetBodyType(ulong entityID, RigidBody2DType bodyType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyForce(ulong entityID, ref Vector2 force, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyForceToCenter(ulong entityID, ref Vector2 force, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_GetVelocity(ulong entityID, out Vector2 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetVelocity(ulong entityID, ref Vector2 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBody2DComponent_GetDrag(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetDrag(ulong entityID, float drag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBody2DComponent_GetAngularVelocity(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetAngularVelocity(ulong entityID, float angularVelocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBody2DComponent_GetAngularDrag(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetAngularDrag(ulong entityID, float angularDrag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBody2DComponent_GetFixedRotation(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetFixedRotation(ulong entityID, bool freeze);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBody2DComponent_GetGravityScale(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetGravityScale(ulong entityID, float gravityScale);

		#endregion

		#region Physics2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Physics2D_Raycast(ref Vector2 start, ref Vector2 end, out RaycastHit2D hit, bool drawDebugLine);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Physics2D_GetWorldGravity(out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Physics2D_SetWorldGravity(ref Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern uint Physics2D_GetWorldPositionIterations();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Physics2D_SetWorldPositionIterations(uint positionIterations);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern uint Physics2D_GetWorldVelocityIterations();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Physics2D_SetWorldVelocityIterations(uint velocityIterations);

		#endregion

		#region BoxCollider2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetOffset(ulong entityID, out Vector2 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetOffset(ulong entityID, ref Vector2 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetSize(ulong entityID, out Vector2 size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetSize(ulong entityID, ref Vector2 size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetDensity(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetDensity(ulong entityID, float density);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetFriction(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetFriction(ulong entityID, float friction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetRestitution(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetRestitution(ulong entityID, float restitution);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetRestitutionThreshold(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetRestitutionThreshold(ulong entityID, float restitutionThreshold);

		#endregion

		#region CircleCollider2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetOffset(ulong entityID, out Vector2 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetOffset(ulong entityID, ref Vector2 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetRadius(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetRadius(ulong entityID, float radius);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetDensity(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetDensity(ulong entityID, float density);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetFriction(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetFriction(ulong entityID, float friction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetRestitution(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetRestitution(ulong entityID, float restitution);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetRestitutionThreshold(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetRestitutionThreshold(ulong entityID, float restitutionThreshold);

		#endregion

		#region Texture2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static IntPtr Texture2D_LoadFromPath(string filepath);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static IntPtr Texture2D_Constructor(uint width, uint height);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static uint Texture2D_GetWidth(IntPtr unmanagedInstance);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static uint Texture2D_GetHeight(IntPtr unmanagedInstance);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Texture2D_SetPixel(IntPtr unmanagedInstance, uint xOffset, uint yOffset, ref Color4 color);

		#endregion

		#region Random

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static int Random_RangedInt32(int min, int max);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Random_RangedFloat(float min, float max);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Random_Float();

		#endregion

		#region Matrix4

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Matrix4_Rotate(float angleDeg, ref Vector3 axis, out Matrix4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Matrix4_LookAt(ref Vector3 eyePos, ref Vector3 worldPoint, ref Vector3 up, out Matrix4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Matrix4_Multiply(ref Matrix4 matrix, ref Matrix4 other, out Matrix4 result);

		#endregion

		#region Mathf

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_GetPI();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static double Mathf_GetPI_D();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Round(float value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Abs(float value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Sqrt(float value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Sin(float value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Cos(float value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Acos(float value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Tan(float value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Max(float x, float y);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Min(float x, float y);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Deg2Rad(float degrees);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_Rad2Deg(float radians);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Mathf_Deg2RadVector3(ref Vector3 value, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Mathf_Rad2DegVector3(ref Vector3 value, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Mathf_LookAt(ref Vector3 eyePos, ref Vector3 worldPoint, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Mathf_InverseQuat(ref Quaternion value, out Quaternion result);

		#endregion

		#region Noise

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static IntPtr Noise_Constructor(int seed, NoiseType type);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Noise_Destructor(IntPtr unmanagedInstance);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Noise_GetFrequency(IntPtr unmanagedInstance);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Noise_SetFrequency(IntPtr unmanagedInstance, float freqeuncy);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static int Noise_GetFractalOctaves(IntPtr unmanagedInstance);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Noise_SetFractalOctaves(IntPtr unmanagedInstance, int octaves);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Noise_GetFractalLacunarity(IntPtr unmanagedInstance);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Noise_SetFractalLacunarity(IntPtr unmanagedInstance, float lacunarity);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Noise_GetFractalGain(IntPtr unmanagedInstance);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Noise_SetFractalGain(IntPtr unmanagedInstance, float gain);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Noise_GetVec2(IntPtr unmanagedInstance, float x, float y);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Noise_GetVec3(IntPtr unmanagedInstance, float x, float y, float z);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Noise_SetSeed(int seed);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Noise_PerlinNoiseVec2(float x, float y);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Noise_PerlinNoiseVec3(float x, float y, float z);

		#endregion

		#region Time

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetElapsed();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetDeltaTime();

		#endregion

		#region Input

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyPressed(KeyCode key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyReleased(KeyCode key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyUp(KeyCode key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonPressed(MouseButton mouseButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonReleased(MouseButton mouseButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonDown(MouseButton mouseButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonUp(MouseButton mouseButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Input_GetMousePosition(out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Input_SetMousePosition(ref Vector2 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Input_GetMouseWheelMovement(out Vector2 movement);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsGamepadButtonDown(GamepadButton gamepadButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsGamepadButtonUp(GamepadButton gamepadButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Input_GetGamepadAxis(GamepadAxis gamepadAxis);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static CursorMode Input_GetCursorMode();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Input_SetCursorMode(CursorMode cursorMode);

		#endregion

		#region Gui

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_Begin(string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_BeginWithPosition(string name, ref Vector2 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_BeginWithSize(string name, float width, float height);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_BeginWithPositionAndSize(string name, ref Vector2 position, ref Vector2 size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_End();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_Underline();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_Spacing(ulong count);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_Text(string text);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_Button(string text);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyBool(string label, out bool value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyInt(string label, out int value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyULong(string label, out ulong value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyFloat(string label, out float value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyDouble(string label, out double value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyVec2(string label, out Vector2 value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyVec3(string label, out Vector3 value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyVec4(string label, out Vector4 value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyColor3(string label, out Color3 value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyColor4(string label, out Color4 value);

		#endregion

		#region Log

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Log_Message(string message, LogType type);

		#endregion
	}

}
