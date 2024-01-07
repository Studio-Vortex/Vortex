namespace Vortex {

	public abstract class Component
	{
		public Actor Actor { get; internal set; }
	}

	public class Transform: Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(Actor.ID, out Vector3 translation);
				return translation;
			}

			set => InternalCalls.TransformComponent_SetTranslation(Actor.ID, ref value);
		}

		public void Translate(Vector3 translation) => Translation += translation;
		public void Translate(float x, float y, float z) => Translate(new Vector3(x, y, z));

		public Quaternion Rotation
		{
			get
			{
				InternalCalls.TransformComponent_GetRotation(Actor.ID, out Quaternion rotation);
				return rotation;
			}

			set => InternalCalls.TransformComponent_SetRotation(Actor.ID, ref value);
		}

		public Vector3 EulerAngles
		{
			get
			{
				InternalCalls.TransformComponent_GetEulerAngles(Actor.ID, out Vector3 eulerAngles);
				return eulerAngles;
			}

			set => InternalCalls.TransformComponent_SetEulerAngles(Actor.ID, ref value);
		}

		public void Rotate(Vector3 eulers, Space relativeTo = Space.Local)
		{
			InternalCalls.TransformComponent_Rotate(Actor.ID, ref eulers, relativeTo);
		}

		public void Rotate(float x, float y, float z, Space relativeTo = Space.Local) => Rotate(new Vector3(x, y, z), relativeTo);

		public void RotateAround(Vector3 worldPoint, Vector3 axis, float angle)
		{
			InternalCalls.TransformComponent_RotateAround(Actor.ID, ref worldPoint, ref axis, angle);
		}

		public Vector3 Scale
		{
			get
			{
				InternalCalls.TransformComponent_GetScale(Actor.ID, out Vector3 scale);
				return scale;
			}

			set => InternalCalls.TransformComponent_SetScale(Actor.ID, ref value);
		}

		/// <summary>
		/// Sets the scale of the actor
		/// </summary>
		/// <param name="scale">the new scale</param>
		public void LocalScale(Vector3 scale) => Scale = scale;

		/// <summary>
		/// Sets the scale of the actor
		/// </summary>
		/// <param name="x">the new x scale</param>
		/// <param name="y">the new y scale</param>
		/// <param name="z">the new z scale</param>
		public void LocalScale(float x, float y, float z) => Scale = new Vector3(x, y, z);

		/// <summary>
		/// Applys the given scale to the current actor's scale
		/// </summary>
		/// <param name="scale">the scale to be applied</param>
		public void ApplyScale(Vector3 scale) => Scale += scale;

		/// <summary>
		/// Applys the given scale to the current actor's scale
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
				InternalCalls.TransformComponent_GetWorldSpaceTransform(Actor.ID, out Vector3 translation, out Quaternion rotation, out Vector3 eulers, out Vector3 scale);

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
				InternalCalls.TransformComponent_GetTransformMatrix(Actor.ID, out Matrix4 result);
				return result;
			}

			set => InternalCalls.TransformComponent_SetTransformMatrix(Actor.ID, ref value);
		}

		public Vector3 Forward
		{
			get
			{
				InternalCalls.TransformComponent_GetForwardDirection(Actor.ID, out Vector3 result);
				return result;
			}
		}

		public Vector3 Backward
		{
			get
			{
				InternalCalls.TransformComponent_GetBackwardDirection(Actor.ID, out Vector3 result);
				return result;
			}
		}

		public Vector3 Up
		{
			get
			{
				InternalCalls.TransformComponent_GetUpDirection(Actor.ID, out Vector3 result);
				return result;
			}
		}

		public Vector3 Down
		{
			get
			{
				InternalCalls.TransformComponent_GetDownDirection(Actor.ID, out Vector3 result);
				return result;
			}
		}

		public Vector3 Right
		{
			get
			{
				InternalCalls.TransformComponent_GetRightDirection(Actor.ID, out Vector3 result);
				return result;
			}
		}

		public Vector3 Left
		{
			get
			{
				InternalCalls.TransformComponent_GetLeftDirection(Actor.ID, out Vector3 result);
				return result;
			}
		}

		public Actor Parent
		{
			get
			{
				ulong actorID = InternalCalls.TransformComponent_GetParent(Actor.ID);

				if (actorID == 0)
				{
					return null;
				}

				return new Actor(actorID);
			}
		}

		public void SetParent(Actor parent) => InternalCalls.TransformComponent_SetParent(Actor.ID, parent.ID);
		public void Unparent() => InternalCalls.TransformComponent_Unparent(Actor.ID);

		public void SetTranslationAndRotation(Vector3 translation, Vector3 rotation)
		{
			InternalCalls.TransformComponent_SetTranslationAndRotation(Actor.ID, ref translation, ref rotation);
		}

		public void LookAt(Vector3 worldPoint) => InternalCalls.TransformComponent_LookAt(Actor.ID, ref worldPoint);
		public void LookAt(Actor target) => LookAt(target.transform.Translation);

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
			get => InternalCalls.CameraComponent_GetProjectionType(Actor.ID);
			set => InternalCalls.CameraComponent_SetProjectionType(Actor.ID, value);
		}

		public static Camera Primary
		{
			get
			{
				ulong actorID = InternalCalls.Scene_GetPrimaryCamera();

				if (actorID == 0)
					return null;

				Actor primaryCameraActor = new Actor(actorID);
				return primaryCameraActor.GetComponent<Camera>();
			}
		}

		public bool IsPrimary
		{
			get
			{
				InternalCalls.CameraComponent_GetPrimary(Actor.ID, out bool primary);
				return primary;
			}

			set => InternalCalls.CameraComponent_SetPrimary(Actor.ID, value);
		}

		public float FieldOfView
		{
			get => InternalCalls.CameraComponent_GetPerspectiveVerticalFOV(Actor.ID);
			set => InternalCalls.CameraComponent_SetPerspectiveVerticalFOV(Actor.ID, value);
		}

		public float NearClip
		{
			get => InternalCalls.CameraComponent_GetNearClip(Actor.ID);
			set => InternalCalls.CameraComponent_SetNearClip(Actor.ID, value);
		}

		public float FarClip
		{
			get => InternalCalls.CameraComponent_GetFarClip(Actor.ID);
			set => InternalCalls.CameraComponent_SetFarClip(Actor.ID, value);
		}

		public float OrthographicSize
		{
			get => InternalCalls.CameraComponent_GetOrthographicSize(Actor.ID);
			set => InternalCalls.CameraComponent_SetOrthographicSize(Actor.ID, value);
		}

		public float OrthographicNear
		{
			get => InternalCalls.CameraComponent_GetOrthographicNear(Actor.ID);
			set => InternalCalls.CameraComponent_SetOrthographicNear(Actor.ID, value);
		}

		public float OrthographicFar
		{
			get => InternalCalls.CameraComponent_GetOrthographicFar(Actor.ID);
			set => InternalCalls.CameraComponent_SetOrthographicFar(Actor.ID, value);
		}

		public Color3 ClearColor
		{
			get
			{
				InternalCalls.CameraComponent_GetClearColor(Actor.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.CameraComponent_SetClearColor(Actor.ID, ref value);
		}

		public bool IsFixedAspectRatio
		{
			get
			{
				InternalCalls.CameraComponent_GetFixedAspectRatio(Actor.ID, out bool fixedAspectRatio);
				return fixedAspectRatio;
			}

			set => InternalCalls.CameraComponent_SetFixedAspectRatio(Actor.ID, value);
		}

		public PostProcessInfo PostProcess
		{
			get => new PostProcessInfo(Actor);
		}

		public Ray Raycast(Vector2 position, float maxDistance)
		{
			InternalCalls.CameraComponent_Raycast(Actor.ID, ref position, maxDistance, out Ray result);
			return result;
		}

		public Vector3 ScreenToWorldPoint(Vector2 position, float maxDistance)
		{
			InternalCalls.CameraComponent_ScreenToWorldPoint(Actor.ID, ref position, maxDistance, out Vector3 result);
			return result;
		}

		public Vector2 ScreenToViewportPoint(Vector2 position)
		{
			InternalCalls.CameraComponent_ScreenToViewportPoint(Actor.ID, ref position, out Vector2 result);
			return result;
		}
	}

	public class LightSource: Component
	{
		public LightType LightType
		{
			get => InternalCalls.LightSourceComponent_GetLightType(Actor.ID);
			set => InternalCalls.LightSourceComponent_SetLightType(Actor.ID, value);
		}

		public Vector3 Radiance
		{
			get
			{
				InternalCalls.LightSourceComponent_GetRadiance(Actor.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.LightSourceComponent_SetRadiance(Actor.ID, ref value);
		}

		public float Intensity
		{
			get => InternalCalls.LightSourceComponent_GetIntensity(Actor.ID);
			set => InternalCalls.LightSourceComponent_SetIntensity(Actor.ID, value);
		}

		public float Cutoff
		{
			get => InternalCalls.LightSourceComponent_GetCutoff(Actor.ID);
			set => InternalCalls.LightSourceComponent_SetCutoff(Actor.ID, value);
		}

		public float OuterCutoff
		{
			get => InternalCalls.LightSourceComponent_GetOuterCutoff(Actor.ID);
			set => InternalCalls.LightSourceComponent_SetOuterCutoff(Actor.ID, value);
		}

		public float ShadowBias
		{
			get => InternalCalls.LightSourceComponent_GetShadowBias(Actor.ID);
			set => InternalCalls.LightSourceComponent_SetShadowBias(Actor.ID, value);
		}

		public bool CastShadows
		{
			get => InternalCalls.LightSourceComponent_GetCastShadows(Actor.ID);
			set => InternalCalls.LightSourceComponent_SetCastShadows(Actor.ID, value);
		}

		public bool UseSoftShadows
		{
			get => InternalCalls.LightSourceComponent_GetSoftShadows(Actor.ID);
			set => InternalCalls.LightSourceComponent_SetSoftShadows(Actor.ID, value);
		}

		public bool Visible
		{
			get => InternalCalls.LightSourceComponent_IsVisible(Actor.ID);
			set => InternalCalls.LightSourceComponent_SetVisible(Actor.ID, value);
		}
	}

	public class TextMesh: Component
	{
		public string Text
		{
			get => InternalCalls.TextMeshComponent_GetTextString(Actor.ID);
			set => InternalCalls.TextMeshComponent_SetTextString(Actor.ID, value);
		}

		public Vector4 Color
		{
			get
			{
				InternalCalls.TextMeshComponent_GetColor(Actor.ID, out Vector4 color);
				return color;
			}

			set => InternalCalls.TextMeshComponent_SetColor(Actor.ID, ref value);
		}

		public Vector4 OutlineColor
		{
			get
			{
				InternalCalls.TextMeshComponent_GetOutlineColor(Actor.ID, out Vector4 color);
				return color;
			}
			
			set => InternalCalls.TextMeshComponent_SetOutlineColor(Actor.ID, ref value);
		}

		public float LineSpacing
		{
			get => InternalCalls.TextMeshComponent_GetLineSpacing(Actor.ID);
			set => InternalCalls.TextMeshComponent_SetLineSpacing(Actor.ID, value);
		}

		public float Kerning
		{
			get => InternalCalls.TextMeshComponent_GetKerning(Actor.ID);
			set => InternalCalls.TextMeshComponent_SetKerning(Actor.ID, value);
		}

		public float MaxWidth
		{
			get => InternalCalls.TextMeshComponent_GetMaxWidth(Actor.ID);
			set => InternalCalls.TextMeshComponent_SetMaxWidth(Actor.ID, value);
		}

		public bool Visible
		{
			get => InternalCalls.TextMeshComponent_IsVisible(Actor.ID);
			set => InternalCalls.TextMeshComponent_SetVisible(Actor.ID, value);
		}
	}

	public class Button: Component
	{

	}

	public class Animation: Component
	{

	}

	public class Animator: Component
	{
		public bool IsPlaying => InternalCalls.AnimatorComponent_IsPlaying(Actor.ID);

		public void Play() => InternalCalls.AnimatorComponent_Play(Actor.ID);
		public void Stop() => InternalCalls.AnimatorComponent_Stop(Actor.ID);
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
			return InternalCalls.MeshRendererComponent_GetMaterialHandle(submeshIndex, Actor.ID, out AssetHandle materialHandle)
				? materialHandle : AssetHandle.Invalid;
		}

		public bool Visible
		{
			get => InternalCalls.MeshRendererComponent_IsVisible(Actor.ID);
			set => InternalCalls.MeshRendererComponent_SetVisible(Actor.ID, value);
		}

		public bool CastShadows
		{
			get => InternalCalls.MeshRendererComponent_GetCastShadows(Actor.ID);
			set => InternalCalls.MeshRendererComponent_SetCastShadows(Actor.ID, value);
		}
	}

	public class StaticMeshRenderer: Component
	{
		public MeshType MeshType
		{
			get => InternalCalls.StaticMeshRendererComponent_GetMeshType(Actor.ID);
			set => InternalCalls.StaticMeshRendererComponent_SetMeshType(Actor.ID, value);
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

			InternalCalls.StaticMeshRendererComponent_SetMaterialHandle(index, Actor.ID, ref materialHandle);
		}

		public bool Visible
		{
			get => InternalCalls.StaticMeshRendererComponent_IsVisible(Actor.ID);
			set => InternalCalls.StaticMeshRendererComponent_SetVisible(Actor.ID, value);
		}

		public bool CastShadows
		{
			get => InternalCalls.StaticMeshRendererComponent_GetCastShadows(Actor.ID);
			set => InternalCalls.StaticMeshRendererComponent_SetCastShadows(Actor.ID, value);
		}

		AssetHandle GetMaterialHandle(uint submeshIndex)
		{
			return InternalCalls.StaticMeshRendererComponent_GetMaterialHandle(submeshIndex, Actor.ID, out AssetHandle materialHandle)
				? materialHandle : AssetHandle.Invalid;
		}
	}

	public class SpriteRenderer: Component
	{
		public Texture2D Texture
		{
			get
			{
				return InternalCalls.SpriteRendererComponent_GetTextureHandle(Actor.ID, out AssetHandle textureHandle)
					? new Texture2D(textureHandle) : null;
			}

			set => InternalCalls.SpriteRendererComponent_SetTextureHandle(Actor.ID, ref value.m_Handle);
		}

		public Vector4 Color
		{
			get
			{
				InternalCalls.SpriteRendererComponent_GetColor(Actor.ID, out Vector4 color);
				return color;
			}

			set => InternalCalls.SpriteRendererComponent_SetColor(Actor.ID, ref value);
		}

		public Vector2 Scale
		{
			get
			{
				InternalCalls.SpriteRendererComponent_GetUV(Actor.ID, out Vector2 scale);
				return scale;
			}

			set => InternalCalls.SpriteRendererComponent_SetUV(Actor.ID, ref value);
		}

		public bool Visible
		{
			get => InternalCalls.SpriteRendererComponent_IsVisible(Actor.ID);
			set => InternalCalls.SpriteRendererComponent_SetVisible(Actor.ID, value);
		}
	}

	public class CircleRenderer: Component
	{
		public Vector4 Color
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetColor(Actor.ID, out Vector4 color);
				return color;
			}

			set => InternalCalls.CircleRendererComponent_SetColor(Actor.ID, ref value);
		}

		public float Thickness
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetThickness(Actor.ID, out float thickness);
				return thickness;
			}

			set => InternalCalls.CircleRendererComponent_SetThickness(Actor.ID, value);
		}

		public float Fade
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetFade(Actor.ID, out float fade);
				return fade;
			}

			set => InternalCalls.CircleRendererComponent_SetFade(Actor.ID, value);
		}

		public bool Visible
		{
			get => InternalCalls.CircleRendererComponent_IsVisible(Actor.ID);
			set => InternalCalls.CircleRendererComponent_SetVisible(Actor.ID, value);
		}
	}

	public class AudioSource : Component
	{
		public AudioClip Clip => new AudioClip(this);

		public Vector3 Position
		{
			get
			{
				InternalCalls.AudioSourceComponent_GetPosition(Actor.ID, out Vector3 position);
				return position;
			}

			set => InternalCalls.AudioSourceComponent_SetPosition(Actor.ID, ref value);
		}

		public Vector3 Direction
		{
			get
			{
				InternalCalls.AudioSourceComponent_GetDirection(Actor.ID, out Vector3 direction);
				return direction;
			}

			set => InternalCalls.AudioSourceComponent_SetDirection(Actor.ID, ref value);
		}

		public Vector3 Velocity
		{
			get
			{
				InternalCalls.AudioSourceComponent_GetVelocity(Actor.ID, out Vector3 veloctiy);
				return veloctiy;
			}

			set => InternalCalls.AudioSourceComponent_SetVelocity(Actor.ID, ref value);
		}

		public AudioCone<AudioSource> Cone => new AudioCone<AudioSource>(this);

		public float MinGain
		{
			get => InternalCalls.AudioSourceComponent_GetMinGain(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetMinGain(Actor.ID, value);
		}

		public float MaxGain
		{
			get => InternalCalls.AudioSourceComponent_GetMaxGain(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetMaxGain(Actor.ID, value);
		}

		public float DirectionalAttenuationFactor
		{
			get => InternalCalls.AudioSourceComponent_GetDirectionalAttenuationFactor(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetDirectionalAttenuationFactor(Actor.ID, value);
		}

		public AttenuationMode AttenuationModel
		{
			get => InternalCalls.AudioSourceComponent_GetAttenuationModel(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetAttenuationModel(Actor.ID, value);
		}

		public float Pan
		{
			get => InternalCalls.AudioSourceComponent_GetPan(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetPan(Actor.ID, value);
		}

		public PanMode PanModel
		{
			get => InternalCalls.AudioSourceComponent_GetPanMode(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetPanMode(Actor.ID, value);
		}

		public PositioningMode PositioningModel
		{
			get => InternalCalls.AudioSourceComponent_GetPositioningMode(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetPositioningMode(Actor.ID, value);
		}

		public float Falloff
		{
			get => InternalCalls.AudioSourceComponent_GetFalloff(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetFalloff(Actor.ID, value);
		}

		public float MinDistance
		{
			get => InternalCalls.AudioSourceComponent_GetMinDistance(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetMinDistance(Actor.ID, value);
		}

		public float MaxDistance
		{
			get => InternalCalls.AudioSourceComponent_GetMaxDistance(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetMaxDistance(Actor.ID, value);
		}

		public float Pitch
		{
			get => InternalCalls.AudioSourceComponent_GetPitch(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetPitch(Actor.ID, value);
		}

		public float DopplerFactor
		{
			get => InternalCalls.AudioSourceComponent_GetDopplerFactor(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetDopplerFactor(Actor.ID, value);
		}

		public float Volume
		{
			get => InternalCalls.AudioSourceComponent_GetVolume(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetVolume(Actor.ID, value);
		}

		public bool PlayOnStart
		{
			get => InternalCalls.AudioSourceComponent_GetPlayOnStart(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetPlayOnStart(Actor.ID, value);
		}

		public Vector3 DirectionToListener
		{
			get
			{
				InternalCalls.AudioSourceComponent_GetDirectionToListener(Actor.ID, out Vector3 result);
				return result;
			}
		}

		public bool IsSpacialized
		{
			get => InternalCalls.AudioSourceComponent_GetIsSpacialized(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetIsSpacialized(Actor.ID, value);
		}

		public bool IsLooping
		{
			get => InternalCalls.AudioSourceComponent_GetIsLooping(Actor.ID);
			set => InternalCalls.AudioSourceComponent_SetIsLooping(Actor.ID, value);
		}

		public float CurrentFadeVolume => InternalCalls.AudioSourceComponent_GetCurrentFadeVolume(Actor.ID);

		public bool IsPlaying => InternalCalls.AudioSourceComponent_GetIsPlaying(Actor.ID);
		public bool IsPaused => InternalCalls.AudioSourceComponent_GetIsPaused(Actor.ID);

		public void Play() => InternalCalls.AudioSourceComponent_Play(Actor.ID);

		public void SetStartTime(ulong delay, TimeMeasure measure = TimeMeasure.Seconds)
		{
			switch (measure)
			{
				case TimeMeasure.Milliseconds: InternalCalls.AudioSourceComponent_SetStartTimeInMilliseconds(Actor.ID, delay);        break;
				case TimeMeasure.Seconds:      InternalCalls.AudioSourceComponent_SetStartTimeInMilliseconds(Actor.ID, delay * 1000); break;
				case TimeMeasure.PCMFrames:    InternalCalls.AudioSourceComponent_SetStartTimeInPCMFrames(Actor.ID, delay);           break;
			}
		}

		public void SetFadeIn(float volumeStart, float volumeEnd, ulong length, TimeMeasure measure = TimeMeasure.Seconds)
		{
			switch (measure)
			{
				case TimeMeasure.Milliseconds: InternalCalls.AudioSourceComponent_SetFadeInMilliseconds(Actor.ID, volumeStart, volumeEnd, length);        break;
				case TimeMeasure.Seconds:      InternalCalls.AudioSourceComponent_SetFadeInMilliseconds(Actor.ID, volumeStart, volumeEnd, length * 1000); break;
				case TimeMeasure.PCMFrames:    InternalCalls.AudioSourceComponent_SetFadeInPCMFrames(Actor.ID, volumeStart, volumeEnd, length);           break;
			}
		}
		
		public void SetFadeStart(float volumeStart, float volumeEnd, ulong length, ulong time, TimeMeasure measure = TimeMeasure.Seconds)
		{
			switch (measure)
			{
				case TimeMeasure.Milliseconds: InternalCalls.AudioSourceComponent_SetFadeStartInMilliseconds(Actor.ID, volumeStart, volumeEnd, length, time);        break;
				case TimeMeasure.Seconds:      InternalCalls.AudioSourceComponent_SetFadeStartInMilliseconds(Actor.ID, volumeStart, volumeEnd, length * 1000, time); break;
				case TimeMeasure.PCMFrames:    InternalCalls.AudioSourceComponent_SetFadeStartInPCMFrames(Actor.ID, volumeStart, volumeEnd, length, time);           break;
			}
		}

		public void PlayOneShot() => InternalCalls.AudioSourceComponent_PlayOneShot(Actor.ID);
		
		public void Pause() => InternalCalls.AudioSourceComponent_Pause(Actor.ID);
		public void Restart() => InternalCalls.AudioSourceComponent_Restart(Actor.ID);
		public void Stop() => InternalCalls.AudioSourceComponent_Stop(Actor.ID);

		public void SetStopTime(ulong delay, TimeMeasure measure = TimeMeasure.Seconds)
		{
			switch (measure)
			{
				case TimeMeasure.Milliseconds: InternalCalls.AudioSourceComponent_SetStopTimeInMilliseconds(Actor.ID, delay);        break;
				case TimeMeasure.Seconds:      InternalCalls.AudioSourceComponent_SetStopTimeInMilliseconds(Actor.ID, delay * 1000); break;
				case TimeMeasure.PCMFrames:    InternalCalls.AudioSourceComponent_SetStopTimeInMilliseconds(Actor.ID, delay);        break;
			}
		}

		public void SetStopTimeWithFade(ulong stopTime, ulong fadeLength, TimeMeasure measure = TimeMeasure.Seconds)
		{
			switch (measure)
			{
				case TimeMeasure.Milliseconds: InternalCalls.AudioSourceComponent_SetStopTimeWithFadeInMilliseconds(Actor.ID, stopTime, fadeLength); break;
				case TimeMeasure.Seconds:      InternalCalls.AudioSourceComponent_SetStopTimeWithFadeInMilliseconds(Actor.ID, stopTime, fadeLength); break;
				case TimeMeasure.PCMFrames:    InternalCalls.AudioSourceComponent_SetStopTimeWithFadeInPCMFrames(Actor.ID, stopTime, fadeLength);    break;
			}
		}

		public bool Seek(ulong frameIndex) => InternalCalls.AudioSourceComponent_SeekToPCMFrame(Actor.ID, frameIndex);
	}

	public class AudioListener: Component
	{
		public AudioCone<AudioListener> Cone => new AudioCone<AudioListener>(this);
	}

	public class RigidBody: Component
	{
		public RigidBodyType BodyType
		{
			get => InternalCalls.RigidBodyComponent_GetBodyType(Actor.ID);
			set => InternalCalls.RigidBodyComponent_SetBodyType(Actor.ID, value);
		}

		public CollisionDetectionType CollisionDetection
		{
			get => InternalCalls.RigidBodyComponent_GetCollisionDetectionType(Actor.ID);
			set => InternalCalls.RigidBodyComponent_SetCollisionDetectionType(Actor.ID, value);
		}

		public float Mass
		{
			get => InternalCalls.RigidBodyComponent_GetMass(Actor.ID);
			set => InternalCalls.RigidBodyComponent_SetMass(Actor.ID, value);
		}

		public Vector3 Velocity
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetLinearVelocity(Actor.ID, out Vector3 velocity);
				return velocity;
			}
			set => InternalCalls.RigidBodyComponent_SetLinearVelocity(Actor.ID, ref value);
		}

		public float MaxVelocity
		{
			get => InternalCalls.RigidBodyComponent_GetMaxLinearVelocity(Actor.ID);
			set => InternalCalls.RigidBodyComponent_SetMaxLinearVelocity(Actor.ID, value);
		}

		public float Drag
		{
			get =>InternalCalls.RigidBodyComponent_GetLinearDrag(Actor.ID);
			set => InternalCalls.RigidBodyComponent_SetLinearDrag(Actor.ID, value);
		}

		public Vector3 AngularVelocity
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetAngularVelocity(Actor.ID, out Vector3 velocity);
				return velocity;
			}
			set => InternalCalls.RigidBodyComponent_SetAngularVelocity(Actor.ID, ref value);
		}

		public float MaxAngularVelocity
		{
			get => InternalCalls.RigidBodyComponent_GetMaxAngularVelocity(Actor.ID);
			set => InternalCalls.RigidBodyComponent_SetMaxAngularVelocity(Actor.ID, value);
		}

		public float AngularDrag
		{
			get => InternalCalls.RigidBodyComponent_GetAngularDrag(Actor.ID);
			set => InternalCalls.RigidBodyComponent_SetAngularDrag(Actor.ID, value);
		}

		public bool DisableGravity
		{
			get => InternalCalls.RigidBodyComponent_GetDisableGravity(Actor.ID);
			set => InternalCalls.RigidBodyComponent_SetDisableGravity(Actor.ID, value);
		}

		public bool IsKinematic
		{
			get => InternalCalls.RigidBodyComponent_GetIsKinematic(Actor.ID);
			set => InternalCalls.RigidBodyComponent_SetIsKinematic(Actor.ID, value);
		}

		public Vector3 KinematicTarget
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetKinematicTargetTranslation(Actor.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.RigidBodyComponent_SetKinematicTargetTranslation(Actor.ID, ref value);
		}

		public Quaternion KinematicTargetRotation
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetKinematicTargetRotation(Actor.ID, out Quaternion result);
				return result;
			}

			set => InternalCalls.RigidBodyComponent_SetKinematicTargetRotation(Actor.ID, ref value);
		}

		public uint GetLockFlags() => InternalCalls.RigidBodyComponent_GetLockFlags(Actor.ID);
		
		public void SetLockFlag(ActorLockFlag flag, bool value, bool forceWake = false)
		{
			InternalCalls.RigidBodyComponent_SetLockFlag(Actor.ID, flag, value, forceWake);
		}

		public bool IsLockFlagSet(ActorLockFlag flag)
		{
			return InternalCalls.RigidBodyComponent_IsLockFlagSet(Actor.ID, flag);
		}

		public bool IsSleeping => InternalCalls.RigidBodyComponent_IsSleeping(Actor.ID);
		public void WakeUp() => InternalCalls.RigidBodyComponent_WakeUp(Actor.ID);

		public void AddForce(Vector3 force, ForceMode forceMode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_AddForce(Actor.ID, ref force, forceMode);
		}
		
		public void AddForceAtPosition(Vector3 force, Vector3 position, ForceMode forceMode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_AddForceAtPosition(Actor.ID, ref force, ref position, forceMode);
		}

		public void AddTorque(Vector3 torque, ForceMode forceMode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_AddTorque(Actor.ID, ref torque, forceMode);
		}

		public void ClearTorque(ForceMode mode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_ClearTorque(Actor.ID, mode);
		}

		public void ClearForce(ForceMode mode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_ClearForce(Actor.ID, mode);
		}
	}

	public class CharacterController: Component
	{
		public float SpeedDown => InternalCalls.CharacterControllerComponent_GetSpeedDown(Actor.ID);
		public bool IsGrounded => InternalCalls.CharacterControllerComponent_IsGrounded(Actor.ID);

		public Vector3 FootPosition
		{
			get
			{
				InternalCalls.CharacterControllerComponent_GetFootPosition(Actor.ID, out Vector3 footPosition);
				return footPosition;
			}
		}

		public float SlopeLimit
		{
			get => InternalCalls.CharacterControllerComponent_GetSlopeLimit(Actor.ID);
			set => InternalCalls.CharacterControllerComponent_SetSlopeLimit(Actor.ID, value);
		}

		public float StepOffset
		{
			get => InternalCalls.CharacterControllerComponent_GetStepOffset(Actor.ID);
			set => InternalCalls.CharacterControllerComponent_SetStepOffset(Actor.ID, value);
		}

		public float ContactOffset
		{
			get => InternalCalls.CharacterControllerComponent_GetContactOffset(Actor.ID);
			set => InternalCalls.CharacterControllerComponent_SetContactOffset(Actor.ID, value);
		}

		public bool DisableGravity
		{
			get => InternalCalls.CharacterControllerComponent_GetDisableGravity(Actor.ID);
			set => InternalCalls.CharacterControllerComponent_SetDisableGravity(Actor.ID, value);
		}

		public NonWalkableMode NonWalkMode
		{
			get => InternalCalls.CharacterControllerComponent_GetNonWalkableMode(Actor.ID);
			set => InternalCalls.CharacterControllerComponent_SetNonWalkableMode(Actor.ID, value);
		}

		public CapsuleClimbMode ClimbMode
		{
			get => InternalCalls.CharacterControllerComponent_GetClimbMode(Actor.ID);
			set => InternalCalls.CharacterControllerComponent_SetClimbMode(Actor.ID, value);
		}

		public void Move(Vector3 displacement)
		{
			InternalCalls.CharacterControllerComponent_Move(Actor.ID, ref displacement);
		}

		public void Jump(float jumpForce)
		{
			InternalCalls.CharacterControllerComponent_Jump(Actor.ID, jumpForce);
		}
	}

	public class FixedJoint: Component
	{
		public Actor ConnectedActor
		{
			get
			{
				ulong actorID = InternalCalls.FixedJointComponent_GetConnectedActor(Actor.ID);

				if (actorID != 0)
				{
					return new Actor(actorID);
				}

				return null;
			}

			set => InternalCalls.FixedJointComponent_SetConnectedActor(Actor.ID, value.ID);
		}

		public float BreakForce
		{
			get => InternalCalls.FixedJointComponent_GetBreakForce(Actor.ID);
			set => InternalCalls.FixedJointComponent_SetBreakForce(Actor.ID, value);
		}

		public float BreakTorque
		{
			get => InternalCalls.FixedJointComponent_GetBreakTorque(Actor.ID);
			set => InternalCalls.FixedJointComponent_SetBreakTorque(Actor.ID, value);
		}

		public void SetBreakForceAndTorque(float breakForce, float breakTorque)
		{
			InternalCalls.FixedJointComponent_SetBreakForceAndTorque(Actor.ID, breakForce, breakTorque);
		}

		public bool IsCollisionEnabled
		{
			get => InternalCalls.FixedJointComponent_GetEnableCollision(Actor.ID);
			set => InternalCalls.FixedJointComponent_SetCollisionEnabled(Actor.ID, value);
		}

		public bool IsPreProcessingEnabled
		{
			get => InternalCalls.FixedJointComponent_GetPreProcessingEnabled(Actor.ID);
			set => InternalCalls.FixedJointComponent_SetPreProcessingEnabled(Actor.ID, value);
		}

		public bool IsBroken => InternalCalls.FixedJointComponent_IsBroken(Actor.ID);

		public bool IsBreakable
		{
			get => InternalCalls.FixedJointComponent_GetIsBreakable(Actor.ID);
			set => InternalCalls.FixedJointComponent_SetIsBreakable(Actor.ID, value);
		}

		public void Break() => InternalCalls.FixedJointComponent_Break(Actor.ID);
	}

	public class BoxCollider: Component
	{
		public Vector3 HalfSize
		{
			get
			{
				InternalCalls.BoxColliderComponent_GetHalfSize(Actor.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.BoxColliderComponent_SetHalfSize(Actor.ID, ref value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.BoxColliderComponent_GetOffset(Actor.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.BoxColliderComponent_SetOffset(Actor.ID, ref value);
		}

		public bool IsTrigger
		{
			get => InternalCalls.BoxColliderComponent_GetIsTrigger(Actor.ID);
			set => InternalCalls.BoxColliderComponent_SetIsTrigger(Actor.ID, value);
		}

		public PhysicsMaterial Material
		{
			get
			{
				return InternalCalls.BoxColliderComponent_GetMaterialHandle(Actor.ID, out AssetHandle materialHandle)
					? new PhysicsMaterial(materialHandle) : null;
			}
		}
	}

	public class SphereCollider: Component
	{
		public float Radius
		{
			get => InternalCalls.SphereColliderComponent_GetRadius(Actor.ID);
			set => InternalCalls.SphereColliderComponent_SetRadius(Actor.ID, value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.SphereColliderComponent_GetOffset(Actor.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.SphereColliderComponent_SetOffset(Actor.ID, ref value);
		}

		public bool IsTrigger
		{
			get => InternalCalls.SphereColliderComponent_GetIsTrigger(Actor.ID);
			set => InternalCalls.SphereColliderComponent_SetIsTrigger(Actor.ID, value);
		}

		public PhysicsMaterial Material
		{
			get
			{
				return InternalCalls.SphereColliderComponent_GetMaterialHandle(Actor.ID, out AssetHandle materialHandle)
					? new PhysicsMaterial(materialHandle) : null;
			}
		}
	}

	public class CapsuleCollider: Component
	{
		public float Radius
		{
			get => InternalCalls.CapsuleColliderComponent_GetRadius(Actor.ID);
			set => InternalCalls.CapsuleColliderComponent_SetRadius(Actor.ID, value);
		}
		
		public float Height
		{
			get => InternalCalls.CapsuleColliderComponent_GetHeight(Actor.ID);
			set => InternalCalls.CapsuleColliderComponent_SetHeight(Actor.ID, value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.CapsuleColliderComponent_GetOffset(Actor.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.CapsuleColliderComponent_SetOffset(Actor.ID, ref value);
		}

		public bool IsTrigger
		{
			get => InternalCalls.CapsuleColliderComponent_GetIsTrigger(Actor.ID);
			set => InternalCalls.CapsuleColliderComponent_SetIsTrigger(Actor.ID, value);
		}

		public PhysicsMaterial Material
		{
			get
			{
				return InternalCalls.CapsuleColliderComponent_GetMaterialHandle(Actor.ID, out AssetHandle materialHandle)
					? new PhysicsMaterial(materialHandle) : null;
			}
		}
	}

	public class MeshCollider: Component
	{
		public bool IsStaticMesh => InternalCalls.MeshColliderComponent_IsStaticMesh(Actor.ID);

		public AssetHandle ColliderMeshHandle
		{
			get => InternalCalls.MeshColliderComponent_GetColliderMesh(Actor.ID, out AssetHandle colliderHandle)
				? colliderHandle : AssetHandle.Invalid;
		}

		public bool IsTrigger
		{
			get => InternalCalls.MeshColliderComponent_GetIsTrigger(Actor.ID);
			set => InternalCalls.MeshColliderComponent_SetIsTrigger(Actor.ID, value);
		}

		public PhysicsMaterial Material
		{
			get => InternalCalls.MeshColliderComponent_GetMaterialHandle(Actor.ID, out AssetHandle materialHandle)
				? new PhysicsMaterial(materialHandle) : null;
		}
	}

	public class RigidBody2D: Component
	{
		public RigidBody2DType BodyType
		{
			get => InternalCalls.RigidBody2DComponent_GetBodyType(Actor.ID);
			set => InternalCalls.RigidBody2DComponent_SetBodyType(Actor.ID, value);
		}

		public Vector2 Velocity
		{
			get
			{
				InternalCalls.RigidBody2DComponent_GetVelocity(Actor.ID, out Vector2 velocity);
				return velocity;
			}
			set => InternalCalls.RigidBody2DComponent_SetVelocity(Actor.ID, ref value);
		}

		public float Drag
		{
			get => InternalCalls.RigidBody2DComponent_GetDrag(Actor.ID);
			set => InternalCalls.RigidBody2DComponent_SetDrag(Actor.ID, value);
		}

		public float AngularVelocity
		{
			get => InternalCalls.RigidBody2DComponent_GetAngularVelocity(Actor.ID);
			set => InternalCalls.RigidBody2DComponent_SetAngularVelocity(Actor.ID, value);
		}

		public float AngularDrag
		{
			get => InternalCalls.RigidBody2DComponent_GetAngularDrag(Actor.ID);
			set => InternalCalls.RigidBody2DComponent_SetAngularDrag(Actor.ID, value);
		}

		public float GravityScale
		{
			get => InternalCalls.RigidBody2DComponent_GetGravityScale(Actor.ID);
			set => InternalCalls.RigidBody2DComponent_SetGravityScale(Actor.ID, value);
		}

		public bool FreezeRotation
		{
			get => InternalCalls.RigidBody2DComponent_GetFixedRotation(Actor.ID);
			set => InternalCalls.RigidBody2DComponent_SetFixedRotation(Actor.ID, value);
		}

		public void ApplyForce(Vector2 force, Vector2 worldPosition, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyForce(Actor.ID, ref force, ref worldPosition, wake);
		}

		public void ApplyForce(Vector2 force, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyForceToCenter(Actor.ID, ref force, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyLinearImpulse(Actor.ID, ref impulse, ref worldPosition, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyLinearImpulseToCenter(Actor.ID, ref impulse, wake);
		}
	}

	public class BoxCollider2D: Component
	{
		public Vector2 Offset
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetOffset(Actor.ID, out Vector2 result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetOffset(Actor.ID, ref value);
		}

		public Vector2 Size
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetSize(Actor.ID, out Vector2 result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetSize(Actor.ID, ref value);
		}

		public float Density
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetDensity(Actor.ID, out float result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetDensity(Actor.ID, value);
		}

		public float Friction
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetFriction(Actor.ID, out float result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetFriction(Actor.ID, value);
		}

		public float Restitution
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetRestitution(Actor.ID, out float result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetRestitution(Actor.ID, value);
		}

		public float RestitutionThreshold
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetRestitutionThreshold(Actor.ID, out float result);
				return result;
			}

			set => InternalCalls.BoxCollider2DComponent_SetRestitutionThreshold(Actor.ID, value);
		}
	}

	public class CircleCollider2D: Component
	{
		public Vector2 Offset
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetOffset(Actor.ID, out Vector2 result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetOffset(Actor.ID, ref value);
		}

		public float Radius
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetRadius(Actor.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetRadius(Actor.ID, value);
		}

		public float Density
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetDensity(Actor.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetDensity(Actor.ID, value);
		}

		public float Friction
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetFriction(Actor.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetFriction(Actor.ID, value);
		}

		public float Restitution
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetRestitution(Actor.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetRestitution(Actor.ID, value);
		}

		public float RestitutionThreshold
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetRestitutionThreshold(Actor.ID, out float result);
				return result;
			}

			set => InternalCalls.CircleCollider2DComponent_SetRestitutionThreshold(Actor.ID, value);
		}
	}

	public class ParticleEmitter: Component
	{
		public Vector3 Velocity
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetVelocity(Actor.ID, out Vector3 velocity);
				return velocity;
			}

			set => InternalCalls.ParticleEmitterComponent_SetVelocity(Actor.ID, ref value);
		}

		public Vector3 VelocityVariation
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetVelocityVariation(Actor.ID, out Vector3 velocityVariation);
				return velocityVariation;
			}

			set => InternalCalls.ParticleEmitterComponent_SetVelocityVariation(Actor.ID, ref value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetOffset(Actor.ID, out Vector3 offset);
				return offset;
			}

			set => InternalCalls.ParticleEmitterComponent_SetOffset(Actor.ID, ref value);
		}

		public Vector2 SizeBegin
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetSizeBegin(Actor.ID, out Vector2 sizeBegin);
				return sizeBegin;
			}

			set => InternalCalls.ParticleEmitterComponent_SetSizeBegin(Actor.ID, ref value);
		}

		public Vector2 SizeEnd
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetSizeEnd(Actor.ID, out Vector2 sizeEnd);
				return sizeEnd;
			}

			set => InternalCalls.ParticleEmitterComponent_SetSizeEnd(Actor.ID, ref value);
		}

		public Vector2 SizeVariation
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetSizeVariation(Actor.ID, out Vector2 sizeVariation);
				return sizeVariation;
			}

			set => InternalCalls.ParticleEmitterComponent_SetSizeVariation(Actor.ID, ref value);
		}

		public Vector4 ColorBegin
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetColorBegin(Actor.ID, out Vector4 colorBegin);
				return colorBegin;
			}

			set => InternalCalls.ParticleEmitterComponent_SetColorBegin(Actor.ID, ref value);
		}

		public Vector4 ColorEnd
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetColorEnd(Actor.ID, out Vector4 colorEnd);
				return colorEnd;
			}

			set => InternalCalls.ParticleEmitterComponent_SetColorEnd(Actor.ID, ref value);
		}

		public float Rotation
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetRotation(Actor.ID, out float rotation);
				return rotation;
			}

			set => InternalCalls.ParticleEmitterComponent_SetRotation(Actor.ID, value);
		}

		public float LifeTime
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetLifeTime(Actor.ID, out float lifetime);
				return lifetime;
			}

			set => InternalCalls.ParticleEmitterComponent_SetLifeTime(Actor.ID, value);
		}

		public void Start() => InternalCalls.ParticleEmitterComponent_Start(Actor.ID);
		public void Stop() => InternalCalls.ParticleEmitterComponent_Stop(Actor.ID);

		public bool IsActive => InternalCalls.ParticleEmitterComponent_IsActive(Actor.ID);
	}

	public class Script: Component
	{
		public object Instance => InternalCalls.Actor_GetScriptInstance(Actor.ID);

		public bool Is<T>()
			where T : Actor, new()
		{
			return Instance is T;
		}

		public T As<T>()
			where T : Actor, new()
		{
			return Instance as T;
		}
	}

}
