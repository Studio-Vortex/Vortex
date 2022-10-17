﻿namespace Sparky
{
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

	public class MeshRenderer : Component
	{
		public Vector4 Color
		{
			get
			{
				InternalCalls.MeshRendererComponent_GetColor(Entity.ID, out Vector4 color);
				return color;
			}

			set
			{
				InternalCalls.MeshRendererComponent_SetColor(Entity.ID, ref value);
			}
		}

		public string Texture
		{
			get
			{
				InternalCalls.MeshRendererComponent_GetTexture(Entity.ID, out string texturePath);
				return texturePath;
			}

			set
			{
				InternalCalls.MeshRendererComponent_SetTexture(Entity.ID, value);
			}
		}

		public float Scale
		{
			get
			{
				InternalCalls.MeshRendererComponent_GetScale(Entity.ID, out float scale);
				return scale;
			}

			set
			{
				InternalCalls.MeshRendererComponent_SetScale(Entity.ID, value);
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
				InternalCalls.SpriteRendererComponent_GetTexture(Entity.ID, out string texturePath);
				return texturePath;
			}

			set
			{
				InternalCalls.SpriteRendererComponent_SetTexture(Entity.ID, value);
			}
		}

		public float Scale
		{
			get
			{
				InternalCalls.SpriteRendererComponent_GetScale(Entity.ID, out float scale);
				return scale;
			}

			set
			{
				InternalCalls.SpriteRendererComponent_SetScale(Entity.ID, value);
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

		public void Play()
		{
			InternalCalls.AudioSourceComponent_Play(Entity.ID);
		}

		public void Stop()
		{
			InternalCalls.AudioSourceComponent_Stop(Entity.ID);
		}
	}

	public class RigidBody2D : Component
	{
		public enum BodyType
		{
			Static, Dynamic, Kinematic,
		}

		public BodyType Type
		{
			get
			{
				InternalCalls.RigidBody2DComponent_GetBodyType(Entity.ID, out BodyType bodyType);
				return bodyType;
			}

			set
			{
				InternalCalls.RigidBody2DComponent_SetBodyType(Entity.ID, value);
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

}
