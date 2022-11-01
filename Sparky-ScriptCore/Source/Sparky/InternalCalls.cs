using System;
using System.Runtime.CompilerServices;

namespace Sparky {

	internal static class InternalCalls
	{
		#region Application

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Application_Shutdown();

		#endregion

		#region Window

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Window_GetSize(out Vector2 size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Window_GetPosition(out Vector2 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Window_IsMaximized();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Window_ShowMouseCursor(bool enabled);

		#endregion

		#region Renderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Renderer_SetClearColor(ref Vector3 color);

		#endregion

		#region Scene

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

		#region Entity

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddCamera(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveCamera(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddLightSource(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveLightSource(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddMeshRenderer(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveMeshRenderer(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddSpriteRenderer(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveSpriteRenderer(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddCircleRenderer(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveCircleRenderer(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddParticleEmitter(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveParticleEmitter(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddAudioSource(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveAudioSource(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddAudioListener(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveAudioListener(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddRigidBody2D(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveRigidBody2D(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddBoxCollider2D(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveBoxCollider2D(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddCircleCollider2D(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveCircleCollider2D(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_CreateWithName(string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_GetName(ulong entityID, out string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static object Entity_GetScriptInstance(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_Destroy(ulong entityID, bool isScriptInstance = true);

		#endregion

		#region Transform

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotation(ulong entityID, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);

		#endregion

		#region Camera

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetPrimary(ulong entityID, out bool outPrimary);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetPrimary(ulong entityID, bool primary);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetFixedAspectRatio(ulong entityID, out bool outFixedAspectRatio);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFixedAspectRatio(ulong entityID, bool fixedAspectRatio);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_LookAt(ulong entityID, ref Vector3 point, ref Vector3 up);

		#endregion

		#region LightSource

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_GetAmbient(ulong entityID, out Vector3 outAmbient);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetAmbient(ulong entityID, ref Vector3 ambient);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_GetDiffuse(ulong entityID, out Vector3 outDiffuse);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetDiffuse(ulong entityID, ref Vector3 diffuse);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_GetSpecular(ulong entityID, out Vector3 outSpecular);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetSpecular(ulong entityID, ref Vector3 specular);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_GetColor(ulong entityID, out Vector3 outColor);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void LightSourceComponent_SetColor(ulong entityID, ref Vector3 color);

		#endregion

		#region MeshRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_GetColor(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_SetColor(ulong entityID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_GetTexture(ulong entityID, out string texturePathString);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_SetTexture(ulong entityID, string texturePathString);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_GetScale(ulong entityID, out Vector2 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void MeshRendererComponent_SetScale(ulong entityID, ref Vector2 scale);

		#endregion

		#region SpriteRenderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetColor(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetColor(ulong entityID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetTexture(ulong entityID, out string texturePathString);

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
		internal extern static void AudioSourceComponent_Play(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Stop(ulong entityID);

		#endregion

		#region RigidBody2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_GetBodyType(ulong entityID, out RigidBody2DType bodyType);

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

		#endregion

		#region Physics2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Physics2D_Raycast(ref Vector2 start, ref Vector2 end, out RayCastHit2D hit, bool drawDebugLine);

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

		#region Algebra

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static Vector3 Algebra_CrossProductVec3(ref Vector3 left, ref Vector3 right);

		#endregion

		#region Input

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyUp(KeyCode key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Input_GetMousePosition(out Vector2 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsGamepadButtonDown(Gamepad gamepadButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsGamepadButtonUp(Gamepad gamepadButton);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float Input_GetGamepadAxis(Gamepad gamepadAxis);

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
