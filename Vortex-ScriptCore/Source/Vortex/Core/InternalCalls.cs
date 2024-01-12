using System.Runtime.CompilerServices;
using System;

namespace Vortex {

	internal static class InternalCalls
	{
		#region Application

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Application_Quit();

		#endregion

		#region Window

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Window_GetSize(out Vector2 size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Window_GetPosition(out Vector2 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Window_IsMaximized();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Window_SetMaximized(bool maximized);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Window_IsResizeable();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Window_SetResizeable(bool resizable);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Window_IsDecorated();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Window_SetDecorated(bool decorated);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Window_IsVSyncEnabled();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Window_SetVSync(bool use);

		#endregion

		#region SceneRenderer

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
		internal extern static float DebugRenderer_GetLineWidth();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_SetLineWidth(float width);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawLine(ref Vector3 startPoint, ref Vector3 endPoint, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawQuadBillboard(ref Vector3 translation, ref Vector2 size, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawCircleVec2(ref Vector2 translation, ref Vector2 size, ref Vector4 color, float thickness, float fade);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawCircleVec3(ref Vector3 translation, ref Vector3 size, ref Vector4 color, float thickness, float fade);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawBoundingBox(ref Vector3 worldPosition, ref Vector3 size, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawBoundingBoxFromTransform(ulong actorID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_Flush();

		#endregion

		#region Scene

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_GetPrimaryCamera();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Scene_FindActorByID(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_FindActorByTag(string tag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static object Scene_FindActorByType(Type type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_FindChildByTag(ulong parentID, string tag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static object Scene_FindChildByType(ulong parentID, Type type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_CreateActor(string tag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_Instantiate(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_InstantiateAsChild(ulong actorID, ulong parentID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Scene_IsPaused();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Scene_Pause();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Scene_Resume();

		#endregion

		#region SceneManager

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SceneManager_LoadScene(string sceneName);

		#endregion

		#region Component

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Component_IsValid(ulong actorID, Type componentType);

		#endregion

		#region Actor

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Actor_AddComponent(ulong actorID, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Actor_HasComponent(ulong actorID, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Actor_RemoveComponent(ulong actorID, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string Actor_GetTag(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Actor_SetTag(ulong actorID, string value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string Actor_GetMarker(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Actor_SetMarker(ulong actorID, string value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static object Actor_GetScriptInstance(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static Actor[] Actor_GetChildren(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Actor_GetChild(ulong actorID, uint index);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Actor_AddChild(ulong parentActorID, ulong childActorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Actor_RemoveChild(ulong parentActorID, ulong childActorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Actor_Destroy(ulong actorID, bool excludeChildren);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Actor_DestroyWithDelay(ulong actorID, float delay, bool excludeChildren);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Actor_Invoke(ulong actorID, string method);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Actor_InvokeWithDelay(ulong actorID, string method, float delay);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Actor_IsActive(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Actor_SetActive(ulong actorID, bool isActive);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Actor_AddTimer(ulong actorID, string name, float delay);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Actor_IsValid(ulong actorID);

		#endregion

		#region AssetHandle

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AssetHandle_IsValid(ref AssetHandle assetHandle);

		#endregion

		#region Timer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Timer_GetTimeLeft(ulong actorID, string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Timer_IsStarted(ulong actorID, string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Timer_IsFinished(ulong actorID, string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Timer_Start(ulong actorID, string name);

		#endregion

		#region Transform

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong actorID, ref Vector3 translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotation(ulong actorID, out Quaternion result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotation(ulong actorID, ref Quaternion orientation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetEulerAngles(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetEulerAngles(ulong actorID, ref Vector3 eulerAngles);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_Rotate(ulong actorID, ref Vector3 eulers, Space relativeTo);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_RotateAround(ulong actorID, ref Vector3 worldPoint, ref Vector3 axis, float angle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslationAndRotation(ulong actorID, ref Vector3 translation, ref Vector3 eulerAngles);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong actorID, ref Vector3 scale);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetWorldSpaceTransform(ulong actorID, out Vector3 translation, out Quaternion rotation, out Vector3 eulers, out Vector3 scale);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTransformMatrix(ulong actorID, out Matrix4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTransformMatrix(ulong actorID, ref Matrix4 transform);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetForwardDirection(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetBackwardDirection(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetUpDirection(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetDownDirection(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRightDirection(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetLeftDirection(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_LookAt(ulong actorID, ref Vector3 worldPoint);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong TransformComponent_GetParent(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetParent(ulong childID, ulong parentID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_Unparent(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_Multiply(ref Transform a, ref Transform b, out Transform result);

		#endregion

		#region Camera

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ProjectionType CameraComponent_GetProjectionType(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetProjectionType(ulong actorID, ProjectionType type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetPrimary(ulong actorID, out bool outPrimary);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetPrimary(ulong actorID, bool primary);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetPerspectiveVerticalFOV(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetPerspectiveVerticalFOV(ulong actorID, float perspectiveVerticalFOV);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetNearClip(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetNearClip(ulong actorID, float nearClip);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetFarClip(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFarClip(ulong actorID, float farClip);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetOrthographicSize(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetOrthographicSize(ulong actorID, float orthographicSize);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetOrthographicNear(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetOrthographicNear(ulong actorID, float orthographicNear);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetOrthographicFar(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetOrthographicFar(ulong actorID, float orthographicFar);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetFixedAspectRatio(ulong actorID, out bool outFixedAspectRatio);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFixedAspectRatio(ulong actorID, bool fixedAspectRatio);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetClearColor(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetClearColor(ulong actorID, ref Color3 clearColor);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_Raycast(ulong actorID, ref Vector2 position, float maxDistance, out Ray result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_ScreenToWorldPoint(ulong actorID, ref Vector2 position, float maxDistance, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_ScreenToViewportPoint(ulong actorID, ref Vector2 position, out Vector2 result);

		#endregion

		#region PostProcessInfo

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool PostProcessInfo_GetEnabled(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void PostProcessInfo_SetEnabled(ulong actorID, bool enabled);

		#endregion

		#region BloomInfo

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float BloomInfo_GetThreshold(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BloomInfo_SetThreshold(ulong actorID, float threshold);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float BloomInfo_GetKnee(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BloomInfo_SetKnee(ulong actorID, float knee);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float BloomInfo_GetIntensity(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BloomInfo_SetIntensity(ulong actorID, float intensity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool BloomInfo_GetEnabled(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BloomInfo_SetEnabled(ulong actorID, bool enabled);

		#endregion

		#region LightSource

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static LightType LightSourceComponent_GetLightType(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetLightType(ulong actorID, LightType type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_GetRadiance(ulong actorID, out Vector3 outRadiance);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetRadiance(ulong actorID, ref Vector3 radiance);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float LightSourceComponent_GetIntensity(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetIntensity(ulong actorID, float intensity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float LightSourceComponent_GetCutoff(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetCutoff(ulong actorID, float cutoff);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float LightSourceComponent_GetOuterCutoff(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetOuterCutoff(ulong actorID, float outerCutoff);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float LightSourceComponent_GetShadowBias(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetShadowBias(ulong actorID, float shadowBias);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool LightSourceComponent_GetCastShadows(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetCastShadows(ulong actorID, bool castShadows);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool LightSourceComponent_GetSoftShadows(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetSoftShadows(ulong actorID, bool softShadows);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool LightSourceComponent_IsVisible(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetVisible(ulong actorID, bool visible);

		#endregion

		#region TextMesh

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string TextMeshComponent_GetTextString(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetTextString(ulong actorID, string text);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_GetColor(ulong actorID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetColor(ulong actorID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_GetOutlineColor(ulong actorID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetOutlineColor(ulong actorID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float TextMeshComponent_GetLineSpacing(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetLineSpacing(ulong actorID, float lineSpacing);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float TextMeshComponent_GetKerning(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetKerning(ulong actorID, float kerning);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float TextMeshComponent_GetMaxWidth(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetMaxWidth(ulong actorID, float maxWidth);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool TextMeshComponent_IsVisible(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TextMeshComponent_SetVisible(ulong actorID, bool visible);

		#endregion

		#region Animator

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AnimatorComponent_IsPlaying(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AnimatorComponent_Play(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AnimatorComponent_Stop(ulong actorID);

		#endregion

		#region MeshRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool MeshRendererComponent_GetMaterialHandle(uint submeshIndex, ulong actorID, out AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool MeshRendererComponent_IsVisible(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_SetVisible(ulong actorID, bool visible);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool MeshRendererComponent_GetCastShadows(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_SetCastShadows(ulong actorID, bool castShadows);

		#endregion

		#region StaticMeshRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static MeshType StaticMeshRendererComponent_GetMeshType(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void StaticMeshRendererComponent_SetMeshType(ulong actorID, MeshType meshType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void StaticMeshRendererComponent_SetMaterialHandle(uint submeshIndex, ulong actorID, ref AssetHandle materialHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool StaticMeshRendererComponent_IsVisible(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void StaticMeshRendererComponent_SetVisible(ulong actorID, bool visible);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool StaticMeshRendererComponent_GetCastShadows(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void StaticMeshRendererComponent_SetCastShadows(ulong actorID, bool castShadows);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool StaticMeshRendererComponent_GetMaterialHandle(uint submeshIndex, ulong actorID, out AssetHandle assetHandle);

		#endregion

		#region Material

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_GetAlbedo(ref AssetHandle assetHandle, out Vector3 outAlbedo);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetAlbedo(ref AssetHandle assetHandle, ref Vector3 albedo);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetMetallic(ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetMetallic(ref AssetHandle assetHandle, float metallic);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetRoughness(ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetRoughness(ref AssetHandle assetHandle, float roughness);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetEmission(ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetEmission(ref AssetHandle assetHandle, float emission);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_GetUV(ref AssetHandle assetHandle, out Vector2 outUV);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetUV(ref AssetHandle assetHandle, ref Vector2 uv);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetOpacity(ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetOpacity(ref AssetHandle assetHandle, float opacity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Material_HasTexture(ref AssetHandle assetHandle, string textureName);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Material_GetTexture(ref AssetHandle assetHandle, string textureName, out AssetHandle outHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetTexture(ref AssetHandle assetHandle, string textureName, ref AssetHandle textureHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Material_IsFlagSet(ref AssetHandle assetHandle, MaterialFlag flag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetFlag(ref AssetHandle assetHandle, MaterialFlag flag, bool value);

		#endregion

		#region SpriteRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool SpriteRendererComponent_GetTextureHandle(ulong actorID, out AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetTextureHandle(ulong actorID, ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetColor(ulong actorID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetColor(ulong actorID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetUV(ulong actorID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetUV(ulong actorID, ref Vector2 uv);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool SpriteRendererComponent_IsVisible(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetVisible(ulong actorID, bool visible);

		#endregion

		#region CircleRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetColor(ulong actorID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetColor(ulong actorID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetThickness(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetThickness(ulong actorID, float thickness);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetFade(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetFade(ulong actorID, float fade);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool CircleRendererComponent_IsVisible(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetVisible(ulong actorID, bool visible);

		#endregion

		#region ParticleEmitter

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetVelocity(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetVelocity(ulong actorID, ref Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetVelocityVariation(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetVelocityVariation(ulong actorID, ref Vector3 velocityVariation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetOffset(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetOffset(ulong actorID, ref Vector3 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetSizeBegin(ulong actorID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetSizeBegin(ulong actorID, ref Vector2 sizeBegin);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetSizeEnd(ulong actorID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetSizeEnd(ulong actorID, ref Vector2 sizeEnd);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetSizeVariation(ulong actorID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetSizeVariation(ulong actorID, ref Vector2 sizeVariation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetColorBegin(ulong actorID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetColorBegin(ulong actorID, ref Vector4 colorBegin);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetColorEnd(ulong actorID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetColorEnd(ulong actorID, ref Vector4 colorEnd);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetRotation(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetRotation(ulong actorID, float rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_GetLifeTime(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_SetLifeTime(ulong actorID, float lifetime);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_Start(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void ParticleEmitterComponent_Stop(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool ParticleEmitterComponent_IsActive(ulong actorID);

		#endregion

		#region AudioSource

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetPosition(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPosition(ulong actorID, ref Vector3 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetDirection(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetDirection(ulong actorID, ref Vector3 direction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetVelocity(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetVelocity(ulong actorID, ref Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetMinGain(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetMinGain(ulong actorID, float minGain);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetMaxGain(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetMaxGain(ulong actorID, float maxGain);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetDirectionalAttenuationFactor(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetDirectionalAttenuationFactor(ulong actorID, float factor);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static AttenuationMode AudioSourceComponent_GetAttenuationModel(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetAttenuationModel(ulong actorID, AttenuationMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetPan(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPan(ulong actorID, float pan);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static PanMode AudioSourceComponent_GetPanMode(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPanMode(ulong actorID, PanMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static PositioningMode AudioSourceComponent_GetPositioningMode(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPositioningMode(ulong actorID, PositioningMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetFalloff(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetFalloff(ulong actorID, float falloff);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetMinDistance(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetMinDistance(ulong actorID, float minDistance);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetMaxDistance(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetMaxDistance(ulong actorID, float maxDistance);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetPitch(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPitch(ulong actorID, float pitch);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetDopplerFactor(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetDopplerFactor(ulong actorID, float effect);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetVolume(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetVolume(ulong actorID, float volume);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetDirectionToListener(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_GetPlayOnStart(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPlayOnStart(ulong actorID, bool playOnStart);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_GetIsSpacialized(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetIsSpacialized(ulong actorID, bool spacialized);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_GetIsLooping(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetIsLooping(ulong actorID, bool looping);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_GetIsPlaying(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_GetIsPaused(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong AudioSourceComponent_GetCursorInMilliseconds(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static uint AudioSourceComponent_GetPinnedListenerIndex(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPinnedListenerIndex(ulong actorID, uint listenerIndex);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Play(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetStartTimeInMilliseconds(ulong actorID, ulong millis);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetStartTimeInPCMFrames(ulong actorID, ulong frames);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetFadeInMilliseconds(ulong actorID, float volumeStart, float volumeEnd, ulong lengthInMillis);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetFadeStartInMilliseconds(ulong actorID, float volumeStart, float volumeEnd, ulong lengthInMillis, ulong absoluteGlobalTime);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetFadeInPCMFrames(ulong actorID, float volumeStart, float volumeEnd, ulong lengthInFrames);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetFadeStartInPCMFrames(ulong actorID, float volumeStart, float volumeEnd, ulong lengthInFrames, ulong absoluteGlobalTime);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioSourceComponent_GetCurrentFadeVolume(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_PlayOneShot(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Pause(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Restart(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Stop(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetStopTimeInMilliseconds(ulong actorID, ulong millis);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetStopTimeInPCMFrames(ulong actorID, ulong frames);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetStopTimeWithFadeInMilliseconds(ulong actorID, ulong stopTimeInMillis, ulong fadeLengthInMillis);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetStopTimeWithFadeInPCMFrames(ulong actorID, ulong stopTimeInFrames, ulong fadeLengthInFrames);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool AudioSourceComponent_SeekToPCMFrame(ulong actorID, ulong frameIndex);

		#endregion

		#region AudioClip

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string AudioClip_GetName(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioClip_GetLength(ulong actorID);

		#endregion

		#region AudioCone

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioCone_GetInnerAngle(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioCone_SetInnerAngle(ulong actorID, float innerAngle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioCone_GetOuterAngle(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioCone_SetOuterAngle(ulong actorID, float outerAngle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float AudioCone_GetOuterGain(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioCone_SetOuterGain(ulong actorID, float outerGain);

		#endregion

		#region AudioListener

		#endregion

		#region RigidBody

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static RigidBodyType RigidBodyComponent_GetBodyType(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetBodyType(ulong actorID, RigidBodyType bodyType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static CollisionDetectionType RigidBodyComponent_GetCollisionDetectionType(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetCollisionDetectionType(ulong actorID, CollisionDetectionType collisionDetectionType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetMass(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetMass(ulong actorID, float mass);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetLinearVelocity(ulong actorID, out Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetLinearVelocity(ulong actorID, ref Vector3 velocity);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetMaxLinearVelocity(ulong actorID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetMaxLinearVelocity(ulong actorID, float maxLinearVelocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetLinearDrag(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetLinearDrag(ulong actorID, float drag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetAngularVelocity(ulong actorID, out Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetAngularVelocity(ulong actorID, ref Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetMaxAngularVelocity(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetMaxAngularVelocity(ulong actorID, float maxAngularVelocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBodyComponent_GetAngularDrag(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetAngularDrag(ulong actorID, float drag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBodyComponent_GetDisableGravity(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetDisableGravity(ulong actorID, bool disabled);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBodyComponent_GetIsKinematic(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetIsKinematic(ulong actorID, bool isKinematic);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetKinematicTargetTranslation(ulong actorID, out Vector3 result);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetKinematicTargetTranslation(ulong actorID, ref Vector3 target);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetKinematicTargetRotation(ulong actorID, out Quaternion result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetKinematicTargetRotation(ulong actorID, ref Quaternion target);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static uint RigidBodyComponent_GetLockFlags(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetLockFlag(ulong actorID, ActorLockFlag flag, bool value, bool forceWake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBodyComponent_IsLockFlagSet(ulong actorID, ActorLockFlag flag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBodyComponent_IsSleeping(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_WakeUp(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_AddForce(ulong actorID, ref Vector3 force, ForceMode forceMode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_AddForceAtPosition(ulong actorID, ref Vector3 force, ref Vector3 position, ForceMode forceMode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_AddTorque(ulong actorID, ref Vector3 torque, ForceMode forceMode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_ClearTorque(ulong actorID, ForceMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_ClearForce(ulong actorID, ForceMode mode);

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

		#region PhysicsMaterial

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float PhysicsMaterial_GetStaticFriction(ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void PhysicsMaterial_SetStaticFriction(ref AssetHandle assetHandle, float staticFriction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float PhysicsMaterial_GetDynamicFriction(ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void PhysicsMaterial_SetDynamicFriction(ref AssetHandle assetHandle, float dynamicFriction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float PhysicsMaterial_GetBounciness(ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void PhysicsMaterial_SetBounciness(ref AssetHandle assetHandle, float bounciness);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static CombineMode PhysicsMaterial_GetFrictionCombineMode(ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void PhysicsMaterial_SetFrictionCombineMode(ref AssetHandle assetHandle, CombineMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static CombineMode PhysicsMaterial_GetBouncinessCombineMode(ref AssetHandle assetHandle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void PhysicsMaterial_SetBouncinessCombineMode(ref AssetHandle assetHandle, CombineMode mode);

		#endregion

		#region CharacterController

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_Move(ulong actorID, ref Vector3 displacement);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_Jump(ulong actorID, float jumpForce);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool CharacterControllerComponent_IsGrounded(ulong actorID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_GetFootPosition(ulong actorID, out Vector3 footPosition);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float CharacterControllerComponent_GetSpeedDown(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float CharacterControllerComponent_GetSlopeLimit(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetSlopeLimit(ulong actorID, float slopeLimit);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float CharacterControllerComponent_GetStepOffset(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetStepOffset(ulong actorID, float stepOffset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float CharacterControllerComponent_GetContactOffset(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetContactOffset(ulong actorID, float contactOffset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern NonWalkableMode CharacterControllerComponent_GetNonWalkableMode(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetNonWalkableMode(ulong actorID, NonWalkableMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern CapsuleClimbMode CharacterControllerComponent_GetClimbMode(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetClimbMode(ulong actorID, CapsuleClimbMode mode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool CharacterControllerComponent_GetDisableGravity(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CharacterControllerComponent_SetDisableGravity(ulong actorID, bool disableGravity);

		#endregion

		#region FixedJoint

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern ulong FixedJointComponent_GetConnectedActor(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetConnectedActor(ulong actorID, ulong connectedactorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float FixedJointComponent_GetBreakForce(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetBreakForce(ulong actorID, float breakForce);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern float FixedJointComponent_GetBreakTorque(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetBreakTorque(ulong actorID, float breakTorque);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetBreakForceAndTorque(ulong actorID, float breakForce, float breakTorque);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool FixedJointComponent_GetEnableCollision(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetCollisionEnabled(ulong actorID, bool enableCollision);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool FixedJointComponent_GetPreProcessingEnabled(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetPreProcessingEnabled(ulong actorID, bool enablePreProcessing);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool FixedJointComponent_IsBroken(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool FixedJointComponent_GetIsBreakable(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_SetIsBreakable(ulong actorID, bool isBreakable);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void FixedJointComponent_Break(ulong actorID);

		#endregion

		#region BoxCollider

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_GetHalfSize(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_SetHalfSize(ulong actorID, ref Vector3 halfSize);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_GetOffset(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_SetOffset(ulong actorID, ref Vector3 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool BoxColliderComponent_GetIsTrigger(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxColliderComponent_SetIsTrigger(ulong actorID, bool isTrigger);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool BoxColliderComponent_GetMaterialHandle(ulong actorID, out AssetHandle assetHandle);

		#endregion

		#region SphereCollider

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float SphereColliderComponent_GetRadius(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SphereColliderComponent_SetRadius(ulong actorID, float radius);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SphereColliderComponent_GetOffset(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SphereColliderComponent_SetOffset(ulong actorID, ref Vector3 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool SphereColliderComponent_GetIsTrigger(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SphereColliderComponent_SetIsTrigger(ulong actorID, bool isTrigger);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool SphereColliderComponent_GetMaterialHandle(ulong actorID, out AssetHandle assetHandle);

		#endregion

		#region CapsuleCollider

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CapsuleColliderComponent_GetRadius(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_SetRadius(ulong actorID, float radius);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CapsuleColliderComponent_GetHeight(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_SetHeight(ulong actorID, float height);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_GetOffset(ulong actorID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_SetOffset(ulong actorID, ref Vector3 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool CapsuleColliderComponent_GetIsTrigger(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CapsuleColliderComponent_SetIsTrigger(ulong actorID, bool isTrigger);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool CapsuleColliderComponent_GetMaterialHandle(ulong actorID, out AssetHandle assetHandle);

		#endregion

		#region MeshCollider

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool MeshColliderComponent_IsStaticMesh(ulong actorID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool MeshColliderComponent_IsColliderMeshValid(ulong actorID, ref AssetHandle assetHandle);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool MeshColliderComponent_GetColliderMesh(ulong actorID, out AssetHandle assetHandle);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshColliderComponent_SetColliderMesh(ulong actorID, ref AssetHandle assetHandle);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool MeshColliderComponent_GetIsTrigger(ulong actorID);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshColliderComponent_SetIsTrigger(ulong actorID, bool isTrigger);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool MeshColliderComponent_GetMaterialHandle(ulong actorID, out AssetHandle assetHandle);

		#endregion

		#region RigidBody2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static RigidBody2DType RigidBody2DComponent_GetBodyType(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetBodyType(ulong actorID, RigidBody2DType bodyType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyForce(ulong actorID, ref Vector2 force, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyForceToCenter(ulong actorID, ref Vector2 force, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyLinearImpulse(ulong actorID, ref Vector2 impulse, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyLinearImpulseToCenter(ulong actorID, ref Vector2 impulse, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_GetVelocity(ulong actorID, out Vector2 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetVelocity(ulong actorID, ref Vector2 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBody2DComponent_GetDrag(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetDrag(ulong actorID, float drag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBody2DComponent_GetAngularVelocity(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetAngularVelocity(ulong actorID, float angularVelocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBody2DComponent_GetAngularDrag(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetAngularDrag(ulong actorID, float angularDrag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBody2DComponent_GetFixedRotation(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetFixedRotation(ulong actorID, bool freeze);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBody2DComponent_GetGravityScale(ulong actorID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetGravityScale(ulong actorID, float gravityScale);

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
		internal extern static void BoxCollider2DComponent_GetOffset(ulong actorID, out Vector2 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetOffset(ulong actorID, ref Vector2 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetSize(ulong actorID, out Vector2 size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetSize(ulong actorID, ref Vector2 size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetDensity(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetDensity(ulong actorID, float density);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetFriction(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetFriction(ulong actorID, float friction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetRestitution(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetRestitution(ulong actorID, float restitution);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetRestitutionThreshold(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetRestitutionThreshold(ulong actorID, float restitutionThreshold);

		#endregion

		#region CircleCollider2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetOffset(ulong actorID, out Vector2 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetOffset(ulong actorID, ref Vector2 offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetRadius(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetRadius(ulong actorID, float radius);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetDensity(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetDensity(ulong actorID, float density);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetFriction(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetFriction(ulong actorID, float friction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetRestitution(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetRestitution(ulong actorID, float restitution);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_GetRestitutionThreshold(ulong actorID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2DComponent_SetRestitutionThreshold(ulong actorID, float restitutionThreshold);

		#endregion

		#region Texture2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Texture2D_LoadFromPath(string filepath, out AssetHandle assetHandle);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Texture2D_Constructor(uint width, uint height, out AssetHandle assetHandle);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static uint Texture2D_GetWidth(ref AssetHandle assetHandle);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static uint Texture2D_GetHeight(ref AssetHandle assetHandle);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Texture2D_SetPixel(ref AssetHandle assetHandle, uint xOffset, uint yOffset, ref Color4 color);

		#endregion

		#region Random

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static int Random_RangedInt32(int min, int max);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Random_RangedFloat(float min, float max);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Random_Float();

		#endregion

		#region Quaternion

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Quaternion_Inverse(ref Quaternion quat, out Quaternion result);

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
		internal extern static void Mathf_EulerAngles(ref Quaternion orientation, out Vector3 eulers);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Mathf_LookAt(ref Vector3 eyePos, ref Vector3 worldPoint, out Quaternion result);

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

		#region PlayerPrefs

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool PlayerPrefs_HasKey(string key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool PlayerPrefs_RemoveKey(string key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void PlayerPrefs_WriteInt(string key, int value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static int PlayerPrefs_ReadInt(string key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static int PlayerPrefs_ReadIntWithDefault(string key, int defaultValue);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void PlayerPrefs_WriteString(string key, string value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string PlayerPrefs_ReadString(string key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string PlayerPrefs_ReadStringWithDefault(string key, string defaultValue);

		#endregion

		#region Gui

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_Begin(string name);

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
		internal extern static bool Gui_ButtonWithSize(string text, ref Vector2 size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_BeginPropertyGrid();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_EndPropertyGrid();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyGridHeader(string label, bool defaultOpen);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_EndGridHeader();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyBool(string label, ref bool value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyInt(string label, ref int value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyULong(string label, ref ulong value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyFloat(string label, ref float value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyDouble(string label, ref double value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyVec2(string label, ref Vector2 value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyVec3(string label, ref Vector3 value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyVec4(string label, ref Vector4 value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyColor3(string label, ref Color3 value);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_PropertyColor4(string label, ref Color4 value);

		#endregion

		#region Log

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Log_Message(string message, LogType type);

		#endregion
	}

}
