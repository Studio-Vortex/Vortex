using System;
using System.Runtime.CompilerServices;

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
		internal extern static void DebugRenderer_Flush();

		#endregion

		#region Scene

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Scene_FindEntityByID(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_Instantiate(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Scene_IsPaused();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Scene_Pause();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Scene_Resume();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_GetHoveredEntity();

		#endregion

		#region SceneManager

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SceneManager_LoadScene(string sceneName);

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
		internal extern static Entity[] Entity_GetChildren(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_GetChild(ulong entityID, uint index);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string Entity_GetTag(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string Entity_GetMarker(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_CreateWithName(string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_AddChild(ulong parentEntityID, ulong childEntityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_RemoveChild(ulong parentEntityID, ulong childEntityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static object Entity_GetScriptInstance(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_Destroy(ulong entityID, bool excludeChildren);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetActive(ulong entityID, bool isActive);

		#endregion

		#region Transform

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotation(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslationAndRotation(ulong entityID, ref Vector3 translation, ref Vector3 rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotationQuaternion(ulong entityID, out Quaternion result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotationQuaternion(ulong entityID, ref Quaternion orientation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetWorldSpaceTransform(ulong entityID, out Vector3 translation, out Vector3 rotationEuler, out Vector3 scale);

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
		internal extern static void CameraComponent_GetPrimary(ulong entityID, out bool outPrimary);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetPrimary(ulong entityID, bool primary);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetPerspectiveVerticalFOV(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetPerspectiveVerticalFOV(ulong entityID, float perspectiveVerticalFOV);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetFixedAspectRatio(ulong entityID, out bool outFixedAspectRatio);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFixedAspectRatio(ulong entityID, bool fixedAspectRatio);

		#endregion

		#region LightSource

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

		#endregion

		#region MeshRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static MeshType MeshRendererComponent_GetMeshType(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_SetMeshType(ulong entityID, MeshType meshType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_GetScale(ulong entityID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_SetScale(ulong entityID, ref Vector2 scale);

		#endregion

		#region Material

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_GetAlbedo(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetAlbedo(ulong entityID, ref Vector3 albedo);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetMetallic(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetMetallic(ulong entityID, float metallic);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetRoughness(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetRoughness(ulong entityID, float roughness);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetEmission(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetEmission(ulong entityID, float emission);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Material_GetOpacity(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Material_SetOpacity(ulong entityID, float opacity);

		#endregion

		#region SpriteRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetColor(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetColor(ulong entityID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string SpriteRendererComponent_GetTexture(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetTexture(ulong entityID, string texturePathString);

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
		internal extern static void AudioSourceComponent_Stop(ulong entityID);

		#endregion

		#region RigidBody

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetTranslation(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetTranslation(ulong entityID, ref Vector3 translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetRotation(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetRotation(ulong entityID, ref Vector3 rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_Translate(ulong entityID, ref Vector3 translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_Rotate(ulong entityID, ref Vector3 rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_LookAt(ulong entityID, ref Vector3 worldPoint);

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
		internal extern static float RigidBodyComponent_GetLinearDrag(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetLinearDrag(ulong entityID, float drag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_GetAngularVelocity(ulong entityID, out Vector3 velocity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBodyComponent_SetAngularVelocity(ulong entityID, ref Vector3 velocity);

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

		#region RigidBody2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_GetTranslation(ulong entityID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetTranslation(ulong entityID, ref Vector2 translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RigidBody2DComponent_GetAngle(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetAngle(ulong entityID, float angle);

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
		internal extern static ulong Physics2D_Raycast(ref Vector2 start, ref Vector2 end, out RayCastHit2D hit, bool drawDebugLine);

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

		#region RandomDevice

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static int RandomDevice_RangedInt32(int min, int max);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float RandomDevice_RangedFloat(float min, float max);

		#endregion

		#region Mathf

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Mathf_GetPI();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static double Mathf_GetPI_D();

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

		#endregion

		#region Quaternion

		

		#endregion

		#region Vector3

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Vector3_CrossProductVec3(ref Vector3 left, ref Vector3 right, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Vector3_DotProductVec3(ref Vector3 left, ref Vector3 right);

		#endregion

		#region Time

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetElapsed();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetDeltaTime();

		#endregion

		#region Input

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyUp(KeyCode key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonDown(MouseButton mouseButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonUp(MouseButton mouseButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Input_GetMousePosition(out Vector2 position);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Input_GetMouseScrollOffset(out Vector2 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsGamepadButtonDown(Gamepad gamepadButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsGamepadButtonUp(Gamepad gamepadButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Input_GetGamepadAxis(Gamepad gamepadAxis);

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
		internal extern static void Gui_Separator();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_Spacing();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Gui_Text(string text);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Gui_Button(string text);

		#endregion

		#region Debug

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Debug_Log(string message);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Debug_Info(string message);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Debug_Warn(string message);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Debug_Error(string message);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Debug_Critical(string message);

		#endregion
	}

}
