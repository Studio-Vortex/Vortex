using System;
using System.Runtime.CompilerServices;

namespace Sparky {

	internal static class InternalCalls
	{

		#region Entity

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static object Entity_GetScriptInstance(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_Destroy(ulong entityID);

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

		#region Sprite

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteComponent_GetColor(ulong entityID, out Vector4 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteComponent_SetColor(ulong entityID, ref Vector4 color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteComponent_GetScale(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteComponent_SetScale(ulong entityID, float scale);

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

		#region RigidBody2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyForce(ulong entityID, ref Vector2 force, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyForceToCenter(ulong entityID, ref Vector2 force, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse, bool wake);

		#endregion

		#region BoxCollider2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2D_GetDensity(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2D_SetDensity(ulong entityID, float density);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2D_GetFriction(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2D_SetFriction(ulong entityID, float friction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2D_GetRestitution(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2D_SetRestitution(ulong entityID, float restitution);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2D_GetRestitutionThreshold(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2D_SetRestitutionThreshold(ulong entityID, float restitutionThreshold);

		#endregion

		#region CircleCollider2D

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2D_GetDensity(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2D_SetDensity(ulong entityID, float density);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2D_GetFriction(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2D_SetFriction(ulong entityID, float friction);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2D_GetRestitution(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2D_SetRestitution(ulong entityID, float restitution);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2D_GetRestitutionThreshold(ulong entityID, out float result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleCollider2D_SetRestitutionThreshold(ulong entityID, float restitutionThreshold);

		#endregion

		#region Algebra

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static Vector3 Algebra_CrossProductVec3(ref Vector3 left, ref Vector3 right);

		#endregion

		#region Renderer

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Renderer_SetClearColor(ref Vector3 color);

		#endregion

		#region Input

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode key);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyUp(KeyCode key);

		#endregion

	}

}
