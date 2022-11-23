namespace Sparky {

	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class Transform : Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
				return translation;
			}

			set
			{
				InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
			}
		}

		public void Translate(Vector3 translation)
		{
			Translation += translation;
		}

		public void Translate(float x, float y, float z)
		{
			Translation += new Vector3(x, y, z);
		}

		public Vector3 Scale
		{
			get
			{
				InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
				return scale;
			}

			set
			{
				InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
			}
		}

		public Vector3 Forward
		{
			get
			{
				InternalCalls.TransformComponent_GetForwardDirection(Entity.ID, out Vector3 forward);
				return forward;
			}
		}

		public Vector3 Right
		{
			get
			{
				InternalCalls.TransformComponent_GetRightDirection(Entity.ID, out Vector3 right);
				return right;
			}
		}

		public Vector3 Up
		{
			get
			{
				InternalCalls.TransformComponent_GetUpDirection(Entity.ID, out Vector3 up);
				return up;
			}
		}

		/// <summary>
		/// Sets the scale of the entity
		/// </summary>
		/// <param name="scale">the new scale</param>
		public void LocalScale(Vector3 scale)
		{
			Scale = scale;
		}

		/// <summary>
		/// Sets the scale of the entity
		/// </summary>
		/// <param name="x">the new x scale</param>
		/// <param name="y">the new y scale</param>
		/// <param name="z">the new z scale</param>
		public void LocalScale(float x, float y, float z)
		{
			Scale = new Vector3(x, y, z);
		}

		/// <summary>
		/// Applys the given scale to the current entity's scale
		/// </summary>
		/// <param name="scale">the scale to be applied</param>
		public void ApplyScale(Vector3 scale)
		{
			Scale += scale;
		}

		/// <summary>
		/// Applys the given scale to the current entity's scale
		/// </summary>
		/// <param name="x">the x scale to be applied</param>
		/// <param name="y">the y scale to be applied</param>
		/// <param name="z">the z scale to be applied</param>
		public void ApplyScale(float x, float y, float z)
		{
			Scale += new Vector3(x, y, z);
		}

		public Vector3 Rotation
		{
			get
			{
				InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 rotation);
				return rotation;
			}

			set
			{
				InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
			}
		}

		public void Rotate(Vector3 rotation)
		{
			Rotation += rotation;
		}

		public void Rotate(float x, float y, float z)
		{
			Rotation += new Vector3(x, y, z);
		}

		public void LookAt(Vector3 worldPoint)
		{
			InternalCalls.TransformComponent_LookAt(Entity.ID, ref worldPoint);
		}
	}

	public class Camera : Component
	{
		public bool Primary
		{
			get
			{
				InternalCalls.CameraComponent_GetPrimary(Entity.ID, out bool primary);
				return primary;
			}

			set
			{
				InternalCalls.CameraComponent_SetPrimary(Entity.ID, value);
			}
		}

		public bool FixedAspectRatio
		{
			get
			{
				InternalCalls.CameraComponent_GetFixedAspectRatio(Entity.ID, out bool fixedAspectRatio);
				return fixedAspectRatio;
			}

			set
			{
				InternalCalls.CameraComponent_SetFixedAspectRatio(Entity.ID, value);
			}
		}
	}

	public class LightSource : Component
	{
		public LightType Type;

		public Vector3 Ambient
		{
			get
			{
				InternalCalls.LightSourceComponent_GetAmbient(Entity.ID, out Vector3 ambient);
				return ambient;
			}

			set
			{
				InternalCalls.LightSourceComponent_SetAmbient(Entity.ID, ref value);
			}
		}

		public Vector3 Diffuse
		{
			get
			{
				InternalCalls.LightSourceComponent_GetDiffuse(Entity.ID, out Vector3 diffuse);
				return diffuse;
			}

			set
			{
				InternalCalls.LightSourceComponent_SetDiffuse(Entity.ID, ref value);
			}
		}

		public Vector3 Specular
		{
			get
			{
				InternalCalls.LightSourceComponent_GetSpecular(Entity.ID, out Vector3 specular);
				return specular;
			}

			set
			{
				InternalCalls.LightSourceComponent_SetSpecular(Entity.ID, ref value);
			}
		}

		public Vector3 Color
		{
			get
			{
				InternalCalls.LightSourceComponent_GetColor(Entity.ID, out Vector3 color);
				return color;
			}

			set
			{
				InternalCalls.LightSourceComponent_SetColor(Entity.ID, ref value);
			}
		}

		public Vector3 Direction
		{
			get
			{
				InternalCalls.LightSourceComponent_GetDirection(Entity.ID, out Vector3 direction);
				return direction;
			}
			set
			{
				InternalCalls.LightSourceComponent_SetDirection(Entity.ID, ref value);
			}
		}
	}

	public class MeshRenderer : Component
	{
		public MeshType Type;

		public Vector2 Scale
		{
			get
			{
				InternalCalls.MeshRendererComponent_GetScale(Entity.ID, out Vector2 scale);
				return scale;
			}

			set
			{
				InternalCalls.MeshRendererComponent_SetScale(Entity.ID, ref value);
			}
		}
	}

	public class SpriteRenderer: Component
	{
		public Vector4 Color
		{
			get
			{
				InternalCalls.SpriteRendererComponent_GetColor(Entity.ID, out Vector4 color);
				return color;
			}

			set
			{
				InternalCalls.SpriteRendererComponent_SetColor(Entity.ID, ref value);
			}
		}

		public string Texture
		{
			get
			{
				return InternalCalls.SpriteRendererComponent_GetTexture(Entity.ID);
			}

			set
			{
				InternalCalls.SpriteRendererComponent_SetTexture(Entity.ID, value);
			}
		}

		public Vector2 Scale
		{
			get
			{
				InternalCalls.SpriteRendererComponent_GetScale(Entity.ID, out Vector2 scale);
				return scale;
			}

			set
			{
				InternalCalls.SpriteRendererComponent_SetScale(Entity.ID, ref value);
			}
		}
	}

	public class CircleRenderer : Component
	{
		public Vector4 Color
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetColor(Entity.ID, out Vector4 color);
				return color;
			}

			set
			{
				InternalCalls.CircleRendererComponent_SetColor(Entity.ID, ref value);
			}
		}

		public float Thickness
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetThickness(Entity.ID, out float thickness);
				return thickness;
			}

			set
			{
				InternalCalls.CircleRendererComponent_SetThickness(Entity.ID, value);
			}
		}

		public float Fade
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetFade(Entity.ID, out float fade);
				return fade;
			}

			set
			{
				InternalCalls.CircleRendererComponent_SetFade(Entity.ID, value);
			}
		}
	}

	public class AudioSource : Component
	{
		public string Source;
		public bool IsPlaying
		{
			get
			{
				return InternalCalls.AudioSourceComponent_GetIsPlaying(Entity.ID);
			}
		}

		public void Play()
		{
			InternalCalls.AudioSourceComponent_Play(Entity.ID);
		}

		public void Stop()
		{
			InternalCalls.AudioSourceComponent_Stop(Entity.ID);
		}
	}

	public class AudioListener : Component
	{
		
	}

	public class RigidBody : Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetTranslation(Entity.ID, out Vector3 result);
				return result;
			}
			set
			{
				InternalCalls.RigidBodyComponent_SetTranslation(Entity.ID, ref value);
			}
		}

		public Vector3 Rotation
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetRotation(Entity.ID, out Vector3 result);
				return result;
			}
			set
			{
				InternalCalls.RigidBodyComponent_SetRotation(Entity.ID, ref value);
			}
		}

		public RigidBodyType BodyType
		{
			get
			{
				return InternalCalls.RigidBodyComponent_GetBodyType(Entity.ID);
			}

			set
			{
				InternalCalls.RigidBodyComponent_SetBodyType(Entity.ID, value);
			}
		}

		public float Mass
		{
			get
			{
				return InternalCalls.RigidBodyComponent_GetMass(Entity.ID);
			}
			set
			{
				InternalCalls.RigidBodyComponent_SetMass(Entity.ID, value);
			}
		}

		public Vector3 Velocity
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetLinearVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.RigidBodyComponent_SetLinearVelocity(Entity.ID, ref value);
			}
		}

		public float Drag
		{
			get
			{
				return InternalCalls.RigidBodyComponent_GetLinearDrag(Entity.ID);
			}
			set
			{
				InternalCalls.RigidBodyComponent_SetLinearDrag(Entity.ID, value);
			}
		}

		public Vector3 AngularVelocity
		{
			get
			{
				InternalCalls.RigidBodyComponent_GetAngularVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.RigidBodyComponent_SetAngularVelocity(Entity.ID, ref value);
			}
		}

		public float AngularDrag
		{
			get
			{
				return InternalCalls.RigidBodyComponent_GetAngularDrag(Entity.ID);
			}
			set
			{
				InternalCalls.RigidBodyComponent_SetAngularDrag(Entity.ID, value);
			}
		}

		public bool DisableGravity
		{
			get
			{
				return InternalCalls.RigidBodyComponent_GetDisableGravity(Entity.ID);
			}
			set
			{
				InternalCalls.RigidBodyComponent_SetDisableGravity(Entity.ID, value);
			}
		}

		public bool IsKinematic
		{
			get
			{
				return InternalCalls.RigidBodyComponent_GetIsKinematic(Entity.ID);
			}
			set
			{
				InternalCalls.RigidBodyComponent_SetIsKinematic(Entity.ID, value);
			}
		}

		public void Translate(Vector3 translation)
		{
			InternalCalls.RigidBodyComponent_Translate(Entity.ID, ref translation);
		}

		public void Translate(float x, float y, float z) => Translate(new Vector3(x, y, z));

		public void Rotate(Vector3 rotation)
		{
			InternalCalls.RigidBodyComponent_Rotate(Entity.ID, ref rotation);
		}

		public void Rotate(float x, float y, float z) => Rotate(new Vector3(x, y, z));

		public void AddForce(Vector3 force, ForceMode forceMode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_AddForce(Entity.ID, ref force, forceMode);
		}

		public void AddTorque(Vector3 torque, ForceMode forceMode = ForceMode.Force)
		{
			InternalCalls.RigidBodyComponent_AddTorque(Entity.ID, ref torque, forceMode);
		}
	}

	public class CharacterController : Component
	{

		public void Move(Vector3 displacement)
		{
			InternalCalls.CharacterControllerComponent_Move(Entity.ID, ref displacement);
		}

		public void Jump(float force)
		{
			InternalCalls.CharacterControllerComponent_Jump(Entity.ID, force);
		}
	}

	public class PhysicsMaterial : Component
	{
		public float StaticFriction;
		public float DynamicFriction;
		public float Bounciness;
	}

	public class BoxCollider : Component
	{

	}

	public class SphereCollider : Component
	{

	}

	public class CapsuleCollider : Component
	{

	}

	public class StaticMeshCollider : Component
	{

	}

	public class RigidBody2D : Component
	{
		public RigidBody2DType BodyType
		{
			get
			{
				return InternalCalls.RigidBody2DComponent_GetBodyType(Entity.ID);
			}

			set
			{
				InternalCalls.RigidBody2DComponent_SetBodyType(Entity.ID, value);
			}
		}

		public Vector2 Velocity
		{
			get
			{
				InternalCalls.RigidBody2DComponent_GetVelocity(Entity.ID, out Vector2 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.RigidBody2DComponent_SetVelocity(Entity.ID, ref value);
			}
		}

		public float Drag
		{
			get
			{
				return InternalCalls.RigidBody2DComponent_GetDrag(Entity.ID);
			}
			set
			{
				InternalCalls.RigidBody2DComponent_SetDrag(Entity.ID, value);
			}
		}

		public bool FreezeRotation
		{
			get
			{
				return InternalCalls.RigidBody2DComponent_GetFixedRotation(Entity.ID);
			}
			set
			{
				InternalCalls.RigidBody2DComponent_SetFixedRotation(Entity.ID, value);
			}
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

	public class BoxCollider2D : Component
	{
		public Vector2 Offset
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetOffset(Entity.ID, out Vector2 result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2DComponent_SetOffset(Entity.ID, ref value);
			}
		}

		public Vector2 Size
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetSize(Entity.ID, out Vector2 result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2DComponent_SetSize(Entity.ID, ref value);
			}
		}

		public float Density
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetDensity(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2DComponent_SetDensity(Entity.ID, value);
			}
		}

		public float Friction
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetFriction(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2DComponent_SetFriction(Entity.ID, value);
			}
		}

		public float Restitution
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetRestitution(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2DComponent_SetRestitution(Entity.ID, value);
			}
		}

		public float RestitutionThreshold
		{
			get
			{
				InternalCalls.BoxCollider2DComponent_GetRestitutionThreshold(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2DComponent_SetRestitutionThreshold(Entity.ID, value);
			}
		}
	}

	public class CircleCollider2D : Component
	{
		public Vector2 Offset
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetOffset(Entity.ID, out Vector2 result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2DComponent_SetOffset(Entity.ID, ref value);
			}
		}

		public float Radius
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetRadius(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2DComponent_SetRadius(Entity.ID, value);
			}
		}

		public float Density
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetDensity(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2DComponent_SetDensity(Entity.ID, value);
			}
		}

		public float Friction
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetFriction(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2DComponent_SetFriction(Entity.ID, value);
			}
		}

		public float Restitution
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetRestitution(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2DComponent_SetRestitution(Entity.ID, value);
			}
		}

		public float RestitutionThreshold
		{
			get
			{
				InternalCalls.CircleCollider2DComponent_GetRestitutionThreshold(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2DComponent_SetRestitutionThreshold(Entity.ID, value);
			}
		}
	}

	public class ParticleEmitter : Component
	{
		public Vector3 Velocity
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetVelocity(Entity.ID, ref value);
			}
		}

		public Vector3 VelocityVariation
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetVelocityVariation(Entity.ID, out Vector3 velocityVariation);
				return velocityVariation;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetVelocityVariation(Entity.ID, ref value);
			}
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetOffset(Entity.ID, out Vector3 offset);
				return offset;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetOffset(Entity.ID, ref value);
			}
		}

		public Vector2 SizeBegin
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetSizeBegin(Entity.ID, out Vector2 sizeBegin);
				return sizeBegin;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetSizeBegin(Entity.ID, ref value);
			}
		}

		public Vector2 SizeEnd
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetSizeEnd(Entity.ID, out Vector2 sizeEnd);
				return sizeEnd;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetSizeEnd(Entity.ID, ref value);
			}
		}

		public Vector2 SizeVariation
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetSizeVariation(Entity.ID, out Vector2 sizeVariation);
				return sizeVariation;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetSizeVariation(Entity.ID, ref value);
			}
		}

		public Vector4 ColorBegin
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetColorBegin(Entity.ID, out Vector4 colorBegin);
				return colorBegin;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetColorBegin(Entity.ID, ref value);
			}
		}

		public Vector4 ColorEnd
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetColorEnd(Entity.ID, out Vector4 colorEnd);
				return colorEnd;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetColorEnd(Entity.ID, ref value);
			}
		}

		public float Rotation
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetRotation(Entity.ID, out float rotation);
				return rotation;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetRotation(Entity.ID, value);
			}
		}

		public float LifeTime
		{
			get
			{
				InternalCalls.ParticleEmitterComponent_GetLifeTime(Entity.ID, out float lifetime);
				return lifetime;
			}

			set
			{
				InternalCalls.ParticleEmitterComponent_SetLifeTime(Entity.ID, value);
			}
		}

		public void Start()
		{
			InternalCalls.ParticleEmitterComponent_Start(Entity.ID);
		}

		public void Stop()
		{
			InternalCalls.ParticleEmitterComponent_Stop(Entity.ID);
		}
	}

}
