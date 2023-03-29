﻿using System;

namespace Vortex {

	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class Transform: Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
				return translation;
			}

			set => InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
		}

		public void Translate(Vector3 translation) => Translation += translation;
		public void Translate(float x, float y, float z) => Translate(new Vector3(x, y, z));

		public Quaternion Rotation
		{
			get
			{
				InternalCalls.TransformComponent_GetRotation(Entity.ID, out Quaternion rotation);
				return rotation;
			}

			set => InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
		}

		public Vector3 EulerAngles
		{
			get
			{
				InternalCalls.TransformComponent_GetEulerAngles(Entity.ID, out Vector3 eulerAngles);
				return eulerAngles;
			}

			set => InternalCalls.TransformComponent_SetEulerAngles(Entity.ID, ref value);
		}

		public void Rotate(Vector3 eulers, Space relativeTo = Space.Local)
		{
			InternalCalls.TransformComponent_Rotate(Entity.ID, ref eulers, relativeTo);
		}

		public void Rotate(float x, float y, float z, Space relativeTo = Space.Local) => Rotate(new Vector3(x, y, z), relativeTo);

		public void RotateAround(Vector3 worldPoint, Vector3 axis, float angle)
		{
			InternalCalls.TransformComponent_RotateAround(Entity.ID, ref worldPoint, ref axis, angle);
		}

		public Vector3 Scale
		{
			get
			{
				InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
				return scale;
			}

			set => InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
		}

		/// <summary>
		/// Sets the scale of the entity
		/// </summary>
		/// <param name="scale">the new scale</param>
		public void LocalScale(Vector3 scale) => Scale = scale;

		/// <summary>
		/// Sets the scale of the entity
		/// </summary>
		/// <param name="x">the new x scale</param>
		/// <param name="y">the new y scale</param>
		/// <param name="z">the new z scale</param>
		public void LocalScale(float x, float y, float z) => Scale = new Vector3(x, y, z);

		/// <summary>
		/// Applys the given scale to the current entity's scale
		/// </summary>
		/// <param name="scale">the scale to be applied</param>
		public void ApplyScale(Vector3 scale) => Scale += scale;

		/// <summary>
		/// Applys the given scale to the current entity's scale
		/// </summary>
		/// <param name="x">the x scale to be applied</param>
		/// <param name="y">the y scale to be applied</param>
		/// <param name="z">the z scale to be applied</param>
		public void ApplyScale(float x, float y, float z) => Scale += new Vector3(x, y, z);

		public struct WorldTransform
		{
			public Vector3 Translation;
			public Quaternion Rotation;
			public Vector3 EulerAngles;
			public Vector3 Scale;
		}

		public WorldTransform worldTransform
		{
			get
			{
				InternalCalls.TransformComponent_GetWorldSpaceTransform(Entity.ID, out Vector3 translation, out Quaternion rotation, out Vector3 eulers, out Vector3 scale);

				return new WorldTransform
				{
					Translation = translation,
					Rotation = rotation,
					EulerAngles = eulers,
					Scale = scale
				};
			}
		}

		public Matrix4 Matrix
		{
			get
			{
				InternalCalls.TransformComponent_GetTransformMatrix(Entity.ID, out Matrix4 result);
				return result;
			}

			set => InternalCalls.TransformComponent_SetTransformMatrix(Entity.ID, ref value);
		}

		public Vector3 Up
		{
			get
			{
				InternalCalls.TransformComponent_GetUpDirection(Entity.ID, out Vector3 result);
				return result;
			}
		}

		public Vector3 Right
		{
			get
			{
				InternalCalls.TransformComponent_GetRightDirection(Entity.ID, out Vector3 result);
				return result;
			}
		}

		public Vector3 Forward
		{
			get
			{
				InternalCalls.TransformComponent_GetForwardDirection(Entity.ID, out Vector3 result);
				return result;
			}
		}

		public Entity Parent
		{
			get
			{
				ulong entityID = InternalCalls.TransformComponent_GetParent(Entity.ID);

				if (entityID == 0)
				{
					return null;
				}

				return new Entity(entityID);
			}
		}

		public void SetParent(Entity parent) => InternalCalls.TransformComponent_SetParent(Entity.ID, parent.ID);
		public void Unparent() => InternalCalls.TransformComponent_Unparent(Entity.ID);

		public void SetTranslationAndRotation(Vector3 translation, Vector3 rotation)
		{
			InternalCalls.TransformComponent_SetTranslationAndRotation(Entity.ID, ref translation, ref rotation);
		}

		public void LookAt(Vector3 worldPoint)
		{
			InternalCalls.TransformComponent_LookAt(Entity.ID, ref worldPoint);
		}

		public static Transform operator *(Transform a, Transform b)
		{
			InternalCalls.TransformComponent_Multiply(ref a, ref b, out Transform result);
			return result;
		}
	}

	public class Camera: Component
	{
		public ProjectionType ProjectionType
		{
			get => InternalCalls.CameraComponent_GetProjectionType(Entity.ID);
			set => InternalCalls.CameraComponent_SetProjectionType(Entity.ID, value);
		}

		public bool IsPrimary
		{
			get
			{
				InternalCalls.CameraComponent_GetPrimary(Entity.ID, out bool primary);
				return primary;
			}

			set => InternalCalls.CameraComponent_SetPrimary(Entity.ID, value);
		}

		public float FieldOfView
		{
			get => InternalCalls.CameraComponent_GetPerspectiveVerticalFOV(Entity.ID);
			set => InternalCalls.CameraComponent_SetPerspectiveVerticalFOV(Entity.ID, value);
		}

		public float NearClip
		{
			get => InternalCalls.CameraComponent_GetNearClip(Entity.ID);
			set => InternalCalls.CameraComponent_SetNearClip(Entity.ID, value);
		}

		public float FarClip
		{
			get => InternalCalls.CameraComponent_GetFarClip(Entity.ID);
			set => InternalCalls.CameraComponent_SetFarClip(Entity.ID, value);
		}

		public float OrthographicSize
		{
			get => InternalCalls.CameraComponent_GetOrthographicSize(Entity.ID);
			set => InternalCalls.CameraComponent_SetOrthographicSize(Entity.ID, value);
		}

		public float OrthographicNear
		{
			get => InternalCalls.CameraComponent_GetOrthographicNear(Entity.ID);
			set => InternalCalls.CameraComponent_SetOrthographicNear(Entity.ID, value);
		}

		public float OrthographicFar
		{
			get => InternalCalls.CameraComponent_GetOrthographicFar(Entity.ID);
			set => InternalCalls.CameraComponent_SetOrthographicFar(Entity.ID, value);
		}

		public bool IsFixedAspectRatio
		{
			get
			{
				InternalCalls.CameraComponent_GetFixedAspectRatio(Entity.ID, out bool fixedAspectRatio);
				return fixedAspectRatio;
			}

			set => InternalCalls.CameraComponent_SetFixedAspectRatio(Entity.ID, value);
		}

		public Color3 ClearColor
		{
			get
			{
				InternalCalls.CameraComponent_GetClearColor(Entity.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.CameraComponent_SetClearColor(Entity.ID, ref value);
		}
	}

	public class LightSource: Component
	{
		public LightType LightType
		{
			get => InternalCalls.LightSourceComponent_GetLightType(Entity.ID);
			set => InternalCalls.LightSourceComponent_SetLightType(Entity.ID, value);
		}

		public Vector3 Radiance
		{
			get
			{
				InternalCalls.LightSourceComponent_GetRadiance(Entity.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.LightSourceComponent_SetRadiance(Entity.ID, ref value);
		}

		public float Intensity
		{
			get => InternalCalls.LightSourceComponent_GetIntensity(Entity.ID);
			set => InternalCalls.LightSourceComponent_SetIntensity(Entity.ID, value);
		}

		public float Cutoff
		{
			get => InternalCalls.LightSourceComponent_GetCutoff(Entity.ID);
			set => InternalCalls.LightSourceComponent_SetCutoff(Entity.ID, value);
		}

		public float OuterCutoff
		{
			get => InternalCalls.LightSourceComponent_GetOuterCutoff(Entity.ID);
			set => InternalCalls.LightSourceComponent_SetOuterCutoff(Entity.ID, value);
		}

		public float ShadowBias
		{
			get => InternalCalls.LightSourceComponent_GetShadowBias(Entity.ID);
			set => InternalCalls.LightSourceComponent_SetShadowBias(Entity.ID, value);
		}

		public bool CastShadows
		{
			get => InternalCalls.LightSourceComponent_GetCastShadows(Entity.ID);
			set => InternalCalls.LightSourceComponent_SetCastShadows(Entity.ID, value);
		}

		public bool UseSoftShadows
		{
			get => InternalCalls.LightSourceComponent_GetSoftShadows(Entity.ID);
			set => InternalCalls.LightSourceComponent_SetSoftShadows(Entity.ID, value);
		}

		public bool Visible
		{
			get => InternalCalls.LightSourceComponent_IsVisible(Entity.ID);
			set => InternalCalls.LightSourceComponent_SetVisible(Entity.ID, value);
		}
	}

	public class TextMesh: Component
	{
		public string Text
		{
			get => InternalCalls.TextMeshComponent_GetTextString(Entity.ID);
			set => InternalCalls.TextMeshComponent_SetTextString(Entity.ID, value);
		}

		public Vector4 Color
		{
			get
			{
				InternalCalls.TextMeshComponent_GetColor(Entity.ID, out Vector4 color);
				return color;
			}

			set => InternalCalls.TextMeshComponent_SetColor(Entity.ID, ref value);
		}

		public Vector4 BackgroundColor
		{
			get
			{
				InternalCalls.TextMeshComponent_GetBackgroundColor(Entity.ID, out Vector4 color);
				return color;
			}
			
			set => InternalCalls.TextMeshComponent_SetBackgroundColor(Entity.ID, ref value);
		}

		public float LineSpacing
		{
			get => InternalCalls.TextMeshComponent_GetLineSpacing(Entity.ID);
			set => InternalCalls.TextMeshComponent_SetLineSpacing(Entity.ID, value);
		}

		public float Kerning
		{
			get => InternalCalls.TextMeshComponent_GetKerning(Entity.ID);
			set => InternalCalls.TextMeshComponent_SetKerning(Entity.ID, value);
		}

		public float MaxWidth
		{
			get => InternalCalls.TextMeshComponent_GetMaxWidth(Entity.ID);
			set => InternalCalls.TextMeshComponent_SetMaxWidth(Entity.ID, value);
		}

		public bool Visible
		{
			get => InternalCalls.TextMeshRendererComponent_IsVisible(Entity.ID);
			set => InternalCalls.TextMeshRendererComponent_SetVisible(Entity.ID, value);
		}
	}

	public class Animation: Component
	{

	}

	public class Animator: Component
	{
		public bool IsPlaying => InternalCalls.AnimatorComponent_IsPlaying(Entity.ID);

		public void Play() => InternalCalls.AnimatorComponent_Play(Entity.ID);
		public void Stop() => InternalCalls.AnimatorComponent_Stop(Entity.ID);
	}

	public class MeshRenderer: Component
	{
		public Submesh BaseMesh => GetSubmesh(0);
		public Submesh GetSubmesh(uint index)
		{
			AssetHandle materialHandle = GetMaterialHandle(index);
			if (!materialHandle)
				return null;

			return new Submesh(materialHandle, index);
		}

		AssetHandle GetMaterialHandle(uint submeshIndex)
		{
			return InternalCalls.MeshRendererComponent_GetMaterialHandle(submeshIndex, Entity.ID, out AssetHandle materialHandle)
				? materialHandle : AssetHandle.Invalid;
		}

		public bool Visible
		{
			get => InternalCalls.MeshRendererComponent_IsVisible(Entity.ID);
			set => InternalCalls.MeshRendererComponent_SetVisible(Entity.ID, value);
		}
	}

	public class StaticMeshRenderer: Component
	{
		public MeshType MeshType
		{
			get => InternalCalls.StaticMeshRendererComponent_GetMeshType(Entity.ID);
			set => InternalCalls.StaticMeshRendererComponent_SetMeshType(Entity.ID, value);
		}

		public Submesh BaseMesh => GetSubmesh(0);
		public Submesh GetSubmesh(uint index)
		{
			AssetHandle materialHandle = GetMaterialHandle(index);
			if (!materialHandle)
				return null;

			return new Submesh(materialHandle, index);
		}

		public void SetMaterial(Submesh submesh, Material material)
		{
			SetMaterial(submesh.Index, material);
		}

		public void SetMaterial(uint index, Material material)
		{
			AssetHandle materialHandle = material.Handle;
			if (!materialHandle)
				return;

			InternalCalls.StaticMeshRendererComponent_SetMaterialHandle(index, Entity.ID, ref materialHandle);
		}

		public bool Visible
		{
			get => InternalCalls.StaticMeshRendererComponent_IsVisible(Entity.ID);
			set => InternalCalls.StaticMeshRendererComponent_SetVisible(Entity.ID, value);
		}

		AssetHandle GetMaterialHandle(uint submeshIndex)
		{
			return InternalCalls.StaticMeshRendererComponent_GetMaterialHandle(submeshIndex, Entity.ID, out AssetHandle materialHandle)
				? materialHandle : AssetHandle.Invalid;
		}
	}

	public class SpriteRenderer: Component
	{
		public Texture2D Texture
		{
			get
			{
				return InternalCalls.SpriteRendererComponent_GetTextureHandle(Entity.ID, out AssetHandle textureHandle)
					? new Texture2D(textureHandle) : null;
			}

			set => InternalCalls.SpriteRendererComponent_SetTextureHandle(Entity.ID, ref value.m_Handle);
		}

		public Vector4 Color
		{
			get
			{
				InternalCalls.SpriteRendererComponent_GetColor(Entity.ID, out Vector4 color);
				return color;
			}

			set => InternalCalls.SpriteRendererComponent_SetColor(Entity.ID, ref value);
		}

		public Vector2 Scale
		{
			get
			{
				InternalCalls.SpriteRendererComponent_GetUV(Entity.ID, out Vector2 scale);
				return scale;
			}

			set => InternalCalls.SpriteRendererComponent_SetUV(Entity.ID, ref value);
		}

		public bool Visible
		{
			get => InternalCalls.SpriteRendererComponent_IsVisible(Entity.ID);
			set => InternalCalls.SpriteRendererComponent_SetVisible(Entity.ID, value);
		}
	}

	public class CircleRenderer: Component
	{
		public Vector4 Color
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetColor(Entity.ID, out Vector4 color);
				return color;
			}

			set => InternalCalls.CircleRendererComponent_SetColor(Entity.ID, ref value);
		}

		public float Thickness
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetThickness(Entity.ID, out float thickness);
				return thickness;
			}

			set => InternalCalls.CircleRendererComponent_SetThickness(Entity.ID, value);
		}

		public float Fade
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetFade(Entity.ID, out float fade);
				return fade;
			}

			set => InternalCalls.CircleRendererComponent_SetFade(Entity.ID, value);
		}

		public bool Visible
		{
			get => InternalCalls.CircleRendererComponent_IsVisible(Entity.ID);
			set => InternalCalls.CircleRendererComponent_SetVisible(Entity.ID, value);
		}
	}

	public class AudioSource: Component
	{
		public AudioClip Clip => new AudioClip(this);

		public bool IsPlaying => InternalCalls.AudioSourceComponent_GetIsPlaying(Entity.ID);

		public Vector3 Position
		{
			get
			{
				InternalCalls.AudioSourceComponent_GetPosition(Entity.ID, out Vector3 position);
				return position;
			}

			set => InternalCalls.AudioSourceComponent_SetPosition(Entity.ID, ref value);
		}

		public Vector3 Direction
		{
			get
			{
				InternalCalls.AudioSourceComponent_GetDirection(Entity.ID, out Vector3 direction);
				return direction;
			}

			set => InternalCalls.AudioSourceComponent_SetDirection(Entity.ID, ref value);
		}

		public Vector3 Velocity
		{
			get
			{
				InternalCalls.AudioSourceComponent_GetVelocity(Entity.ID, out Vector3 veloctiy);
				return veloctiy;
			}

			set => InternalCalls.AudioSourceComponent_SetVelocity(Entity.ID, ref value);
		}

		public AudioCone<AudioSource> Cone => new AudioCone<AudioSource>(this);

		public float MinGain
		{
			get => InternalCalls.AudioSourceComponent_GetMinGain(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetMinGain(Entity.ID, value);
		}

		public float MaxGain
		{
			get => InternalCalls.AudioSourceComponent_GetMaxGain(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetMaxGain(Entity.ID, value);
		}

		public AttenuationModel Attenuation
		{
			get => InternalCalls.AudioSourceComponent_GetAttenuationModel(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetAttenuationModel(Entity.ID, value);
		}

		public float Falloff
		{
			get => InternalCalls.AudioSourceComponent_GetFalloff(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetFalloff(Entity.ID, value);
		}

		public float MinDistance
		{
			get => InternalCalls.AudioSourceComponent_GetMinDistance(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetMinDistance(Entity.ID, value);
		}

		public float MaxDistance
		{
			get => InternalCalls.AudioSourceComponent_GetMaxDistance(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetMaxDistance(Entity.ID, value);
		}

		public float Pitch
		{
			get => InternalCalls.AudioSourceComponent_GetPitch(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetPitch(Entity.ID, value);
		}

		public float DopplerFactor
		{
			get => InternalCalls.AudioSourceComponent_GetDopplerFactor(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetDopplerFactor(Entity.ID, value);
		}

		public float Volume
		{
			get => InternalCalls.AudioSourceComponent_GetVolume(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetVolume(Entity.ID, value);
		}

		public bool PlayOnStart
		{
			get => InternalCalls.AudioSourceComponent_GetPlayOnStart(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetPlayOnStart(Entity.ID, value);
		}

		public bool IsSpacialized
		{
			get => InternalCalls.AudioSourceComponent_GetIsSpacialized(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetIsSpacialized(Entity.ID, value);
		}

		public bool IsLooping
		{
			get => InternalCalls.AudioSourceComponent_GetIsLooping(Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetIsLooping(Entity.ID, value);
		}

		public void Play() => InternalCalls.AudioSourceComponent_Play(Entity.ID);
		public void PlayOneShot() => InternalCalls.AudioSourceComponent_PlayOneShot(Entity.ID);
		public void Restart() => InternalCalls.AudioSourceComponent_Restart(Entity.ID);
		public void Stop() => InternalCalls.AudioSourceComponent_Stop(Entity.ID);
	}

	public class AudioListener: Component
	{
		public AudioCone<AudioListener> Cone => new AudioCone<AudioListener>(this);
	}

	public class RigidBody: Component
	{
		public RigidBodyType BodyType
		{
			get => InternalCalls.RigidBodyComponent_GetBodyType(Entity.ID);
			set => InternalCalls.RigidBodyComponent_SetBodyType(Entity.ID, value);
		}

		public CollisionDetectionType CollisionDetection
		{
			get => InternalCalls.RigidBodyComponent_GetCollisionDetectionType(Entity.ID);
			set => InternalCalls.RigidBodyComponent_SetCollisionDetectionType(Entity.ID, value);
		}

		public float Mass
		{
			get => InternalCalls.RigidBodyComponent_GetMass(Entity.ID);
			set => InternalCalls.RigidBodyComponent_SetMass(Entity.ID, value);
		}

		public Vector3 Velocity
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetLinearVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}
			set => InternalCalls.RigidBodyComponent_SetLinearVelocity(Entity.ID, ref value);
		}

		public float MaxVelocity
		{
			get => InternalCalls.RigidBodyComponent_GetMaxLinearVelocity(Entity.ID);
			set => InternalCalls.RigidBodyComponent_SetMaxLinearVelocity(Entity.ID, value);
		}

		public float Drag
		{
			get =>InternalCalls.RigidBodyComponent_GetLinearDrag(Entity.ID);
			set => InternalCalls.RigidBodyComponent_SetLinearDrag(Entity.ID, value);
		}

		public Vector3 AngularVelocity
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetAngularVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}
			set => InternalCalls.RigidBodyComponent_SetAngularVelocity(Entity.ID, ref value);
		}

		public float MaxAngularVelocity
		{
			get => InternalCalls.RigidBodyComponent_GetMaxAngularVelocity(Entity.ID);
			set => InternalCalls.RigidBodyComponent_SetMaxAngularVelocity(Entity.ID, value);
		}

		public float AngularDrag
		{
			get => InternalCalls.RigidBodyComponent_GetAngularDrag(Entity.ID);
			set => InternalCalls.RigidBodyComponent_SetAngularDrag(Entity.ID, value);
		}

		public bool DisableGravity
		{
			get => InternalCalls.RigidBodyComponent_GetDisableGravity(Entity.ID);
			set => InternalCalls.RigidBodyComponent_SetDisableGravity(Entity.ID, value);
		}

		public bool IsKinematic
		{
			get => InternalCalls.RigidBodyComponent_GetIsKinematic(Entity.ID);
			set => InternalCalls.RigidBodyComponent_SetIsKinematic(Entity.ID, value);
		}

		public Vector3 KinematicTarget
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetKinematicTargetTranslation(Entity.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.RigidBodyComponent_SetKinematicTargetTranslation(Entity.ID, ref value);
		}

		public Quaternion KinematicTargetRotation
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetKinematicTargetRotation(Entity.ID, out Quaternion result);
				return result;
			}

			set => InternalCalls.RigidBodyComponent_SetKinematicTargetRotation(Entity.ID, ref value);
		}

		public uint GetLockFlags() => InternalCalls.RigidBodyComponent_GetLockFlags(Entity.ID);
		
		public void SetLockFlag(ActorLockFlag flag, bool value, bool forceWake = false)
		{
			InternalCalls.RigidBodyComponent_SetLockFlag(Entity.ID, flag, value, forceWake);
		}

		public bool IsLockFlagSet(ActorLockFlag flag)
		{
			return InternalCalls.RigidBodyComponent_IsLockFlagSet(Entity.ID, flag);
		}

		public bool IsSleeping => InternalCalls.RigidBodyComponent_IsSleeping(Entity.ID);
		public void WakeUp() => InternalCalls.RigidBodyComponent_WakeUp(Entity.ID);

		public void AddForce(Vector3 force, ForceMode forceMode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_AddForce(Entity.ID, ref force, forceMode);
		}
		
		public void AddForceAtPosition(Vector3 force, Vector3 position, ForceMode forceMode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_AddForceAtPosition(Entity.ID, ref force, ref position, forceMode);
		}

		public void AddTorque(Vector3 torque, ForceMode forceMode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_AddTorque(Entity.ID, ref torque, forceMode);
		}

		public void ClearTorque(ForceMode mode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_ClearTorque(Entity.ID, mode);
		}

		public void ClearForce(ForceMode mode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_ClearForce(Entity.ID, mode);
		}
	}

	public class CharacterController: Component
	{
		public float SpeedDown => InternalCalls.CharacterControllerComponent_GetSpeedDown(Entity.ID);
		public bool IsGrounded => InternalCalls.CharacterControllerComponent_IsGrounded(Entity.ID);

		public Vector3 FootPosition
		{
			get
			{
				InternalCalls.CharacterControllerComponent_GetFootPosition(Entity.ID, out Vector3 footPosition);
				return footPosition;
			}
		}

		public float SlopeLimit
		{
			get => InternalCalls.CharacterControllerComponent_GetSlopeLimit(Entity.ID);
			set => InternalCalls.CharacterControllerComponent_SetSlopeLimit(Entity.ID, value);
		}

		public float StepOffset
		{
			get => InternalCalls.CharacterControllerComponent_GetStepOffset(Entity.ID);
			set => InternalCalls.CharacterControllerComponent_SetStepOffset(Entity.ID, value);
		}

		public float ContactOffset
		{
			get => InternalCalls.CharacterControllerComponent_GetContactOffset(Entity.ID);
			set => InternalCalls.CharacterControllerComponent_SetContactOffset(Entity.ID, value);
		}

		public bool DisableGravity
		{
			get => InternalCalls.CharacterControllerComponent_GetDisableGravity(Entity.ID);
			set => InternalCalls.CharacterControllerComponent_SetDisableGravity(Entity.ID, value);
		}

		public NonWalkableMode NonWalkMode
		{
			get => InternalCalls.CharacterControllerComponent_GetNonWalkableMode(Entity.ID);
			set => InternalCalls.CharacterControllerComponent_SetNonWalkableMode(Entity.ID, value);
		}

		public CapsuleClimbMode ClimbMode
		{
			get => InternalCalls.CharacterControllerComponent_GetClimbMode(Entity.ID);
			set => InternalCalls.CharacterControllerComponent_SetClimbMode(Entity.ID, value);
		}

		public void Move(Vector3 displacement)
		{
			InternalCalls.CharacterControllerComponent_Move(Entity.ID, ref displacement);
		}

		public void Jump(float jumpForce)
		{
			InternalCalls.CharacterControllerComponent_Jump(Entity.ID, jumpForce);
		}
	}

	public class FixedJoint: Component
	{
		public Entity ConnectedEntity
		{
			get
			{
				ulong entityID = InternalCalls.FixedJointComponent_GetConnectedEntity(Entity.ID);

				if (entityID != 0)
				{
					return new Entity(entityID);
				}

				return null;
			}

			set => InternalCalls.FixedJointComponent_SetConnectedEntity(Entity.ID, value.ID);
		}

		public float BreakForce
		{
			get => InternalCalls.FixedJointComponent_GetBreakForce(Entity.ID);
			set => InternalCalls.FixedJointComponent_SetBreakForce(Entity.ID, value);
		}

		public float BreakTorque
		{
			get => InternalCalls.FixedJointComponent_GetBreakTorque(Entity.ID);
			set => InternalCalls.FixedJointComponent_SetBreakTorque(Entity.ID, value);
		}

		public void SetBreakForceAndTorque(float breakForce, float breakTorque)
		{
			InternalCalls.FixedJointComponent_SetBreakForceAndTorque(Entity.ID, breakForce, breakTorque);
		}

		public bool IsCollisionEnabled
		{
			get => InternalCalls.FixedJointComponent_GetEnableCollision(Entity.ID);
			set => InternalCalls.FixedJointComponent_SetCollisionEnabled(Entity.ID, value);
		}

		public bool IsPreProcessingEnabled
		{
			get => InternalCalls.FixedJointComponent_GetPreProcessingEnabled(Entity.ID);
			set => InternalCalls.FixedJointComponent_SetPreProcessingEnabled(Entity.ID, value);
		}

		public bool IsBroken => InternalCalls.FixedJointComponent_IsBroken(Entity.ID);

		public bool IsBreakable
		{
			get => InternalCalls.FixedJointComponent_GetIsBreakable(Entity.ID);
			set => InternalCalls.FixedJointComponent_SetIsBreakable(Entity.ID, value);
		}

		public void Break() => InternalCalls.FixedJointComponent_Break(Entity.ID);
	}

	public class BoxCollider: Component
	{
		public Vector3 HalfSize
		{
			get
			{
				InternalCalls.BoxColliderComponent_GetHalfSize(Entity.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.BoxColliderComponent_SetHalfSize(Entity.ID, ref value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.BoxColliderComponent_GetOffset(Entity.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.BoxColliderComponent_SetOffset(Entity.ID, ref value);
		}

		public bool IsTrigger
		{
			get => InternalCalls.BoxColliderComponent_GetIsTrigger(Entity.ID);
			set => InternalCalls.BoxColliderComponent_SetIsTrigger(Entity.ID, value);
		}

		public PhysicsMaterial Material
		{
			get
			{
				return InternalCalls.BoxColliderComponent_GetMaterialHandle(Entity.ID, out AssetHandle materialHandle)
					? new PhysicsMaterial(materialHandle) : null;
			}
		}
	}

	public class SphereCollider: Component
	{
		public float Radius
		{
			get => InternalCalls.SphereColliderComponent_GetRadius(Entity.ID);
			set => InternalCalls.SphereColliderComponent_SetRadius(Entity.ID, value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.SphereColliderComponent_GetOffset(Entity.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.SphereColliderComponent_SetOffset(Entity.ID, ref value);
		}

		public bool IsTrigger
		{
			get => InternalCalls.SphereColliderComponent_GetIsTrigger(Entity.ID);
			set => InternalCalls.SphereColliderComponent_SetIsTrigger(Entity.ID, value);
		}

		public PhysicsMaterial Material
		{
			get
			{
				return InternalCalls.SphereColliderComponent_GetMaterialHandle(Entity.ID, out AssetHandle materialHandle)
					? new PhysicsMaterial(materialHandle) : null;
			}
		}
	}

	public class CapsuleCollider: Component
	{
		public float Radius
		{
			get => InternalCalls.CapsuleColliderComponent_GetRadius(Entity.ID);
			set => InternalCalls.CapsuleColliderComponent_SetRadius(Entity.ID, value);
		}
		
		public float Height
		{
			get => InternalCalls.CapsuleColliderComponent_GetHeight(Entity.ID);
			set => InternalCalls.CapsuleColliderComponent_SetHeight(Entity.ID, value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.CapsuleColliderComponent_GetOffset(Entity.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.CapsuleColliderComponent_SetOffset(Entity.ID, ref value);
		}

		public bool IsTrigger
		{
			get => InternalCalls.CapsuleColliderComponent_GetIsTrigger(Entity.ID);
			set => InternalCalls.CapsuleColliderComponent_SetIsTrigger(Entity.ID, value);
		}

		public PhysicsMaterial Material
		{
			get
			{
				return InternalCalls.CapsuleColliderComponent_GetMaterialHandle(Entity.ID, out AssetHandle materialHandle)
					? new PhysicsMaterial(materialHandle) : null;
			}
		}
	}

	public class MeshCollider: Component
	{
		public bool IsStaticMesh => InternalCalls.MeshColliderComponent_IsStaticMesh(Entity.ID);

		public AssetHandle ColliderMeshHandle
		{
			get => InternalCalls.MeshColliderComponent_GetColliderMesh(Entity.ID, out AssetHandle colliderHandle)
				? colliderHandle : AssetHandle.Invalid;
		}

		public bool IsTrigger
		{
			get => InternalCalls.MeshColliderComponent_GetIsTrigger(Entity.ID);
			set => InternalCalls.MeshColliderComponent_SetIsTrigger(Entity.ID, value);
		}

		public PhysicsMaterial Material
		{
			get => InternalCalls.MeshColliderComponent_GetMaterialHandle(Entity.ID, out AssetHandle materialHandle)
				? new PhysicsMaterial(materialHandle) : null;
		}
	}

	public class RigidBody2D: Component
	{
		public RigidBody2DType BodyType
		{
			get => InternalCalls.RigidBody2DComponent_GetBodyType(Entity.ID);
			set => InternalCalls.RigidBody2DComponent_SetBodyType(Entity.ID, value);
		}

		public Vector2 Velocity
		{
			get
			{
				InternalCalls.RigidBody2DComponent_GetVelocity(Entity.ID, out Vector2 velocity);
				return velocity;
			}
			set => InternalCalls.RigidBody2DComponent_SetVelocity(Entity.ID, ref value);
		}

		public float Drag
		{
			get => InternalCalls.RigidBody2DComponent_GetDrag(Entity.ID);
			set => InternalCalls.RigidBody2DComponent_SetDrag(Entity.ID, value);
		}

		public float AngularVelocity
		{
			get => InternalCalls.RigidBody2DComponent_GetAngularVelocity(Entity.ID);
			set => InternalCalls.RigidBody2DComponent_SetAngularVelocity(Entity.ID, value);
		}

		public float AngularDrag
		{
			get => InternalCalls.RigidBody2DComponent_GetAngularDrag(Entity.ID);
			set => InternalCalls.RigidBody2DComponent_SetAngularDrag(Entity.ID, value);
		}

		public float GravityScale
		{
			get => InternalCalls.RigidBody2DComponent_GetGravityScale(Entity.ID);
			set => InternalCalls.RigidBody2DComponent_SetGravityScale(Entity.ID, value);
		}

		public bool FreezeRotation
		{
			get => InternalCalls.RigidBody2DComponent_GetFixedRotation(Entity.ID);
			set => InternalCalls.RigidBody2DComponent_SetFixedRotation(Entity.ID, value);
		}

		public void ApplyForce(Vector2 force, Vector2 worldPosition, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyForce(Entity.ID, ref force, ref worldPosition, wake);
		}

		public void ApplyForce(Vector2 force, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyForceToCenter(Entity.ID, ref force, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
		}
	}

	public class BoxCollider2D: Component
	{
		public Vector2 Offset
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetOffset(Entity.ID, out Vector2 result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetOffset(Entity.ID, ref value);
		}

		public Vector2 Size
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetSize(Entity.ID, out Vector2 result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetSize(Entity.ID, ref value);
		}

		public float Density
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetDensity(Entity.ID, out float result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetDensity(Entity.ID, value);
		}

		public float Friction
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetFriction(Entity.ID, out float result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetFriction(Entity.ID, value);
		}

		public float Restitution
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetRestitution(Entity.ID, out float result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetRestitution(Entity.ID, value);
		}

		public float RestitutionThreshold
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetRestitutionThreshold(Entity.ID, out float result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetRestitutionThreshold(Entity.ID, value);
		}
	}

	public class CircleCollider2D: Component
	{
		public Vector2 Offset
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetOffset(Entity.ID, out Vector2 result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetOffset(Entity.ID, ref value);
		}

		public float Radius
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetRadius(Entity.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetRadius(Entity.ID, value);
		}

		public float Density
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetDensity(Entity.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetDensity(Entity.ID, value);
		}

		public float Friction
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetFriction(Entity.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetFriction(Entity.ID, value);
		}

		public float Restitution
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetRestitution(Entity.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetRestitution(Entity.ID, value);
		}

		public float RestitutionThreshold
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetRestitutionThreshold(Entity.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetRestitutionThreshold(Entity.ID, value);
		}
	}

	public class ParticleEmitter: Component
	{
		public Vector3 Velocity
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}

			set => InternalCalls.ParticleEmitterComponent_SetVelocity(Entity.ID, ref value);
		}

		public Vector3 VelocityVariation
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetVelocityVariation(Entity.ID, out Vector3 velocityVariation);
				return velocityVariation;
			}

			set => InternalCalls.ParticleEmitterComponent_SetVelocityVariation(Entity.ID, ref value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetOffset(Entity.ID, out Vector3 offset);
				return offset;
			}

			set => InternalCalls.ParticleEmitterComponent_SetOffset(Entity.ID, ref value);
		}

		public Vector2 SizeBegin
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetSizeBegin(Entity.ID, out Vector2 sizeBegin);
				return sizeBegin;
			}

			set => InternalCalls.ParticleEmitterComponent_SetSizeBegin(Entity.ID, ref value);
		}

		public Vector2 SizeEnd
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetSizeEnd(Entity.ID, out Vector2 sizeEnd);
				return sizeEnd;
			}

			set => InternalCalls.ParticleEmitterComponent_SetSizeEnd(Entity.ID, ref value);
		}

		public Vector2 SizeVariation
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetSizeVariation(Entity.ID, out Vector2 sizeVariation);
				return sizeVariation;
			}

			set => InternalCalls.ParticleEmitterComponent_SetSizeVariation(Entity.ID, ref value);
		}

		public Vector4 ColorBegin
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetColorBegin(Entity.ID, out Vector4 colorBegin);
				return colorBegin;
			}

			set => InternalCalls.ParticleEmitterComponent_SetColorBegin(Entity.ID, ref value);
		}

		public Vector4 ColorEnd
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetColorEnd(Entity.ID, out Vector4 colorEnd);
				return colorEnd;
			}

			set => InternalCalls.ParticleEmitterComponent_SetColorEnd(Entity.ID, ref value);
		}

		public float Rotation
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetRotation(Entity.ID, out float rotation);
				return rotation;
			}

			set => InternalCalls.ParticleEmitterComponent_SetRotation(Entity.ID, value);
		}

		public float LifeTime
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetLifeTime(Entity.ID, out float lifetime);
				return lifetime;
			}

			set => InternalCalls.ParticleEmitterComponent_SetLifeTime(Entity.ID, value);
		}

		public void Start() => InternalCalls.ParticleEmitterComponent_Start(Entity.ID);

		public void Stop() => InternalCalls.ParticleEmitterComponent_Stop(Entity.ID);
	}

	public class Script: Component
	{
		public object Instance => InternalCalls.Entity_GetScriptInstance(Entity.ID);
	}

}
