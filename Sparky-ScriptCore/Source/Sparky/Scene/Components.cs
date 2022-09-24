namespace Sparky
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
	}

	public class Sprite : Component
	{
		public Vector4 Color
		{
			get
			{
				InternalCalls.SpriteComponent_GetColor(Entity.ID, out Vector4 color);
				return color;
			}

			set
			{
				InternalCalls.SpriteComponent_SetColor(Entity.ID, ref value);
			}
		}

		public float Scale
		{
			get
			{
				InternalCalls.SpriteComponent_GetScale(Entity.ID, out float scale);
				return scale;
			}

			set
			{
				InternalCalls.SpriteComponent_SetScale(Entity.ID, value);
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

	public class RigidBody2D : Component
	{
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
		public float Density
		{
			get
			{
				InternalCalls.BoxCollider2D_GetDensity(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2D_SetDensity(Entity.ID, value);
			}
		}

		public float Friction
		{
			get
			{
				InternalCalls.BoxCollider2D_GetFriction(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2D_SetFriction(Entity.ID, value);
			}
		}

		public float Restitution
		{
			get
			{
				InternalCalls.BoxCollider2D_GetRestitution(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2D_SetRestitution(Entity.ID, value);
			}
		}

		public float RestitutionThreshold
		{
			get
			{
				InternalCalls.BoxCollider2D_GetRestitutionThreshold(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.BoxCollider2D_SetRestitutionThreshold(Entity.ID, value);
			}
		}
	}

	public class CircleCollider2D : Component
	{
		public float Density
		{
			get
			{
				InternalCalls.CircleCollider2D_GetDensity(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2D_SetDensity(Entity.ID, value);
			}
		}

		public float Friction
		{
			get
			{
				InternalCalls.CircleCollider2D_GetFriction(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2D_SetFriction(Entity.ID, value);
			}
		}

		public float Restitution
		{
			get
			{
				InternalCalls.CircleCollider2D_GetRestitution(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2D_SetRestitution(Entity.ID, value);
			}
		}

		public float RestitutionThreshold
		{
			get
			{
				InternalCalls.CircleCollider2D_GetRestitutionThreshold(Entity.ID, out float result);
				return result;
			}

			set
			{
				InternalCalls.CircleCollider2D_SetRestitutionThreshold(Entity.ID, value);
			}
		}
	}

}
